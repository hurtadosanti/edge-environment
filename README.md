# environment

Starter Python project managed with Poetry, tuned for Raspberry Pi 4 workflows.

## Documentation

- [Developer Guide (Setup, Tests, Linting)](file:///Users/shurtado/Projects/edge-environment/docs/dev.md)
- [Examples & Common Commands](file:///Users/shurtado/Projects/edge-environment/docs/examples.md)

## Quick Reference

| Task | Command |
| :--- | :--- |
| **Scan I2C bus** | `poetry run environment scan` |
| **Read sensor** | `poetry run environment read` |
| **Read & publish to MQTT** | `poetry run environment read --mqtt-host <broker-ip>` |
| **Start MQTT broker** | `docker compose -f containers/mosquitto/docker-compose.yml up -d` |


