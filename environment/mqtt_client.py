from __future__ import annotations

import json
from dataclasses import asdict

import paho.mqtt.client as mqtt

from environment.sensor import SensorReading


def publish_reading(
    reading: SensorReading,
    host: str,
    topic: str,
    port: int = 1883,
    keepalive: int = 60,
    client: mqtt.Client | None = None,
) -> None:
    mqtt_client = client or mqtt.Client()
    payload = json.dumps(asdict(reading))

    mqtt_client.connect(host, port, keepalive)
    mqtt_client.publish(topic, payload)
    mqtt_client.disconnect()
