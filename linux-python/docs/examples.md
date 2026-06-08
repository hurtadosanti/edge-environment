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

### Continuous Loop using a YAML Configuration File
Alternatively, store settings in a YAML file (e.g., `config.yaml`). Unspecified settings will fallback to their defaults:

```yaml
mqtt_host: "192.168.1.10"
continuous: true
# Optional overrides (shown with default values):
# poll_interval: 5.0
# interval: 300.0
# delta_temp: 0.5
# delta_humidity: 2.0
# delta_pressure: 1.0
```


And run it using the config file:
```bash
poetry run environment read --config config.yaml
```

To run it for a specific duration (e.g., 5 minutes / 300 seconds) instead of indefinitely, use `--duration`:

```bash
# Using CLI flags (runs for 5m, publishing at least every 60s):
poetry run environment read --continuous --duration 300 --interval 60 --mqtt-host 192.168.1.10

# Using a config file:
poetry run environment read --config config.yaml --duration 300
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

