# Compile and Test Firmware

Instructions to compile and test the Zephyr-based firmware located in [firmware](file:///Users/shurtado/Projects/environment-measures/firmware).

## 1. Activate Environment

First, activate the Python virtual environment and source the Zephyr environment variables so you can run `west` from any directory:

```bash
source ~/zephyrproject/.venv/bin/activate
source ~/zephyrproject/zephyr/zephyr-env.sh
```

## 2. Compile Firmware

Build the main application for the Raspberry Pi Pico W:

```bash
west build -p always -b rpi_pico/rp2040/w /Users/shurtado/Projects/environment-measures/firmware
```

## 3. Run Unit Tests

Build and run the ZTest suite in the host simulator (`native_sim`):

```bash
# Build the test project
west build -p always -b native_sim /Users/shurtado/Projects/environment-measures/firmware/tests/filter

# Run the test executable
./build/zephyr/zephyr.exe
```
