from __future__ import annotations

import json

from environment.mqtt_client import publish_reading
from environment.sensor import SensorReading


class FakeMqttClient:
    def __init__(self) -> None:
        self.connected = None
        self.published = None
        self.disconnected = False
        self.tls_certs = None
        self.tls_insecure = False
        self.username = None
        self.password = None

    def connect(self, host: str, port: int, keepalive: int) -> None:
        self.connected = (host, port, keepalive)

    def publish(self, topic: str, payload: str) -> None:
        self.published = (topic, payload)

    def disconnect(self) -> None:
        self.disconnected = True

    def tls_set(self, ca_certs: str) -> None:
        self.tls_certs = ca_certs

    def tls_insecure_set(self, value: bool) -> None:
        self.tls_insecure = value

    def username_pw_set(self, username: str, password: str | None = None) -> None:
        self.username = username
        self.password = password


def test_publish_reading_sends_json_payload() -> None:
    reading = SensorReading(
        temperature=24.0,
        humidity=50.0,
        pressure=1009.1,
        address=0x77,
    )
    client = FakeMqttClient()

    publish_reading(
        reading,
        host="localhost",
        topic="sensors/bme280",
        port=1883,
        keepalive=60,
        client=client,
    )

    assert client.connected == ("localhost", 1883, 60)
    assert client.published is not None
    topic, payload = client.published
    assert topic == "sensors/bme280"
    assert json.loads(payload) == {
        "temperature": 24.0,
        "humidity": 50.0,
        "pressure": 1009.1,
        "address": 0x77,
    }
    assert client.disconnected is True


def test_publish_reading_with_tls_and_auth() -> None:
    reading = SensorReading(
        temperature=24.0,
        humidity=50.0,
        pressure=1009.1,
        address=0x77,
    )
    client = FakeMqttClient()

    publish_reading(
        reading,
        host="192.168.1.10",
        topic="sensors/bme280",
        port=8883,
        keepalive=60,
        client=client,
        ca_path="ca.crt",
        tls_insecure=True,
        username="user",
        password="pwd",
    )

    assert client.connected == ("192.168.1.10", 8883, 60)
    assert client.tls_certs == "ca.crt"
    assert client.tls_insecure is True
    assert client.username == "user"
    assert client.password == "pwd"
    assert client.disconnected is True
