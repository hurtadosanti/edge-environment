#include "mqtt_publisher.hpp"
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/data/json.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_DECLARE(app, LOG_LEVEL_INF);

static void mqtt_evt_handler(struct mqtt_client *const client,
                             const struct mqtt_evt *evt)
{
    MqttPublisher *pub = static_cast<MqttPublisher *>(client->user_data);

    switch (evt->type) {
    case MQTT_EVT_CONNACK:
        if (evt->param.connack.return_code != 0) {
            LOG_ERR("MQTT connection failed %d", evt->param.connack.return_code);
        } else {
            LOG_INF("MQTT client connected!");
            if (pub) {
                pub->set_connected(true);
            }
        }
        break;

    case MQTT_EVT_DISCONNECT:
        LOG_INF("MQTT client disconnected %d", evt->result);
        if (pub) {
            pub->set_connected(false);
        }
        break;

    case MQTT_EVT_PUBACK:
        if (evt->result != 0) {
            LOG_ERR("MQTT PUBACK error %d", evt->result);
        } else {
            LOG_INF("PUBACK packet id: %u", evt->param.puback.message_id);
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
    JSON_OBJ_DESCR_PRIM(struct json_sensor_reading, temperature, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct json_sensor_reading, humidity, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct json_sensor_reading, pressure, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct json_sensor_reading, address, JSON_TOK_NUMBER),
};

MqttPublisher::MqttPublisher() : is_connected_(false) {
    init_client();
}

MqttPublisher::~MqttPublisher() {
    if (is_connected_) {
        disconnect_broker();
    }
}

void MqttPublisher::init_client() {
    mqtt_client_init(&client_);

    // Set broker address
    broker_addr_.sin_family = AF_INET;
    broker_addr_.sin_port = htons(CONFIG_APP_MQTT_BROKER_PORT);
    zsock_inet_pton(AF_INET, CONFIG_APP_MQTT_BROKER_HOST, &broker_addr_.sin_addr);

    // Client configuration
    client_.broker = &broker_addr_;
    client_.evt_cb = mqtt_evt_handler;
    client_.user_data = this;
    client_.client_id.utf8 = (const uint8_t *)"zephyr_pico";
    client_.client_id.size = strlen("zephyr_pico");
    client_.password = NULL;
    client_.user_name = NULL;
    client_.protocol_version = MQTT_VERSION_3_1_1;

    // Buffers
    client_.rx_buf = rx_buffer_;
    client_.rx_buf_size = sizeof(rx_buffer_);
    client_.tx_buf = tx_buffer_;
    client_.tx_buf_size = sizeof(tx_buffer_);

    // Transport configuration
    client_.transport.type = MQTT_TRANSPORT_NON_SECURE;
}

bool MqttPublisher::connect_broker() {
    if (is_connected_) {
        return true;
    }

    int rc = mqtt_connect(&client_);
    if (rc != 0) {
        LOG_ERR("mqtt_connect failed: %d", rc);
        return false;
    }

    // Wait up to 5 seconds for connection to establish
    int timeout = 50;
    while (timeout-- > 0) {
        rc = mqtt_input(&client_);
        if (rc < 0) {
            LOG_ERR("mqtt_input failed: %d", rc);
            return false;
        }
        
        if (is_connected_) {
            return true;
        }
        
        k_sleep(K_MSEC(100));
    }

    LOG_ERR("MQTT connection timeout");
    return false;
}

void MqttPublisher::disconnect_broker() {
    if (!is_connected_) return;

    mqtt_disconnect(&client_, NULL);
    is_connected_ = false;
}

bool MqttPublisher::publish_reading(const SensorReading& reading) {
    if (!is_connected_ && !connect_broker()) {
        LOG_ERR("Cannot publish, not connected to MQTT broker");
        return false;
    }

    // Map reading to JSON struct
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

    LOG_INF("JSON payload: %s", json_buf);

    struct mqtt_publish_param param;
    param.message.topic.qos = MQTT_QOS_0_AT_MOST_ONCE;
    param.message.topic.topic.utf8 = (const uint8_t *)CONFIG_APP_MQTT_TOPIC;
    param.message.topic.topic.size = strlen(CONFIG_APP_MQTT_TOPIC);
    param.message.payload.data = (uint8_t *)json_buf;
    param.message.payload.len = json_len;
    param.message_id = k_uptime_get_32();
    param.dup_flag = 0U;
    param.retain_flag = 0U;

    int rc = mqtt_publish(&client_, &param);
    if (rc != 0) {
        LOG_ERR("mqtt_publish failed: %d", rc);
        is_connected_ = false; // Reset connection state on error
        return false;
    }

    // Process network to send the packet
    mqtt_input(&client_);

    return true;
}

void MqttPublisher::process() {
    if (is_connected_) {
        mqtt_input(&client_);
    }
}

void MqttPublisher::set_connected(bool connected) {
    is_connected_ = connected;
}
