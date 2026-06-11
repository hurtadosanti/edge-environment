terraform {
  required_version = ">= 1.0"
  required_providers {
    google = {
      source  = "hashicorp/google"
      version = "~> 5.0"
    }
  }
}

provider "google" {
  project = "your-gcp-project-id" # Replace with your exact GCP Project ID
  region  = "us-central1"         # Must be us-central1, us-east1, or us-west1 for Free Tier
  zone    = "us-central1-a"
}

# 1. Isolated VPC Network
resource "google_compute_network" "vpc_network" {
  name                    = "telemetry-vpc"
  auto_create_subnetworks = true
}

# 2. Firewall: Allow Inbound MQTT traffic to Mosquitto
resource "google_compute_firewall" "allow_mqtt" {
  name    = "allow-mqtt"
  network = google_compute_network.vpc_network.name

  allow {
    protocol = "tcp"
    ports    = ["1883"]
  }

  # Replace with specific CIDR blocks (e.g. your home/edge IP) to maximize security
  source_ranges = ["0.0.0.0/0"] 
}

# 3. Firewall: Optional Inbound access for VictoriaMetrics UI / Grafana
resource "google_compute_firewall" "allow_metrics_ui" {
  name    = "allow-metrics-ui"
  network = google_compute_network.vpc_network.name

  allow {
    protocol = "tcp"
    ports    = ["8428"] # Default VictoriaMetrics single-node port
  }

  # Strongly recommended to restrict this block exclusively to your public IP
  source_ranges = ["0.0.0.0/0"] 
}

# 4. Compute Engine Instance (Strictly Optimized for Free Tier Constraints)
resource "google_compute_instance" "telemetry_server" {
  name         = "vm-iot-stack"
  machine_type = "e2-micro" # 100% Free-tier eligible
  zone         = "us-central1-a"

  boot_disk {
    initialize_params {
      # Lightweight Debian 12 Image maintained by Google
      image = "debian-cloud/debian-12" 
      size  = 30 # Maximum allowed gigabytes before incurring storage costs
      type  = "pd-standard" # Standard mechanical disk required for free tier
    }
  }

  network_interface {
    network = google_compute_network.vpc_network.name
    
    access_config {
      # CRITICAL: Must be STANDARD. Premium network routing incurs costs.
      network_tier = "STANDARD" 
    }
  }

  # Ensures safety mechanisms if you accidentally target a non-free configuration 
  lifecycle {
    prevent_destroy = false
  }

  metadata = {
    # Bootstrap script to cleanly install docker and docker-compose
    startup-script = <<-EOT
      #!/bin/bash
      apt-get update
      apt-get install -y apt-transport-https ca-certificates curl gnupg lsb-release docker.io docker-compose
      systemctl enable docker
      systemctl start docker
    EOT
  }
}

# Output the Public IP address to connect your edge devices to Mosquitto
output "public_ip" {
  value       = google_compute_instance.telemetry_server.network_interface[0].access_config[0].assigned_nat_ip
  description = "The public IP address of your telemetry instance."
}