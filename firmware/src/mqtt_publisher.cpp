#include "mqtt_publisher.hpp"
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/data/json.h>
#include <zephyr/logging/log.h>
#include <string.h>

#if defined(CONFIG_MQTT_LIB_TLS)
#include <zephyr/net/tls_credentials.h>

#define MQTT_TLS_SEC_TAG 1

static const char ca_certificate[] = 
#include "ca_cert.inc"
;

static int register_credentials(void)
{
    int rc = tls_credential_add(MQTT_TLS_SEC_TAG,
                                TLS_CREDENTIAL_CA_CERTIFICATE,
                                ca_certificate,
                                sizeof(ca_certificate));
    if (rc < 0 && rc != -EEXIST) {
        LOG_ERR("Failed to register CA certificate: %d", rc);
        return rc;
    }
    return 0;
}
#endif

LOG_MODULE_DECLARE(app, LOG_LEVEL_INF);

// Define a dedicated thread space for the MQTT network runner
K_THREAD_STACK_DEFINE(mqtt_worker_stack, 2048);
static struct k_thread mqtt_worker_data;

static void mqtt_evt_handler(struct mqtt_client *const client,
                             const struct mqtt_evt *evt)
{
    MqttPublisher *pub = static_cast<MqttPublisher *>(client->user_data);
    if (!pub) return;

    switch (evt->type) {
    case MQTT_EVT_CONNACK:
        if (evt->param.connack.return_code != 0) {
            LOG_ERR("MQTT connection refused: %d", evt->param.connack.return_code);
            pub->set_connected(false);
        } else {
            LOG_INF("MQTT client successfully connected!");
            pub->set_connected(true);
        }
        break;

    case MQTT_EVT_DISCONNECT:
        LOG_WRN("MQTT client disconnected: %d", evt->result);
        pub->set_connected(false);
        break;

    case MQTT_EVT_PUBACK:
        if (evt->result != 0) {
            LOG_ERR("MQTT PUBACK error: %d", evt->result);
        } else {
            LOG_DBG("PUBACK received for packet id: %u", evt->param.puback.message_id);
        }
        break;

    default:
        break;
    }
}

// Zephyr JSON descriptor for SensorReading
struct json_sensor_reading {
    double temperature;
    double humidity;
    double pressure;
    int address;
};

static const struct json_obj_descr json_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct json_sensor_reading, temperature, JSON_TOK_DOUBLE_FP),
    JSON_OBJ_DESCR_PRIM(struct json_sensor_reading, humidity, JSON_TOK_DOUBLE_FP),
    JSON_OBJ_DESCR_PRIM(struct json_sensor_reading, pressure, JSON_TOK_DOUBLE_FP),
    JSON_OBJ_DESCR_PRIM(struct json_sensor_reading, address, JSON_TOK_NUMBER),
};

MqttPublisher::MqttPublisher() : is_connected_(false), should_run_(false) {
    k_mutex_init(&state_mutex_);
    init_client();
}

MqttPublisher::~MqttPublisher() {
    stop();
}

void MqttPublisher::init_client() {
    mqtt_client_init(&client_);

    memset(&broker_addr_, 0, sizeof(broker_addr_));
    broker_addr_.sin_family = AF_INET;
    broker_addr_.sin_port = htons(CONFIG_APP_MQTT_BROKER_PORT);
    zsock_inet_pton(AF_INET, CONFIG_APP_MQTT_BROKER_HOST, &broker_addr_.sin_addr);

    client_.broker = &broker_addr_;
    client_.evt_cb = mqtt_evt_handler;
    client_.user_data = this;
    client_.client_id.utf8 = (const uint8_t *)"zephyr_pico";
    client_.client_id.size = strlen("zephyr_pico");

    static struct mqtt_utf8 username_utf8;
    static struct mqtt_utf8 password_utf8;

    username_utf8.utf8 = (const uint8_t *)CONFIG_APP_MQTT_USERNAME;
    username_utf8.size = strlen(CONFIG_APP_MQTT_USERNAME);

    password_utf8.utf8 = (const uint8_t *)CONFIG_APP_MQTT_PASSWORD;
    password_utf8.size = strlen(CONFIG_APP_MQTT_PASSWORD);

    client_.user_name = &username_utf8;
    client_.password = &password_utf8;

    client_.protocol_version = MQTT_VERSION_3_1_1;
    client_.keepalive = 60;

    client_.rx_buf = rx_buffer_;
    client_.rx_buf_size = sizeof(rx_buffer_);
    client_.tx_buf = tx_buffer_;
    client_.tx_buf_size = sizeof(tx_buffer_);

#if defined(CONFIG_MQTT_LIB_TLS)
    register_credentials();

    static sec_tag_t sec_tag_list[] = { MQTT_TLS_SEC_TAG };
    client_.transport.type = MQTT_TRANSPORT_SECURE;
    struct mqtt_sec_config *tls_config = &client_.transport.tls;

#if defined(CONFIG_APP_MQTT_TLS_VERIFY_REQUIRED)
    tls_config->peer_verify = TLS_PEER_VERIFY_REQUIRED;
#else
    tls_config->peer_verify = TLS_PEER_VERIFY_NONE;
#endif

    tls_config->cipher_list = NULL;
    tls_config->cipher_count = 0;
    tls_config->sec_tag_list = sec_tag_list;
    tls_config->sec_tag_count = ARRAY_SIZE(sec_tag_list);
    tls_config->hostname = CONFIG_APP_MQTT_BROKER_HOST;
#else
    client_.transport.type = MQTT_TRANSPORT_NON_SECURE;
#endif
}

