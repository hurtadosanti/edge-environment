# Setup Zephyr

## For Ubuntu

Only Step 1 changes. You must replace the macOS Homebrew command with Ubuntu's `apt` package manager and its specific native dependencies.

Additionally, Linux requires configuring `udev` rules so your standard user account can access the USB ports for flashing and debugging hardware like the Raspberry Pi Pico without requiring `sudo` every time.

### 1. Install Host Dependencies

```bash
sudo apt update
sudo apt install -y --no-install-recommends git cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler wget \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel python3-venv \
  xz-utils file make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 openocd
```

### 1b. Configure udev Rules (Ubuntu Specific)

```bash
# Download and apply standard OpenOCD udev rules for hardware debuggers
wget https://raw.githubusercontent.com/zephyrproject-rtos/openocd/master/contrib/60-openocd.rules
sudo cp 60-openocd.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Steps 2, 3, and 4 below remain exactly the same.

---

## For macOS

### 1. Install Host Dependencies

```bash
brew install cmake ninja gperf python3 python-tk ccache qemu dtc libmagic wget openocd
```

### 2. Set Up Virtual Environment & Workspace

```bash
mkdir ~/zephyrproject
cd ~/zephyrproject
python3 -m venv .venv
source .venv/bin/activate
pip install west
```

### 3. Initialize and Fetch

```bash
west init .
west update --narrow
west blobs fetch hal_infineon
west zephyr-export
west packages pip --install
```

### 4. Install Toolchain

```bash
cd zephyr
west sdk install --toolchains arm-zephyr-eabi --version 1.0.1
```