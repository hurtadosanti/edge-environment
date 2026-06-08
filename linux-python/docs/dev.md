# Developer Guide

Development guidelines and setup instructions for the **environment** project on Raspberry Pi 4.

## Quality Control

### Run Tests

```bash
poetry run pytest
```

### Linting

```bash
poetry run ruff check .
```

## Setup

```bash
poetry env use python3
poetry install
```


## Syncing to Remote

To sync files from your local machine to the target remote machine (e.g. Raspberry Pi) via SSH:

```bash
rsync -avz --exclude '.git' --exclude '.venv' --exclude '__pycache__' --exclude '.pytest_cache' --exclude '.ruff_cache' ./ <username>@<remote-ip>:/path/to/destination/
```


## Hardware & Dependencies

Install the sensor libraries in this project:

```bash
~/.local/bin/poetry add smbus2 RPi.bme280
```

### Notes for Raspberry Pi 4

- Prefer system packages for heavy native dependencies when possible.
- Keep your Python version aligned with what is available on the Pi.
- If a package builds slowly, look for prebuilt wheels first.

## Prerequisites

- Python 3.11+ installed (`python3 --version`)
- Poetry installed

Install Poetry (official installer):

```bash
curl -sSL https://install.python-poetry.org | python3 -
```
