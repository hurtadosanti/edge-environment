# Infrastructure Provisioning Guide

This directory contains the Terraform configuration for deploying the environment measures infrastructure on Google Cloud Platform (GCP), split into network and services.

---

## Directory Structure

*   **[`network/`](./network):** Provisions the custom VPC, subnetwork, firewall rules, and standard-tier static IP.
*   **[`services/`](./services):** Provisions Secret Manager secrets and the Compute Engine VM instance, referencing network resources via lookup.
*   **[`containers/`](./containers):** Observable collector stack configs (Mosquitto, Telegraf, VictoriaMetrics, vmalert, Alertmanager).

---

## Prerequisites

1.  **gcloud CLI:** Install and initialize the CLI:
    ```bash
    gcloud init
    ```
2.  **Authentication:** Authenticate to establish the local session:
    ```bash
    gcloud auth login
    ```
3.  **Active Project:** Ensure the CLI is pointed to your project:
    ```bash
    gcloud config set project metric-link
    ```

*Note: Terraform will automatically retrieve the active `gcloud` token and project ID during execution, eliminating the need for hardcoded credentials.*

---

## Getting Started

Deploy the layers in order (network first, then services):

### 1. Provision the Network Layer
```bash
cd infrastructure/network
terraform init
terraform plan
terraform apply
```

### 2. Provision the Services Layer
By default, this provisions a smaller `e2-micro` instance. To provision a larger dual-core `e2-medium` instance (e.g. for provisioning tasks):
```bash
cd ../services
terraform init
terraform plan -var="gcp_machine_type=e2-medium"
terraform apply -var="gcp_machine_type=e2-medium"
```

To run with the default micro instance:
```bash
terraform apply
```

---

## Retrieving the Static IP Address

You can fetch the allocated static IP address at any time using either Terraform or the `gcloud` CLI:

*   **Using Terraform (inside network folder):**
    ```bash
    cd infrastructure/network && terraform output static_ip_address
    ```
*   **Using gcloud CLI:**
    ```bash
    gcloud compute addresses describe metric-link-ip-us-east1-mqtt --region=us-east1 --format="value(address)"
    ```

---

## Deploying and Testing the Services

An `infrastructure/Makefile` is provided to automate local testing, credential management, and cloud deployment.

---

### Local Testing

To test the entire container stack (Mosquitto, VictoriaMetrics, Telegraf, vmalert, Alertmanager) locally:

1. **Start the local stack**:
   This runs Docker Compose with a local override that exposes Mosquitto's unencrypted port `1883` directly to your localhost:
   ```bash
   cd infrastructure
   make local-up
   ```
2. **Test local publishing**:
   You can publish to `localhost:1883` anonymously without SSL (e.g., using `mqttx` or `mosquitto_pub`):
   ```bash
   mqttx pub -t "test/topic" -m "hello local" -h "localhost" -p 1883
   ```
3. **Stop the local stack**:
   ```bash
   make local-down
   ```

---

### Cloud Deployment (GCP VM with Traefik Edge TLS)

Deploying to the cloud VM uses **Traefik** to automatically request and rotate a Let's Encrypt TLS certificate for your public domain on port `8883`. The domain is dynamically fetched from your 1Password vault or CLI variables.

#### 1. Setup DNS (GoDaddy)
Ensure you have added an `A` record for your subdomain (e.g., `metrics.yourdomain.com`) pointing to the VM's static IP `35.207.44.241` in your DNS/GoDaddy Control Panel.

#### 2. Provision Credentials
Keep your plaintext passwords out of the repository. Generate a hashed password file locally:
```bash
cd infrastructure
make hash-passwd
```
*Note: This command dynamically retrieves your username and password from 1Password and hashes them into `containers/mosquitto/config/password_file` using a temporary Docker container.*

#### 3. Deploy to the VM
The deploy target syncs configurations via `rsync` (excluding data/log folders) and restarts the docker services on the VM. It will automatically resolve the VM IP via `gcloud` if not specified:
```bash
make deploy
```
*If you need to specify a custom key, user, or IP:*
```bash
make deploy VM_IP=35.207.44.241 SSH_USER=shurtado SSH_KEY=~/.ssh/google_compute_engine
```

---

### Testing the Cloud Broker Connection

Since the cloud broker uses Let's Encrypt certificates, standard clients already trust the certificate authority.

#### Fetch credentials and domain from 1Password
```bash
export MQTT_USER=$(op read "op://Automation/environment-measures-mqtt-auth/username")
export MQTT_PASS=$(op read "op://Automation/environment-measures-mqtt-auth/password")
export MQTT_DOMAIN=$(op read "op://Automation/environment-measures-mqtt-auth/domain")
```

#### Option A: Using MQTTX CLI
*   **Subscribe to a topic**:
    ```bash
    mqttx sub -t "test/topic" -h "$MQTT_DOMAIN" -p 8883 -u "$MQTT_USER" -P "$MQTT_PASS" --protocol mqtts
    ```
*   **Publish to a topic**:
    ```bash
    mqttx pub -t "test/topic" -m "hello cloud" -h "$MQTT_DOMAIN" -p 8883 -u "$MQTT_USER" -P "$MQTT_PASS" --protocol mqtts
    ```

#### Option B: Using Mosquitto Client Tools
*   **Subscribe**:
    ```bash
    mosquitto_sub -h $MQTT_DOMAIN -p 8883 -u "$MQTT_USER" -P "$MQTT_PASS" -t "test/topic" -d
    ```
*   **Publish**:
    ```bash
    mosquitto_pub -h $MQTT_DOMAIN -p 8883 -u "$MQTT_USER" -P "$MQTT_PASS" -t "test/topic" -m "hello cloud" -d
    ```

---

### Firmware (Pico W / Zephyr) Integration

To compile the Let's Encrypt root CA certificate into the firmware:
1. **Download and format the Root CA**:
   This downloads the `ISRG Root X1` certificate PEM and formats it as a C-string header in `firmware/src/ca_cert.inc`:
   ```bash
   cd infrastructure
   make get-root-ca
   ```
2. Build the firmware using `west` as usual. The firmware will now successfully trust and verify Traefik's Let's Encrypt certificate when connecting to your configured domain on port `8883`.


