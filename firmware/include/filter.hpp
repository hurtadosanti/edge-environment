#pragma once
#include "sensor_reading.hpp"
#include <chrono>

class ReadingFilter {
public:
    ReadingFilter(std::chrono::milliseconds interval = std::chrono::minutes(5), 
                  double delta_temp = 0.5, 
                  double delta_humidity = 2.0, 
                  double delta_pressure = 1.0);

    bool should_publish(const SensorReading& reading, std::chrono::milliseconds current_time);

private:
    std::chrono::milliseconds interval_;
    double delta_temp_;
    double delta_humidity_;
    double delta_pressure_;
    SensorReading last_reading_;
    std::chrono::milliseconds last_published_time_;
    bool has_last_reading_;
};
