#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "sensor.hpp"
#include "filter.hpp"
#include "wifi_connect.hpp"
#include "mqtt_publisher.hpp"
#include <chrono>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

int main()
{
    LOG_INF("Starting Edge Environment firmware (C++20)...");

    // 1. Connect to Wi-Fi
    if (!wifi_connect()) {
        LOG_ERR("Failed to connect to Wi-Fi. Halting.");
        return 0;
    }
    LOG_INF("Wi-Fi connected successfully.");

    // 2. Initialize BME280 Sensor
    Bme280Sensor sensor;
    if (!sensor.initialize()) {
        LOG_ERR("Failed to initialize sensor. Halting.");
        return 0;
    }

    // 3. Initialize MQTT Publisher
    MqttPublisher mqtt;
    if (!mqtt.connect_broker()) {
        LOG_ERR("Failed to connect to MQTT broker. But will retry in loop...");
    }

    // 4. Initialize Reading Filter
    ReadingFilter filter;

    LOG_INF("Entering continuous measurement loop...");

    while (true) {
        SensorReading reading;
        if (sensor.read(reading)) {
            uint32_t now_ms = k_uptime_get_32();
            if (filter.should_publish(reading, std::chrono::milliseconds(now_ms))) {
                LOG_INF("New measurement: Temp=%.2f C, Hum=%.2f %%, Press=%.2f hPa",
                        reading.temperature, reading.humidity, reading.pressure);
                
                if (mqtt.publish_reading(reading)) {
                    LOG_INF("Published to MQTT topic '%s'", CONFIG_APP_MQTT_TOPIC);
                } else {
                    LOG_ERR("Failed to publish reading to MQTT");
                }
            }
        } else {
            LOG_ERR("Failed to read from BME280 sensor");
        }
        
        // Process any pending MQTT network traffic
        mqtt.process();

        k_sleep(K_SECONDS(5));
    }
    return 0;
}
