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
    ca_path: str | None = None,
    tls_insecure: bool = False,
    username: str | None = None,
    password: str | None = None,
) -> None:
    mqtt_client = client or mqtt.Client()
    payload = json.dumps(asdict(reading))

    if ca_path:
        import ssl
        mqtt_client.tls_set(ca_certs=ca_path)
        if tls_insecure:
            mqtt_client.tls_insecure_set(True)

    if username or password:
        mqtt_client.username_pw_set(username, password)

    mqtt_client.connect(host, port, keepalive)
    mqtt_client.publish(topic, payload)
    mqtt_client.disconnect()