bool MqttPublisher::start() {
    if (should_run_) return true;

    should_run_ = true;
    
    // Spawn background management loop
    k_thread_create(&mqtt_worker_data, mqtt_worker_stack,
                    K_THREAD_STACK_SIZEOF(mqtt_worker_stack),
                    [](void *p1, void *, void *) {
                        static_cast<MqttPublisher *>(p1)->run_loop();
                    },
                    this, nullptr, nullptr,
                    K_PRIO_PREEMPT(7), 0, K_NO_WAIT);
    
    return true;
}

void MqttPublisher::stop() {
    should_run_ = false;
    disconnect_broker();
    // Wait for worker thread to wind down gracefully if needed
    k_sleep(K_MSEC(200));
}

bool MqttPublisher::connect_broker() {
    int rc = mqtt_connect(&client_);
    if (rc != 0) {
        LOG_ERR("mqtt_connect failed: %d", rc);
        return false;
    }
    return true;
}

void MqttPublisher::disconnect_broker() {
    mqtt_disconnect(&client_, NULL);
    set_connected(false);
}

// Background thread that keeps the MQTT connection alive and handles sockets cleanly
void MqttPublisher::run_loop() {
    while (should_run_) {
        if (!get_connected()) {
            LOG_INF("Attempting connection to MQTT broker...");
            if (connect_broker()) {
                // Give the broker up to 3 seconds to trigger CONNACK via the socket poll
                int timeout_ms = 3000;
                while (timeout_ms > 0 && !get_connected() && should_run_) {
                    process_network(100);
                    timeout_ms -= 100;
                }
            }
            
            if (!get_connected()) {
                LOG_WRN("Connection failed or timed out. Retrying in 5 seconds...");
                disconnect_broker();
                k_sleep(K_MSEC(5000));
                continue;
            }
        }

        // Keep-alive processing and connection maintenance
        process_network(500); 
    }
}

void MqttPublisher::process_network(int timeout_ms) {
    struct zsock_pollfd fds[1];
    
    if (client_.transport.tcp.sock >= 0) {
        
        fds[0].fd = client_.transport.tcp.sock;
        fds[0].events = ZSOCK_POLLIN;

        int rc = zsock_poll(fds, 1, timeout_ms);
        if (rc > 0) {
            if (fds[0].revents & ZSOCK_POLLIN) {
                rc = mqtt_input(&client_);
                if (rc < 0) {
                    LOG_ERR("mqtt_input error: %d", rc);
                    set_connected(false);
                }
            }
            if (fds[0].revents & (ZSOCK_POLLERR | ZSOCK_POLLHUP)) {
                LOG_ERR("Socket error or hangup detected via poll");
                set_connected(false);
            }
        } else if (rc < 0) {
            LOG_ERR("zsock_poll failed: %d", rc);
            set_connected(false);
        }

        // Handle MQTT internal timers and keepalives (PINGREQ)
        rc = mqtt_live(&client_);
        if (rc != 0 && rc != -EAGAIN) {
            LOG_ERR("mqtt_live error: %d", rc);
            set_connected(false);
        }
    } else {
        k_sleep(K_MSEC(timeout_ms));
    }
}

bool MqttPublisher::publish_reading(const SensorReading& reading) {
    if (!get_connected()) {
        LOG_WRN("Cannot publish: Broker is offline.");
        return false;
    }

    struct json_sensor_reading j_read = {
        .temperature = reading.temperature,
        .humidity = reading.humidity,
        .pressure = reading.pressure,
        .address = reading.address
    };

    char json_buf[128];
    ssize_t json_len = json_obj_encode_buf(json_descr, ARRAY_SIZE(json_descr),
                                           &j_read, json_buf, sizeof(json_buf));
    if (json_len < 0) {
        LOG_ERR("JSON encoding failed: %d", (int)json_len);
        return false;
    }

    struct mqtt_publish_param param{};
    param.message.topic.qos = MQTT_QOS_0_AT_MOST_ONCE;
    param.message.topic.topic.utf8 = (const uint8_t *)CONFIG_APP_MQTT_TOPIC;
    param.message.topic.topic.size = strlen(CONFIG_APP_MQTT_TOPIC);
    param.message.payload.data = (uint8_t *)json_buf;
    param.message.payload.len = strlen(json_buf);
    param.message_id = k_uptime_get_32();

    // Mutex protect actual socket transmissions to avoid clashing with the worker thread
    k_mutex_lock(&state_mutex_, K_FOREVER);
    int rc = mqtt_publish(&client_, &param);
    k_mutex_unlock(&state_mutex_);

    if (rc != 0) {
        LOG_ERR("mqtt_publish failed direct write: %d", rc);
        return false;
    }

    return true;
}

void MqttPublisher::set_connected(bool connected) {
    k_mutex_lock(&state_mutex_, K_FOREVER);
    is_connected_ = connected;
    k_mutex_unlock(&state_mutex_);
}

bool MqttPublisher::get_connected() {
    k_mutex_lock(&state_mutex_, K_FOREVER);
    bool status = is_connected_;
    k_mutex_unlock(&state_mutex_);
    return status;
}
