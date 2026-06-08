# environment

Starter Python project managed with Poetry, tuned for Raspberry Pi 4 workflows.

## Documentation

- [Zephyr Setup Guide](file:///Users/shurtado/Projects/environment-measures/docs/zephyr-setup.md)
- [Compile & Test Guide](file:///Users/shurtado/Projects/environment-measures/docs/compile.md)
- [Raspberry Pi Debugging Guide](file:///Users/shurtado/Projects/environment-measures/docs/rpi.md)

## Quick Reference

| Task | Command |
| :--- | :--- |
| **Scan I2C bus** | `poetry run environment scan` |
| **Read sensor** | `poetry run environment read` |
| **Read & publish to MQTT** | `poetry run environment read --mqtt-host <broker-ip>` |
| **Run continuously with filtering** | `poetry run environment read --continuous --mqtt-host <broker-ip>` |
| **Run using YAML configuration** | `poetry run environment read --config <file-path>` |
| **Run for specific duration** | `poetry run environment read --config <file-path> --duration <seconds>` |
| **Start MQTT broker** | `docker compose -f containers/mosquitto/docker-compose.yml up -d` |

## Development on Raspberry Pi

1. Configure connection details in `.env` (use `.env.example` as a template).
2. Sync the firmware folder to your Raspberry Pi:

```bash
make push
```





