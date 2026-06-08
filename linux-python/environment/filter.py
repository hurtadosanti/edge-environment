from __future__ import annotations

import time

from environment.sensor import SensorReading


class ReadingFilter:
    """Filters sensor readings based on time intervals and value deltas."""

    def __init__(
        self,
        interval_seconds: float = 300.0,
        delta_temp: float = 0.5,
        delta_humidity: float = 2.0,
        delta_pressure: float = 1.0,
    ) -> None:
        self.interval_seconds = interval_seconds
        self.delta_temp = delta_temp
        self.delta_humidity = delta_humidity
        self.delta_pressure = delta_pressure
        self.last_reading: SensorReading | None = None
        self.last_published_time: float = 0.0

    def should_publish(self, reading: SensorReading, current_time: float | None = None) -> bool:
        now = current_time if current_time is not None else time.time()

        if self.last_reading is None:
            self.last_reading = reading
            self.last_published_time = now
            return True

        elapsed = now - self.last_published_time
        if elapsed >= self.interval_seconds:
            self.last_reading = reading
            self.last_published_time = now
            return True

        temp_diff = abs(reading.temperature - self.last_reading.temperature)
        hum_diff = abs(reading.humidity - self.last_reading.humidity)
        press_diff = abs(reading.pressure - self.last_reading.pressure)

        if (
            temp_diff >= self.delta_temp
            or hum_diff >= self.delta_humidity
            or press_diff >= self.delta_pressure
        ):
            self.last_reading = reading
            self.last_published_time = now
            return True

        return False
