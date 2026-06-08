#pragma once
#include <zephyr/device.h>
#include "sensor_reading.hpp"

class Bme280Sensor {
public:
    Bme280Sensor();
    bool initialize();
    bool read(SensorReading& out);

private:
    const struct device* dev_;
};
