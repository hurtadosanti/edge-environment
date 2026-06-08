from __future__ import annotations

from environment.filter import ReadingFilter
from environment.sensor import SensorReading


def test_filter_first_reading_always_published() -> None:
    filter_ = ReadingFilter(interval_seconds=60.0)
    reading = SensorReading(temperature=20.0, humidity=40.0, pressure=1000.0, address=0x77)
    assert filter_.should_publish(reading, current_time=100.0) is True
    assert filter_.last_reading == reading


def test_filter_time_interval_forces_publication() -> None:
    filter_ = ReadingFilter(interval_seconds=60.0, delta_temp=1.0)
    reading1 = SensorReading(temperature=20.0, humidity=40.0, pressure=1000.0, address=0x77)
    reading2 = SensorReading(temperature=20.1, humidity=40.0, pressure=1000.0, address=0x77)

    # First is published
    assert filter_.should_publish(reading1, current_time=100.0) is True

    # Same temp, only 10s elapsed -> not published
    assert filter_.should_publish(reading2, current_time=110.0) is False

    # 60s elapsed -> published
    assert filter_.should_publish(reading2, current_time=160.0) is True


def test_filter_delta_forces_publication() -> None:
    filter_ = ReadingFilter(interval_seconds=60.0, delta_temp=0.5, delta_humidity=2.0, delta_pressure=1.0)
    reading1 = SensorReading(temperature=20.0, humidity=40.0, pressure=1000.0, address=0x77)

    # First is published
    assert filter_.should_publish(reading1, current_time=100.0) is True

    # Temp delta exceeded (0.6 > 0.5) -> published
    reading2 = SensorReading(temperature=20.6, humidity=40.0, pressure=1000.0, address=0x77)
    assert filter_.should_publish(reading2, current_time=110.0) is True

    # Humidity delta exceeded (3.0 > 2.0) -> published
    reading3 = SensorReading(temperature=20.6, humidity=43.0, pressure=1000.0, address=0x77)
    assert filter_.should_publish(reading3, current_time=120.0) is True

    # Pressure delta exceeded (1.5 > 1.0) -> published
    reading4 = SensorReading(temperature=20.6, humidity=43.0, pressure=1001.5, address=0x77)
    assert filter_.should_publish(reading4, current_time=130.0) is True
