# Managing MQTT Credentials with 1Password (op CLI)

This guide explains how to store your MQTT broker credentials in 1Password and use the `op` CLI to inject them into the Mosquitto configuration, Zephyr firmware, and Python environment.

---

## 1. Create the Credentials in 1Password

Using the 1Password GUI, create a Login item in the **Automation** vault:

- **Item Title**: `environment-measures-mqtt-auth`
- **Username**: `device_user` (or your chosen username)
- **Password**: `device_pass` (or your chosen password)

---

## 2. Inject Credentials into Mosquitto Broker Configuration

To update the Mosquitto password file (`containers/mosquitto/config/passwd`) with the credentials from 1Password:

```bash
# 1. Fetch credentials from 1Password
MQTT_USER=$(op read "op://Automation/environment-measures-mqtt-auth/username")
MQTT_PASS=$(op read "op://Automation/environment-measures-mqtt-auth/password")

# 2. Hash the password and write to the passwd file
PASS_HASH=$(openssl passwd -6 -salt yoursalt12345 "$MQTT_PASS" 2>/dev/null || openssl passwd -1 -salt yoursalt12345 "$MQTT_PASS")
echo "${MQTT_USER}:${PASS_HASH}" > containers/mosquitto/config/passwd

echo "Mosquitto passwd file updated."
```

Restart your broker container to apply changes:
```bash
docker compose -f containers/docker-compose.yml restart mosquitto
```

## 3. Run the Python Client using 1Password Credentials

Directly fetch credentials inline using `op` when executing the Python CLI:

```bash
poetry run environment read --demo \
  --mqtt-host localhost \
  --mqtt-ca containers/mosquitto/config/certs/ca.crt \
  --tls-insecure \
  --mqtt-user "$(op read "op://Automation/environment-measures-mqtt-auth/username")" \
  --mqtt-password "$(op read "op://Automation/environment-measures-mqtt-auth/password")"
```

---

## 4. Inject Credentials into Zephyr Firmware (`local.conf`)

Add the credentials from 1Password into your Git-ignored firmware overlay config (`firmware/local.conf`):

```bash
# 1. Fetch credentials from 1Password
MQTT_USER=$(op read "op://Automation/environment-measures-mqtt-auth/username")
MQTT_PASS=$(op read "op://Automation/environment-measures-mqtt-auth/password")

# 2. Append/Write to firmware/local.conf
cat <<EOF >> firmware/local.conf

# MQTT Credentials from 1Password
CONFIG_APP_MQTT_USERNAME="${MQTT_USER}"
CONFIG_APP_MQTT_PASSWORD="${MQTT_PASS}"
EOF

echo "Firmware local.conf updated."
```

Rebuild and flash the firmware.

---

## 5. Securely Injecting Telegram Credentials into Alertmanager

Alertmanager doesn't support environment variable expansion natively by default, and using template preprocessors can be complex. Instead, we use Alertmanager's native `bot_token_file` feature to load the bot token from a file mounted into the container.

### One-time Setup

1. Retrieve the Telegram Bot Token from 1Password and write it to the git-ignored token file:
   ```bash
   op read "op://Automation/environment-measures-telegram/credential" > containers/alertmanager/telegram_token
   ```

2. Confirm that `containers/alertmanager/alertmanager.yml` is configured to read the token file and uses your `chat_id`:
   ```yaml
   receivers:
   - name: 'telegram'
     telegram_configs:
     - bot_token_file: '/etc/alertmanager/telegram_token'
       chat_id: 8584821168
   ```

3. Confirm that `containers/docker-compose.yml` mounts the token file into the Alertmanager container:
   ```yaml
     alertmanager:
       volumes:
         - ./alertmanager/alertmanager.yml:/etc/alertmanager/alertmanager.yml
         - ./alertmanager/telegram_token:/etc/alertmanager/telegram_token
   ```

### Start the Stack

Now, you can start the stack normally using standard Docker Compose:
```bash
docker compose -f containers/docker-compose.yml up -d
```

### Verify

Check the Alertmanager logs to ensure it starts cleanly:
```bash
docker compose -f containers/docker-compose.yml logs alertmanager
```
