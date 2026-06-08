#include "filter.hpp"
#include <cmath>

ReadingFilter::ReadingFilter(std::chrono::milliseconds interval, 
                             double delta_temp, 
                             double delta_humidity, 
                             double delta_pressure)
    : interval_(interval),
      delta_temp_(delta_temp),
      delta_humidity_(delta_humidity),
      delta_pressure_(delta_pressure),
      last_reading_{0.0, 0.0, 0.0, 0},
      last_published_time_(0),
      has_last_reading_(false) {}

bool ReadingFilter::should_publish(const SensorReading& reading, std::chrono::milliseconds current_time) {
    if (!has_last_reading_) {
        last_reading_ = reading;
        last_published_time_ = current_time;
        has_last_reading_ = true;
        return true;
    }

    auto elapsed = current_time - last_published_time_;
    if (elapsed >= interval_) {
        last_reading_ = reading;
        last_published_time_ = current_time;
        return true;
    }

    double temp_diff = std::fabs(reading.temperature - last_reading_.temperature);
    double hum_diff = std::fabs(reading.humidity - last_reading_.humidity);
    double press_diff = std::fabs(reading.pressure - last_reading_.pressure);

    if (temp_diff >= delta_temp_ || hum_diff >= delta_humidity_ || press_diff >= delta_pressure_) {
        last_reading_ = reading;
        last_published_time_ = current_time;
        return true;
    }

    return false;
}
