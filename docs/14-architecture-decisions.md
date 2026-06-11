# Architecture Decision Records (ADR)

This document aggregates the core architectural decisions made for the **Environment Measures** telemetry platform.

---

## ADR 1: Telemetry & Ingestion Broker Selection

### Context
We need a lightweight, production-grade telemetry broker to act as the primary ingestion endpoint for edge devices (both Zephyr-based microcontrollers and Python-based clients). The broker must run in resource-constrained environments (like a single GCP e2-micro VM).

### Decision
**Adopt Eclipse Mosquitto** as the primary MQTT broker.
*   **Security**: Authentication is enforced using a local, hashed password file (`passwd`) generated from 1Password credentials. 
*   **Performance**: Extremely low footprint (< 10MB RAM in container).
*   **Transport**: Plain MQTT (port 1883) for the initial phase, with options for TLS wrapping.

---

## ADR 2: Observability & Alerting Stack

### Context
The platform requires time-series storage, alert evaluation, and alert routing (notifications). The backend must be low-overhead, offline-resilient, and cost-effective when running on a free-tier virtual machine.

### Decision
**Adopt the Victoria Stack (VictoriaMetrics + vmalert + Alertmanager)**.

| Component | Role | Resource Footprint |
| :--- | :--- | :--- |
| **Telegraf** | Scraping, protocol translation (MQTT to Influx/VM) | Low (< 30 MB) |
| **VictoriaMetrics** | Single-node time-series database | Low (< 50 MB) |
| **vmalert** | Metric evaluation & alert rule engine | Low (< 15 MB) |
| **Alertmanager** | Alert grouping, deduplication, and routing | Low (< 20 MB) |

### Key Trade-offs & Integrations
*   **MetricsQL**: We use `vmalert` with MetricsQL (PromQL-compatible) to check threshold, sudden-change, and sensor-offline metrics.
*   **Telegram Integration**: Alertmanager routes alerts to Telegram via bot integration.

> [!NOTE]
> We reject heavy observability stacks like managed Cloud SaaS or multi-node Prometheus/Grafana clusters at the edge to remain within the Google Cloud Free Tier constraints.

---

## ADR 3: Cloud Infrastructure & Budget Constraints

### Context
To maintain a cost-effective staging/production environment, the cloud-side deployment must run continuously within the GCP Free Tier limits.

### Decision
**Deploy a single-node virtual machine via Terraform on GCP Free Tier.**
*   **VM Instance**: Compute Engine `e2-micro` (2 vCPUs, 1 GB RAM).
*   **Storage**: 30 GB standard persistent mechanical disk (`pd-standard`) with Debian 12.
*   **Networking**: Standard Tier network routing to avoid premium network egress costs.
*   **Provisioning**: Automated via Terraform with Docker and Docker Compose installed on startup.

---

## ADR 4: Secret Management and Credential Isolation

### Context
Secret credentials (MQTT usernames/passwords, Telegram bot tokens, API keys) must never be committed to version control. However, configuration files and firmware builds need access to these secrets at runtime/build-time.

### Decision
**Inject secrets dynamically using the 1Password (`op`) CLI.**

*   **Firmware (`local.conf`)**: Credentials are loaded from 1Password and written to `firmware/local.conf`, which is explicitly git-ignored.
*   **Mosquitto Passwd**: Hashes are dynamically calculated using `openssl passwd` from credentials fetched by `op` CLI and written to the password file.
*   **Alertmanager Bot Token**: Alertmanager does not support environment variable expansion natively by default. We use Alertmanager's native `bot_token_file` feature to load the Telegram bot token from a file (`telegram_token`) mounted into the container. This token is written dynamically from 1Password.

> [!IMPORTANT]
> All local secret credentials, local config files, and generated tokens must be excluded from Git via `.gitignore`.
