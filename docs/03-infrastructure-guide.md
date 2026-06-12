# Infrastructure & Operations Guide

Deployment, credential management, TLS setup, and alerting configuration for the local container stack (Mosquitto, Telegraf, VictoriaMetrics, vmalert, Alertmanager).

---

## 1. Start the Container Stack

Run the observability and broker services:
```bash
docker compose -f infrastructure/containers/docker-compose.yml up -d
```

### Monitoring Dashboards & UIs
* **vmalert UI**: `http://localhost:8880` (Alert execution status)
* **Alertmanager UI**: `http://localhost:9093` (Active notifications & silences)

---

## 2. 1Password Secret Management

We use the 1Password `op` CLI to inject credentials and TLS configurations without committing secrets to Git.

### 1. MQTT Credentials
Create a Login item in the **Automation** vault named `environment-measures-mqtt-auth`. Inject it into your local setup:
```bash
# Update Mosquitto password file
export MQTT_USER=$(op read "op://Automation/environment-measures-mqtt-auth/username")
export MQTT_PASS=$(op read "op://Automation/environment-measures-mqtt-auth/password")
docker run --rm -v "$(pwd)/infrastructure/containers/mosquitto/config:/mosquitto/config" \
  eclipse-mosquitto:2.1-alpine \
  mosquitto_passwd -b -c /mosquitto/config/passwd "$MQTT_USER" "$MQTT_PASS"

# Inject into Git-ignored firmware configs
echo "CONFIG_APP_MQTT_USERNAME=\"$MQTT_USER\"" >> firmware/local.conf
echo "CONFIG_APP_MQTT_PASSWORD=\"$MQTT_PASS\"" >> firmware/local.conf
```

### 2. Telegram Alertmanager Config
Create a credential item in the **Automation** vault named `environment-measures-telegram`. Set up the Alertmanager integration:
```bash
# Write token to git-ignored configuration file
op read "op://Automation/environment-measures-telegram/credential" > infrastructure/containers/alertmanager/telegram_token
```
Ensure Alertmanager is configured (`infrastructure/containers/alertmanager/alertmanager.yml`) to read from the mounted token path:
```yaml
receivers:
- name: 'telegram'
  telegram_configs:
  - bot_token_file: '/etc/alertmanager/telegram_token'
    chat_id: 8584821168
```

---

## 3. TLS Certificates Setup

Secure MQTT traffic uses TLS. The CA credentials are kept in the **Automation** vault under the item `environment-measures-mqtt-broker`.

### Provision TLS CA Certificates
Retrieve the CA certificate/key and generate the host-specific server certificates:
```bash
# 1. Fetch CA certificates from 1Password
op read "op://Automation/environment-measures-mqtt-broker/ca.key" > infrastructure/containers/mosquitto/config/certs/ca.key
op read "op://Automation/environment-measures-mqtt-broker/ca.crt" > infrastructure/containers/mosquitto/config/certs/ca.crt

# 2. Generate host-specific certificates (replace 'localhost' with server domain/IP)
./infrastructure/containers/mosquitto/config/certs/generate_certs.sh localhost
```

---

## 4. Alert Rules Configuration

Alerts are defined in `infrastructure/containers/vmalert/rules.yml` using MetricsQL:

* **Threshold Alert**: Triggers when temperature exceeds 30°C.
  ```yaml
  expr: mqtt_consumer_temperature > 30
  ```
* **Health Check**: Triggers if a device is offline (no data for 15 minutes) but was active within the last hour.
  ```yaml
  expr: last_over_time(mqtt_consumer_temperature[1h]) unless last_over_time(mqtt_consumer_temperature[15m])
  ```

---

## 5. Exporting Data for Machine Learning

A Python script is provided at [export_csv.py](file:///Users/shurtado/Projects/environment-measures/scripts/export_csv.py) to fetch time series metrics from VictoriaMetrics and export them into a structured CSV file for ML model training.

The script queries `temperature`, `humidity`, and `pressure`, matches them by timestamp and device MAC address (`mac` tag), and outputs them to a formatted file.

### Usage

From the root directory:
```bash
python3 scripts/export_csv.py --host <vm-host-ip>:8428 --hours <time-range-hours> --output <filename.csv>
```

Example (fetch last 24 hours of data from the Raspberry Pi broker):
```bash
python3 scripts/export_csv.py --host 192.168.1.95:8428 --hours 24 --output dataset.csv
```
