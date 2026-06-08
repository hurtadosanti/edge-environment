# Examples & Common Commands

Quick examples of how to run the environment CLI and interact with the MQTT broker.

## Running the CLI

### Scan I2C Devices
Scan the I2C bus (default bus 1) to verify BME280 sensor connection:
```bash
poetry run environment scan
```

### Read Sensor Values
Read temperature, humidity, and pressure from the BME280 sensor:
```bash
poetry run environment read
```

### Read and Publish to MQTT
Read BME280 values and publish them to a specified MQTT broker:
```bash
poetry run environment read --mqtt-host 192.168.1.10 --mqtt-topic sensors/bme280
```

---

## MQTT Broker Examples

### Start Mosquitto Broker
Start the local Mosquitto container:
```bash
docker compose -f containers/mosquitto/docker-compose.yml up -d
```


### Publish & Subscribe Examples

Using [MQTTX CLI](https://mqttx.app/cli) (EMQ's CLI client):

- **Subscribe to topic:**
  ```bash
  mqttx sub -t 'sensors/bme280' -h 'localhost' -p 1883
  ```

- **Publish to topic:**
  ```bash
  mqttx pub -t 'sensors/bme280' -h 'localhost' -p 1883 -m '{"temperature": 21.5, "humidity": 45.2, "pressure": 1013.25}'
  ```

