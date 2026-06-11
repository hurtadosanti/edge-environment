#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "sensor.hpp"
#include "filter.hpp"
#include "wifi_connect.hpp"
#include "mqtt_publisher.hpp"
#include <chrono>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

/* Get the devicetree node identifier for the alias */
#define TX_LED_NODE DT_ALIAS(tx_led)

static const struct gpio_dt_spec tx_led = GPIO_DT_SPEC_GET(TX_LED_NODE, gpios);

int main()
{
    LOG_INF("Starting Edge Environment firmware (C++20)...");

    // Initialize LED GPIO
    if (!gpio_is_ready_dt(&tx_led)) {
        LOG_ERR("Error: TX LED device is not ready");
        return 0;
    }

    int ret = gpio_pin_configure_dt(&tx_led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Error %d: failed to configure TX LED pin", ret);
        return 0;
    }

    // 1. Connect to Wi-Fi
    if (!wifi_connect()) {
        LOG_ERR("Failed to connect to Wi-Fi. Halting.");
        return 0;
    }
    LOG_INF("Wi-Fi connected successfully.");

    // Allow the TCP/IP stack to settle (DHCP, routing) before connecting to MQTT
    k_sleep(K_MSEC(1500));

    // 2. Initialize BME280 Sensor
    Bme280Sensor sensor;
    if (!sensor.initialize()) {
        LOG_ERR("Failed to initialize sensor. Halting.");
        return 0;
    }

    // 3. Initialize MQTT Publisher and start background thread
    MqttPublisher mqtt;
    if (!mqtt.start()) {
        LOG_ERR("Failed to start MQTT background runner.");
    } else {
        // Wait up to 3 seconds for initial connection to avoid dropping first measurement
        int wait_timeout = 30;
        while (wait_timeout-- > 0 && !mqtt.get_connected()) {
            k_sleep(K_MSEC(100));
        }
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
                
                // Turn on LED to indicate transmission starting
                gpio_pin_set_dt(&tx_led, 1);

                if (mqtt.publish_reading(reading)) {
                    LOG_INF("Published to MQTT topic '%s'", CONFIG_APP_MQTT_TOPIC);
                } else {
                    LOG_ERR("Failed to publish reading to MQTT");
                }

                // Keep the LED on for 2 seconds, then turn it off
                k_sleep(K_SECONDS(2));
                gpio_pin_set_dt(&tx_led, 0);
            }
        } else {
            LOG_ERR("Failed to read from BME280 sensor");
        }
        
        k_sleep(K_SECONDS(5));
    }
    return 0;
}
