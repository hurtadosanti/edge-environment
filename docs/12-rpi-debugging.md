# Raspberry Pi 4 Debugging (RP2040)

Instructions to connect, debug, and flash the RP2040 target from a Raspberry Pi 4 host using a Debug Probe (CMSIS-DAP).

## 1. Verify Probe Connection

Connect the Debug Probe to the Raspberry Pi 4 USB port, and ensure it is detected:

```bash
lsusb
```

Look for:
`ID 2e8a:000c Raspberry Pi Debug Probe (CMSIS-DAP)`

Check the permissions of the device node (replace `001` and `003` with Bus and Device numbers from `lsusb`):
```bash
ls -l /dev/bus/usb/001/003
```

## 2. Configure udev Rules

To run OpenOCD as a standard user without `sudo`, ensure you have configured the OpenOCD `udev` rules on your host machine.

See the **Linux Post-Install** section in the [Zephyr Setup Guide](10-zephyr-setup.md#linux-post-install-udev-rules) for detailed instructions.

## 3. Run OpenOCD

To start the OpenOCD GDB server using the correct SWD transport configuration:

```bash
openocd -f interface/cmsis-dap.cfg -c "transport select swd" -f target/rp2040.cfg
```

Successful output should end with:

```text
Info : [rp2040.core0] starting gdb server on 3333
Info : Listening on port 3333 for gdb connections
```

## 4. Verification

To verify that GDB can communicate with the target via OpenOCD, run the debugger (e.g., `gdb-multiarch` or the Zephyr SDK toolchain's GDB):

```bash
# Connect GDB to the local OpenOCD server
arm-zephyr-eabi-gdb -ex "target remote localhost:3333"
```

## 5. Troubleshooting

### Permission Denied (libusb_open failed)
* **Symptom**: OpenOCD fails to open the device with a permission error.
* **Fix**: Ensure the `udev` rules are reloaded and the probe is replugged. If the rule uses a group constraint like `plugdev`, verify your user is a member:
  ```bash
  groups
  # If plugdev is missing, add your user and log back in:
  sudo usermod -aG plugdev $USER
  ```

### JTAG not supported
* **Symptom**: `Error: CMSIS-DAP: JTAG not supported`
* **Fix**: Ensure you select SWD transport explicitly *before* loading the target configuration:
  ```bash
  openocd -f interface/cmsis-dap.cfg -c "transport select swd" -f target/rp2040.cfg
  ```

### Configuration files not found
* **Symptom**: `target/rp2040.cfg: No such file or directory`
* **Fix**: Provide the search directory using the `-s` flag to point to the openocd script path (usually `/usr/share/openocd/scripts` on Linux):
  ```bash
  openocd -s /usr/share/openocd/scripts -f interface/cmsis-dap.cfg -c "transport select swd" -f target/rp2040.cfg
  ```

