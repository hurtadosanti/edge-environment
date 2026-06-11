# environment

Starter Python project managed with Poetry, tuned for Raspberry Pi 4 workflows.

## Documentation

- [Python Developer Guide (Setup & Sync)](docs/01-python-setup.md)
- [Python CLI Usage & MQTT Examples](docs/02-python-usage.md)
- [MQTT Broker Setup & Testing Guide](docs/03-mqtt-testing.md)
- [Zephyr Setup Guide](docs/10-zephyr-setup.md)
- [Firmware Build & Test Guide](docs/11-firmware-build.md)
- [Raspberry Pi Debugging Guide](docs/12-rpi-debugging.md)
- [Alerting Setup & Telegram Guide](docs/13-alerting-setup.md)
- [1Password Credentials Setup Guide](docs/1password-setup.md)
- [Architecture Decisions (ADRs)](docs/14-architecture-decisions.md)

## Quick Reference

| Task | Command |
| :--- | :--- |
| **Scan I2C bus** | `poetry run environment scan` |
| **Read sensor** | `poetry run environment read` |
| **Read & publish to MQTT** | `poetry run environment read --mqtt-host <broker-ip>` |
| **Run continuously with filtering** | `poetry run environment read --continuous --mqtt-host <broker-ip>` |
| **Run using YAML configuration** | `poetry run environment read --config <file-path>` |
| **Generate Telegram token** | `op read "op://Automation/environment-measures-telegram/credential" > containers/alertmanager/telegram_token` |
| **Generate Mosquitto passwd** | `export MQTT_USER=$(op read "op://Automation/environment-measures-mqtt-auth/username") MQTT_PASS=$(op read "op://Automation/environment-measures-mqtt-auth/password") && docker run --rm -v "$(pwd)/containers/mosquitto/config:/mosquitto/config" eclipse-mosquitto:2.1-alpine mosquitto_passwd -b -c /mosquitto/config/passwd "$MQTT_USER" "$MQTT_PASS"` |
| **Start Containers Stack** | `docker compose -f containers/docker-compose.yml up -d` |

## Development

For detailed setup instructions, see the [Python Developer Guide](docs/01-python-setup.md).

To sync the firmware folder to your Raspberry Pi:

```bash
make push
```





