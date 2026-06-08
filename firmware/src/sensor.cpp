#include "sensor.hpp"
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, LOG_LEVEL_INF);

Bme280Sensor::Bme280Sensor() : dev_(nullptr) {}

bool Bme280Sensor::initialize() {
    dev_ = DEVICE_DT_GET_ANY(bosch_bme280);

    if (dev_ == nullptr) {
        LOG_ERR("No BME280 device found in devicetree");
        return false;
    }

    if (!device_is_ready(dev_)) {
        LOG_ERR("BME280 device %s is not ready", dev_->name);
        return false;
    }

    LOG_INF("Found BME280 device: %s", dev_->name);
    return true;
}

bool Bme280Sensor::read(SensorReading& out) {
    if (!dev_) {
        return false;
    }

    struct sensor_value temp, press, humidity;

    if (sensor_sample_fetch(dev_) < 0) {
        LOG_ERR("Sensor sample update error");
        return false;
    }

    sensor_channel_get(dev_, SENSOR_CHAN_AMBIENT_TEMP, &temp);
    sensor_channel_get(dev_, SENSOR_CHAN_PRESS, &press);
    sensor_channel_get(dev_, SENSOR_CHAN_HUMIDITY, &humidity);

    out.temperature = sensor_value_to_double(&temp);
    out.pressure = sensor_value_to_double(&press) * 10.0; // kPa to hPa
    out.humidity = sensor_value_to_double(&humidity);
    
#if DT_HAS_COMPAT_STATUS_OKAY(bosch_bme280)
    out.address = DT_REG_ADDR(DT_COMPAT_GET_ANY_STATUS_OKAY(bosch_bme280));
#else
    out.address = 0;
#endif

    return true;
}
