from __future__ import annotations

import argparse

from environment.mqtt_client import publish_reading
from environment.sensor import read_bme280, scan_i2c


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Environment sensor CLI")
    parser.add_argument("--bus", type=int, default=1, help="I2C bus number")

    subparsers = parser.add_subparsers(dest="command", required=True)

    subparsers.add_parser("scan", help="Scan I2C bus for devices")

    read_parser = subparsers.add_parser("read", help="Read BME280 values")
    read_parser.add_argument("--mqtt-host", type=str, help="MQTT broker host")
    read_parser.add_argument("--mqtt-port", type=int, default=1883, help="MQTT broker port")
    read_parser.add_argument("--mqtt-topic", type=str, default="sensors/bme280", help="MQTT topic")

    return parser


def main() -> None:
    args = build_parser().parse_args()

    if args.command == "scan":
        devices = scan_i2c(bus_number=args.bus)
        print(f"Scanning I2C bus {args.bus}...")
        if not devices:
            print("No I2C devices found.")
            return

        for address in devices:
            print(f"Device found at address: {hex(address)}")
        return

    reading = read_bme280(bus_number=args.bus)
    print(f"Using BME280 at address: {hex(reading.address)}")
    print(f"Temperature: {reading.temperature:.2f} C")
    print(f"Humidity: {reading.humidity:.2f} %")
    print(f"Pressure: {reading.pressure:.2f} hPa")

    if args.mqtt_host:
        publish_reading(
            reading,
            host=args.mqtt_host,
            port=args.mqtt_port,
            topic=args.mqtt_topic,
        )
        print(f"Published reading to MQTT topic '{args.mqtt_topic}' at {args.mqtt_host}:{args.mqtt_port}")


if __name__ == "__main__":
    main()
