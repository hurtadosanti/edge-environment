# MQTT Broker Setup & Testing Guide

This guide describes how to run and test the local MQTT broker using the containers stack and MQTTX CLI.

---

## 1. Start the Mosquitto Broker

Start the container stack (which includes Mosquitto, Telegraf, VictoriaMetrics, vmalert, and Alertmanager):

```bash
docker compose -f containers/docker-compose.yml up -d
```

---

## 2. Publish & Subscribe Examples

Using [MQTTX CLI](https://mqttx.app/cli) (EMQ's CLI client):

### Internal Listener (Port 1883 - Anonymous, No TLS)

This port is used for internal communication (e.g. by Telegraf). No authentication or encryption is needed.

- **Subscribe to topic:**
  ```bash
  mqttx sub -t 'sensors/bme280' -h 'localhost' -p 1883
  ```

- **Publish to topic:**
  ```bash
  mqttx pub -t 'sensors/bme280' -h 'localhost' -p 1883 -m '{"temperature": 21.5, "humidity": 45.2, "pressure": 1013.25}'
  ```

### Public Listener (Port 8883 - Authenticated & TLS)

This port is exposed for external clients (e.g. your firmware or external scripts). It enforces TLS and credentials authentication.

- **Subscribe with TLS & Auth:**
  ```bash
  mqttx sub -t 'sensors/bme280' -h 'localhost' -p 8883 \
    --protocol mqtts \
    --insecure \
    -u "$(op read "op://Automation/environment-measures-mqtt-auth/username")" \
    -P "$(op read "op://Automation/environment-measures-mqtt-auth/password")" \
    --ca containers/mosquitto/config/certs/ca.crt
  ```

- **Publish with TLS & Auth:**
  ```bash
  mqttx pub -t 'sensors/bme280' -h 'localhost' -p 8883 -m '{"temperature": 21.5, "humidity": 45.2, "pressure": 1013.25}' \
    --protocol mqtts \
    --insecure \
    -u "$(op read "op://Automation/environment-measures-mqtt-auth/username")" \
    -P "$(op read "op://Automation/environment-measures-mqtt-auth/password")" \
    --ca containers/mosquitto/config/certs/ca.crt
  ```
