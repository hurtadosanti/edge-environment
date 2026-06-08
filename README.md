# environment

Starter Python project managed with Poetry, tuned for Raspberry Pi 4 workflows.

## Prerequisites

- Python 3.11+ installed (`python3 --version`)
- Poetry installed

Install Poetry (official installer):

```bash
curl -sSL https://install.python-poetry.org | python3 -
```

## Setup

```bash
poetry env use python3
poetry install
```

If Poetry is not on your PATH in non-interactive shells, use:

```bash
~/.local/bin/poetry env use python3
~/.local/bin/poetry install
```

## BME280 Sensor

Install the sensor libraries in this project:

```bash
~/.local/bin/poetry add smbus2 RPi.bme280
```

Scan for I2C devices:

```bash
~/.local/bin/poetry run python scan_i2c.py
```

Run the sensor reader:

```bash
~/.local/bin/poetry run python read_sensor.py
```

Use the package main entrypoint:

```bash
~/.local/bin/poetry run environment scan
~/.local/bin/poetry run environment read
```

Read and publish to MQTT:

```bash
~/.local/bin/poetry run environment read --mqtt-host 192.168.1.10 --mqtt-topic sensors/bme280
```

## Run tests

```bash
poetry run pytest
```

## Lint

```bash
poetry run ruff check .
```

## Notes for Raspberry Pi 4

- Prefer system packages for heavy native deps when possible.
- Keep your Python version aligned with what is available on the Pi.
- If a package builds slowly, look for prebuilt wheels first.
