from __future__ import annotations

import os
import tempfile

import pytest

from environment.config import load_yaml_config


def test_load_yaml_config_parses_types() -> None:
    yaml_content = """
    # This is a comment
    mqtt_host: "192.168.1.10"
    mqtt_port: 1883
    mqtt_topic: sensors/bme280
    continuous: true
    poll_interval: 5.5
    null_val: null
    """
    with tempfile.NamedTemporaryFile("w", delete=False) as f:
        f.write(yaml_content)
        temp_name = f.name

    try:
        config = load_yaml_config(temp_name)
        assert config["mqtt_host"] == "192.168.1.10"
        assert config["mqtt_port"] == 1883
        assert config["mqtt_topic"] == "sensors/bme280"
        assert config["continuous"] is True
        assert config["poll_interval"] == 5.5
        assert config["null_val"] is None
    finally:
        os.unlink(temp_name)


def test_load_yaml_config_raises_on_missing_file() -> None:
    with pytest.raises(FileNotFoundError):
        load_yaml_config("nonexistent_file_path.yaml")
