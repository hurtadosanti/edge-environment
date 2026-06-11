# environment

Starter Python project managed with Poetry, tuned for Raspberry Pi 4 workflows.

## Documentation

- [Host Agent Developer Guide](docs/01-host-agent-guide.md)
- [Firmware Developer Guide](docs/02-firmware-guide.md)
- [Infrastructure & Operations Guide](docs/03-infrastructure-guide.md)
- [Architecture Decisions (ADRs)](docs/14-architecture-decisions.md)


## Quick Reference

| Task | Command |
| :--- | :--- |
| **Scan I2C bus** | `poetry run environment scan` |
| **Read sensor** | `poetry run environment read` |
| **Read & publish to MQTT** | `poetry run environment read --mqtt-host <broker-ip>` |
| **Run continuously with filtering** | `poetry run environment read --continuous --mqtt-host <broker-ip>` |
| **Run using YAML configuration** | `poetry run environment read --config <file-path>` |
| **Generate Telegram token** | `op read "op://Automation/environment-measures-telegram/credential" > infrastructure/containers/alertmanager/telegram_token` |
| **Generate Mosquitto passwd** | `export MQTT_USER=$(op read "op://Automation/environment-measures-mqtt-auth/username") MQTT_PASS=$(op read "op://Automation/environment-measures-mqtt-auth/password") && docker run --rm -v "$(pwd)/infrastructure/containers/mosquitto/config:/mosquitto/config" eclipse-mosquitto:2.1-alpine mosquitto_passwd -b -c /mosquitto/config/passwd "$MQTT_USER" "$MQTT_PASS"` |
| **Start Containers Stack** | `docker compose -f infrastructure/containers/docker-compose.yml up -d` |

## Development

For detailed setup instructions, see the [Host Agent Developer Guide](docs/01-host-agent-guide.md).

To sync the firmware folder to your Raspberry Pi:

```bash
make push
```





