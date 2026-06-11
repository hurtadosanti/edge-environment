from __future__ import annotations

from types import SimpleNamespace

import pytest

from environment.sensor import detect_bme280_address, read_bme280, scan_i2c


class FakeBus:
    def __init__(self, available_addresses: set[int]):
        self.available_addresses = available_addresses
        self.closed = False

    def read_byte(self, address: int) -> int:
        if address in self.available_addresses:
            return 0x00
        raise OSError("No device")

    def close(self) -> None:
        self.closed = True


def test_scan_i2c_returns_detected_addresses(monkeypatch: pytest.MonkeyPatch) -> None:
    fake_bus = FakeBus({0x20, 0x77})
    monkeypatch.setattr("environment.sensor.smbus2.SMBus", lambda _bus_number: fake_bus)

    result = scan_i2c(bus_number=1)

    assert result == [0x20, 0x77]
    assert fake_bus.closed is True


def test_detect_bme280_address_finds_0x77_when_0x76_missing() -> None:
    fake_bus = FakeBus({0x77})

    result = detect_bme280_address(fake_bus)

    assert result == 0x77


def test_detect_bme280_address_raises_when_missing() -> None:
    fake_bus = FakeBus(set())

    with pytest.raises(RuntimeError):
        detect_bme280_address(fake_bus)


def test_read_bme280_returns_structured_reading(monkeypatch: pytest.MonkeyPatch) -> None:
    fake_bus = FakeBus({0x76})
    monkeypatch.setattr("environment.sensor.smbus2.SMBus", lambda _bus_number: fake_bus)
    monkeypatch.setattr("environment.sensor.bme280.load_calibration_params", lambda _bus, _address: object())
    monkeypatch.setattr(
        "environment.sensor.bme280.sample",
        lambda _bus, _address, _params: SimpleNamespace(
            temperature=23.5,
            humidity=40.1,
            pressure=1013.2,
        ),
    )

    reading = read_bme280(bus_number=1)

    assert reading.address == 0x76
    assert reading.temperature == 23.5
    assert reading.humidity == 40.1
    assert reading.pressure == 1013.2
    assert fake_bus.closed is True


def test_read_bme280_demo_returns_mock_reading() -> None:
    from environment.sensor import read_bme280_demo

    reading = read_bme280_demo()
    assert reading.address == 0x76
    assert 19.0 <= reading.temperature <= 25.0
    assert 44.0 <= reading.humidity <= 56.0
    assert 1012.0 <= reading.pressure <= 1014.0
