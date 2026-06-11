from __future__ import annotations

from dataclasses import dataclass

import bme280
import smbus2

POSSIBLE_BME280_ADDRESSES = (0x76, 0x77)


@dataclass(frozen=True)
class SensorReading:
    temperature: float
    humidity: float
    pressure: float
    address: int


def scan_i2c(bus_number: int = 1) -> list[int]:
    found: list[int] = []
    bus = smbus2.SMBus(bus_number)
    try:
        for address in range(0x03, 0x78):
            try:
                bus.read_byte(address)
                found.append(address)
            except OSError:
                continue
    finally:
        bus.close()

    return found


def detect_bme280_address(bus: smbus2.SMBus) -> int:
    for address in POSSIBLE_BME280_ADDRESSES:
        try:
            bus.read_byte(address)
            return address
        except OSError:
            continue

    raise RuntimeError("No BME280 found at addresses 0x76 or 0x77")


def read_bme280(bus_number: int = 1) -> SensorReading:
    bus = smbus2.SMBus(bus_number)
    try:
        address = detect_bme280_address(bus)
        calibration_params = bme280.load_calibration_params(bus, address)
        data = bme280.sample(bus, address, calibration_params)
        return SensorReading(
            temperature=float(data.temperature),
            humidity=float(data.humidity),
            pressure=float(data.pressure),
            address=address,
        )
    finally:
        bus.close()


def read_bme280_demo() -> SensorReading:
    import math
    import random
    import time

    t = time.time()
    temp = 22.0 + 2.0 * math.sin(t / 60.0) + random.uniform(-0.1, 0.1)
    hum = 50.0 + 5.0 * math.cos(t / 60.0) + random.uniform(-0.5, 0.5)
    press = 1013.25 + random.uniform(-0.2, 0.2)
    return SensorReading(
        temperature=temp,
        humidity=hum,
        pressure=press,
        address=0x76,
    )

