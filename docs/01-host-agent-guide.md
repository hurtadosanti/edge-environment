# Host Agent Developer Guide

Development guidelines, setup instructions, and CLI examples for the **host-agent** collector on Raspberry Pi / local machines.

---

## 1. Setup & Installation

### Prerequisites
* Python 3.11+
* [Poetry](https://python-poetry.org/)

### Quick Install
```bash
cd host-agent
poetry env use python3
poetry install
```

---

## 2. CLI Usage & Examples

All commands must be run from the `host-agent` directory:

```bash
cd host-agent
```

### Scan I2C Devices
```bash
poetry run environment scan
```

### Read Sensor Values
```bash
poetry run environment read
```

### Read and Publish to MQTT
```bash
poetry run environment read --mqtt-host localhost --mqtt-topic sensors/bme280
```

### Publish Events
Get the current timestamp in a format you can edit:
```bash
date "+%Y-%m-%d %H:%M:%S"
```

Publish using `config.yaml` (which references `$MQTT_PASS` fetched from 1Password):
```bash
# Fetch credentials from 1Password
export MQTT_PASS=$(op read "op://Automation/environment-measures-mqtt-auth/password")

# Publish a window open event now
poetry run environment event --event open --config config.yaml --address kitchen_window

# Publish a window half-open event now
poetry run environment event --event half-open --config config.yaml --address kitchen_window

# Publish a closed event with a specific (edited) timestamp
poetry run environment event --event closed --timestamp "2026-06-12 07:50:00" --config config.yaml --address kitchen_window
```

### Continuous Loop with Filtering (YAML Config)
Store configuration in `config.yaml` to specify thresholds and fallbacks:
```yaml
mqtt_host: "localhost"
continuous: true
poll_interval: 5.0
interval: 300.0
delta_temp: 0.5
delta_humidity: 2.0
delta_pressure: 1.0
```
Execute with:
```bash
# Run indefinitely using the configuration
poetry run environment read --config config.yaml

# Run for a specific duration (e.g. 5 minutes / 300 seconds)
poetry run environment read --config config.yaml --duration 300
```

---

## 3. Development & Testing

All development and testing commands must be run from the `host-agent` directory:

```bash
cd host-agent
```

### Run Tests
```bash
poetry run pytest
```

### Linting & Formatting
```bash
poetry run ruff check .
```

### Sync Files to Remote (Raspberry Pi)
Configured via the `Makefile` (run from the repository root) to sync workspace files to your remote target:
```bash
make push
```
