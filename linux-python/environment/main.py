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
    read_parser.add_argument("--mqtt-port", type=int, help="MQTT broker port")
    read_parser.add_argument("--mqtt-topic", type=str, help="MQTT topic")
    read_parser.add_argument("--continuous", action="store_true", default=None, help="Run continuously in a loop")
    read_parser.add_argument("--poll-interval", type=float, help="Polling interval in seconds")
    read_parser.add_argument("--interval", type=float, help="Max interval in seconds between publications")
    read_parser.add_argument("--delta-temp", type=float, help="Temperature delta threshold to force publication")
    read_parser.add_argument("--delta-humidity", type=float, help="Humidity delta threshold to force publication")
    read_parser.add_argument("--delta-pressure", type=float, help="Pressure delta threshold to force publication")
    read_parser.add_argument("--config", type=str, help="Path to YAML configuration file")
    read_parser.add_argument("--duration", type=float, help="Duration to run the loop in seconds (default: indefinitely)")

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

    # Load config file if specified
    config: dict[str, any] = {}
    if args.config:
        from environment.config import load_yaml_config
        config = load_yaml_config(args.config)

    # Resolution logic: CLI -> Config -> Default
    DEFAULTS = {
        "mqtt_host": None,
        "mqtt_port": 1883,
        "mqtt_topic": "sensors/bme280",
        "continuous": False,
        "poll_interval": 5.0,
        "interval": 300.0,
        "delta_temp": 0.5,
        "delta_humidity": 2.0,
        "delta_pressure": 1.0,
        "duration": None,
    }

    def get_setting(key: str) -> any:
        cli_val = getattr(args, key, None)
        if cli_val is not None:
            return cli_val
        if key in config:
            return config[key]
        return DEFAULTS[key]

    mqtt_host = get_setting("mqtt_host")
    mqtt_port = get_setting("mqtt_port")
    mqtt_topic = get_setting("mqtt_topic")
    poll_interval = get_setting("poll_interval")
    interval = get_setting("interval")
    delta_temp = get_setting("delta_temp")
    delta_humidity = get_setting("delta_humidity")
    delta_pressure = get_setting("delta_pressure")
    duration = get_setting("duration")

    # Infer continuous mode if any loop-specific settings are provided on the CLI or in the config
    has_loop_setting = any(
        getattr(args, key, None) is not None or key in config
        for key in ["duration", "interval", "poll_interval", "delta_temp", "delta_humidity", "delta_pressure"]
    )
    continuous = get_setting("continuous") or has_loop_setting

    if not continuous:
        reading = read_bme280(bus_number=args.bus)
        print(f"Using BME280 at address: {hex(reading.address)}")
        print(f"Temperature: {reading.temperature:.2f} C")
        print(f"Humidity: {reading.humidity:.2f} %")
        print(f"Pressure: {reading.pressure:.2f} hPa")

        if mqtt_host:
            publish_reading(
                reading,
                host=mqtt_host,
                port=mqtt_port,
                topic=mqtt_topic,
            )
            print(f"Published reading to MQTT topic '{mqtt_topic}' at {mqtt_host}:{mqtt_port}")
        return

    import time
    from environment.filter import ReadingFilter

    reading_filter = ReadingFilter(
        interval_seconds=interval,
        delta_temp=delta_temp,
        delta_humidity=delta_humidity,
        delta_pressure=delta_pressure,
    )

    print(f"Starting continuous read loop (polling every {poll_interval}s)...")
    if duration:
        print(f"Will run for {duration} seconds.")

    start_time = time.time()
    try:
        while True:
            # Check duration
            if duration and (time.time() - start_time >= duration):
                print(f"\nDuration limit of {duration}s reached. Stopping loop.")
                break

            try:
                reading = read_bme280(bus_number=args.bus)
                if reading_filter.should_publish(reading):
                    print(f"[{time.strftime('%Y-%m-%d %H:%M:%S')}] New measurement published:")
                    print(f"  Temperature: {reading.temperature:.2f} C")
                    print(f"  Humidity: {reading.humidity:.2f} %")
                    print(f"  Pressure: {reading.pressure:.2f} hPa")

                    if mqtt_host:
                        publish_reading(
                            reading,
                            host=mqtt_host,
                            port=mqtt_port,
                            topic=mqtt_topic,
                        )
                        print(f"  Published to MQTT topic '{mqtt_topic}' at {mqtt_host}:{mqtt_port}")
            except Exception as e:
                print(f"Error reading sensor or publishing: {e}")
            time.sleep(poll_interval)
    except KeyboardInterrupt:
        print("\nStopping continuous read loop.")


if __name__ == "__main__":
    main()


