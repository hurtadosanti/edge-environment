# environment

Starter Python project managed with Poetry, tuned for Raspberry Pi 4 workflows.

## Documentation

- [Python Developer Guide (Setup & Sync)](docs/01-python-setup.md)
- [Python CLI Usage & MQTT Examples](docs/02-python-usage.md)
- [Zephyr Setup Guide](docs/10-zephyr-setup.md)
- [Firmware Build & Test Guide](docs/11-firmware-build.md)
- [Raspberry Pi Debugging Guide](docs/12-rpi-debugging.md)
- [Alerting Setup & Telegram Guide](docs/13-alerting-setup.md)

## Quick Reference

| Task | Command |
| :--- | :--- |
| **Scan I2C bus** | `poetry run environment scan` |
| **Read sensor** | `poetry run environment read` |
| **Read & publish to MQTT** | `poetry run environment read --mqtt-host <broker-ip>` |
| **Run continuously with filtering** | `poetry run environment read --continuous --mqtt-host <broker-ip>` |
| **Run using YAML configuration** | `poetry run environment read --config <file-path>` |
| **Start MQTT broker** | `docker compose -f containers/mosquitto/docker-compose.yml up -d` |

## Development

For detailed setup instructions, see the [Python Developer Guide](docs/01-python-setup.md).

To sync the firmware folder to your Raspberry Pi:

```bash
make push
```





