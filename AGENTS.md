# Environment Measures Agents

This repository contains multiple agent implementations designed to gather, filter, and transmit environmental telemetry.

---

## 1. Host Agent

* **Location:** [host-agent](file:///Users/shurtado/Projects/environment-measures/host-agent)
* **Language/Stack:** Python 3.11+, managed via [Poetry](https://python-poetry.org/)
* **Target Hardware:** Raspberry Pi 4, Linux systems, or local development environments.
* **Role:** A lightweight CLI utility that reads telemetry from I2C sensors (specifically the BME280) or simulated demo sensors, and publishes the payload to an MQTT broker.
* **Key Features:**
  - **I2C Scanner:** Commands to scan the I2C bus for active devices.
  - **Event Reporting:** Capabilities to report discrete events (e.g. windows opening or closing) with customizable timestamps.
  - **Continuous Telemetry Loop:** Runs continuously with configurable delta-filtering (temperature, humidity, pressure thresholds) to minimize MQTT transport overhead.
* **Documentation:** Refer to the [Host Agent Developer Guide](file:///Users/shurtado/Projects/environment-measures/docs/01-host-agent-guide.md).

---

## 2. Firmware Agent

* **Location:** [firmware](file:///Users/shurtado/Projects/environment-measures/firmware)
* **Language/Stack:** C++, built using the [Zephyr RTOS](https://zephyrproject.org/) framework
* **Target Hardware:** Raspberry Pi Pico W (RP2040)
* **Role:** Real-time microcontroller-based agent designed for headless edge collection. It establishes a Wi-Fi connection, reads hardware sensors, and streams telemetry directly to the MQTT broker.
* **Key Features:**
  - Embedded low-level peripheral access.
  - Hardened local MQTT publisher with support for Wi-Fi reconnection handling.
  - Built-in `ZTest` unit testing suite run via a native simulator.
* **Documentation:** Refer to the [Firmware Developer Guide](file:///Users/shurtado/Projects/environment-measures/docs/02-firmware-guide.md).

---

## 3. Telegraf Agent (Observability Bridge)

* **Location:** [infrastructure/containers/telegraf](file:///Users/shurtado/Projects/environment-measures/infrastructure/containers/telegraf)
* **Configuration:** [telegraf.conf](file:///Users/shurtado/Projects/environment-measures/infrastructure/containers/telegraf/telegraf.conf)
* **Role:** Observability bridge container. It acts as an consumer agent that subscribes to incoming telemetry topics from the MQTT broker, parses the payloads, and writes them to the VictoriaMetrics time-series database.
* **Documentation:** Refer to the [Infrastructure & Operations Guide](file:///Users/shurtado/Projects/environment-measures/docs/03-infrastructure-guide.md).
