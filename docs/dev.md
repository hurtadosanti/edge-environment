# Developer Guide

Development guidelines and setup instructions for the **environment** project on Raspberry Pi 4.

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

## Hardware & Dependencies

Install the sensor libraries in this project:

```bash
~/.local/bin/poetry add smbus2 RPi.bme280
```

## Quality Control

### Run Tests

```bash
poetry run pytest
```

### Linting

```bash
poetry run ruff check .
```

## Notes for Raspberry Pi 4

- Prefer system packages for heavy native dependencies when possible.
- Keep your Python version aligned with what is available on the Pi.
- If a package builds slowly, look for prebuilt wheels first.
