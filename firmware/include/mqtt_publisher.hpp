#pragma once
#include "sensor_reading.hpp"
#include <zephyr/net/mqtt.h>

class MqttPublisher {
public:
    MqttPublisher();
    ~MqttPublisher();
    bool connect_broker();
    void disconnect_broker();
    bool publish_reading(const SensorReading& reading);
    void process();

private:
    struct mqtt_client client_;
    struct sockaddr_in broker_addr_;
    uint8_t rx_buffer_[256];
    uint8_t tx_buffer_[256];
    bool is_connected_;

    void init_client();
};
