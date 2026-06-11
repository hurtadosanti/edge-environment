# Firmware Developer Guide

Instructions to set up, build, flash, and debug the Zephyr-based firmware for the Raspberry Pi Pico W.

---

## 1. Zephyr Environment Setup

### Install Host Dependencies

* **macOS**:
  ```bash
  brew install cmake ninja gperf python3 python-tk ccache qemu dtc libmagic wget openocd
  ```
* **Linux (Debian/Ubuntu/Raspberry Pi OS)**:
  ```bash
  sudo apt update && sudo apt install -y git cmake ninja-build gperf ccache dfu-util \
    device-tree-compiler wget python3-dev python3-pip python3-setuptools \
    python3-tk python3-wheel python3-venv xz-utils file make gcc libsdl2-dev libmagic1 openocd
  ```

### Initialize Workspace & Toolchain
```bash
# 1. Create venv and workspace
mkdir ~/zephyrproject && cd ~/zephyrproject
python3 -m venv .venv
source .venv/bin/activate
pip install west

# 2. Fetch dependencies & toolchain
west init .
west update --narrow cmsis_6 hal_rpi_pico hal_infineon mbedtls tf-psa-crypto
west zephyr-export
west packages pip --install
west blobs fetch hal_infineon

cd zephyr
west sdk install --toolchains arm-zephyr-eabi --version 1.0.1
```

---

## 2. Compile & Flash

Ensure your virtual environment is active:
```bash
source ~/zephyrproject/.venv/bin/activate
source ~/zephyrproject/zephyr/zephyr-env.sh
```

### Compile Firmware
From the `firmware` directory:
```bash
# Clean build (mandatory for config changes)
west build -p always -b rpi_pico/rp2040/w

# Incremental build
west build
```

### Flash Firmware
Connect the target via SWD Debug Probe and run:
```bash
west flash
```

---

## 3. Run Unit Tests

Build and run the ZTest suite on the host simulator (`native_sim`):
```bash
cd firmware/tests/filter
west build -p always -b native_sim
./build/zephyr/zephyr.exe
```

---

## 4. Hardware Debugging & Console

Connect the CMSIS-DAP Debug Probe to the host via USB.

### Configure Linux Permissions (udev)
```bash
# Download and copy openocd udev rules
wget https://raw.githubusercontent.com/zephyrproject-rtos/openocd/master/contrib/60-openocd.rules
sudo cp 60-openocd.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules && sudo udevadm trigger
```

### Run OpenOCD GDB Server
```bash
openocd -f interface/cmsis-dap.cfg -c "transport select swd" -f target/rp2040.cfg
```

### Connect GDB Debugger
```bash
arm-zephyr-eabi-gdb -ex "target remote localhost:3333"
```

### Monitor Serial Console
Connect a serial terminal to view the microcontroller logs:
```bash
tio /dev/ttyACM0
```
