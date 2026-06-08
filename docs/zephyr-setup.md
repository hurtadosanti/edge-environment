# Setup Zephyr

Follow these steps to set up the Zephyr development environment. Only Step 1 differs depending on your host OS; Steps 2, 3, and 4 are identical for both platforms.

---

## 1. Install Host Dependencies

Select the tab/section below corresponding to your host operating system:

### Option A: For Ubuntu / Debian / Raspberry Pi OS

Additionally, Linux requires configuring `udev` rules so your standard user account can access the USB ports for flashing and debugging hardware like the Raspberry Pi Pico without requiring `sudo` every time.

```bash
# 1a. Install packages
sudo apt update
sudo apt install -y --no-install-recommends git cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler wget \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel python3-venv \
  xz-utils file make gcc libsdl2-dev libmagic1 openocd

# (Note: On x86_64 architectures, you may also need to append gcc-multilib and g++-multilib to the list above)

# 1b. Configure udev Rules (for USB debugging)
wget https://raw.githubusercontent.com/zephyrproject-rtos/openocd/master/contrib/60-openocd.rules
sudo cp 60-openocd.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
```

### Option B: For macOS

```bash
brew install cmake ninja gperf python3 python-tk ccache qemu dtc libmagic wget openocd
```

---

## 2. Set Up Virtual Environment & Workspace

*(Applicable to both Linux and macOS)*

```bash
mkdir ~/zephyrproject
cd ~/zephyrproject
python3 -m venv .venv
source .venv/bin/activate
pip install west
```

---

## 3. Initialize and Fetch

*(Applicable to both Linux and macOS)*

```bash
west init .
west update --narrow
west blobs fetch hal_infineon
west zephyr-export
west packages pip --install
```

---

## 4. Install Toolchain

*(Applicable to both Linux and macOS)*

```bash
cd zephyr
west sdk install --toolchains arm-zephyr-eabi --version 1.0.1
```