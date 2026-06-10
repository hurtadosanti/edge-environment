# Compile and Test Firmware

Instructions to compile and test the Zephyr-based firmware. All commands below assume you are navigating to the appropriate project folders first to keep paths short and simple.

---

## 1. Activate Environment

First, activate the Python virtual environment and source the Zephyr environment variables:

```bash
source ~/zephyrproject/.venv/bin/activate
source ~/zephyrproject/zephyr/zephyr-env.sh
```

---

## 2. Compile Firmware

1. Navigate to the `firmware` directory:
   ```bash
   cd firmware
   ```
2. Build the main application for the Raspberry Pi Pico W:
   * **First build or config changes** (clean rebuild):
     ```bash
     west build -p always -b rpi_pico/rp2040/w
     ```
   * **Subsequent changes** (fast incremental build of modified code):
     ```bash
     west build
     ```

---

## 3. Run Unit Tests

1. Navigate to the `tests/filter` directory:
   ```bash
   cd firmware/tests/filter
   ```
2. Build and run the ZTest suite in the host simulator (`native_sim`):
   ```bash
   # Build the test project
   west build -p always -b native_sim

   # Run the test executable
   ./build/zephyr/zephyr.exe
   ```

---

## 4. Flash the Firmware

To upload the compiled firmware to the Raspberry Pi Pico W, choose one of the following methods:

### Option A: Using the Debug Probe (SWD)
Ensure any running background instance of the OpenOCD server is stopped (to prevent USB conflicts).

From the `firmware` directory:
```bash
cd firmware
west flash
```
---

## 5. Verify and Run

Once the Pico W is flashed, verify its operation:

### 1. Monitor the Serial Console
Connect a serial terminal to read the console logs (e.g. WiFi connection status and BME280 sensor data):
```bash
# Using tio (highly recommended, supports auto-reconnect)
tio /dev/ttyACM0

```

### 2. Start the MQTT Broker
Make sure your local MQTT broker is running (see [MQTT Examples](02-python-usage.md#mqtt-broker-examples) for more details):
```bash
docker compose -f containers/mosquitto/docker-compose.yml up -d
```

### 3. Subscribe to the Sensor Topic
Subscribe to the topic defined in `prj.conf` to monitor publications:
```bash
# Using mosquitto_sub
mosquitto_sub -h localhost -p 1883 -t "sensors/bme280"
```
