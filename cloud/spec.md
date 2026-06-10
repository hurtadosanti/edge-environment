# Cloud Deployment Specification

## Purpose

This document defines the cloud-side behavior and constraints for the environment measurements platform. It describes what the deployed system must provide, not how infrastructure is provisioned.

## Scope

The cloud deployment covers a lightweight, single-node telemetry stack that:

- receives measurements from edge devices over MQTT
- stores time-series data for later analysis
- evaluates alert rules against stored metrics
- exposes operational endpoints required for ingestion and maintenance

Infrastructure-as-code details are intentionally excluded from this specification. They belong in the deployment configuration and implementation files in this directory.

## System Overview

The initial deployment is a single virtual machine hosting the telemetry services.

```text
[Edge Devices] -- MQTT --> [Mosquitto] --> [VictoriaMetrics] <-- [vmalert]
```

## Core Components

### Mosquitto

Mosquitto acts as the ingestion entry point for edge devices.

Requirements:

- accept inbound MQTT client connections
- support the topic layout used by the firmware and Linux ingestion clients
- remain lightweight enough to operate within a small VM footprint

### VictoriaMetrics

VictoriaMetrics provides single-node storage for time-series measurements.

Requirements:

- persist telemetry data collected from the MQTT pipeline
- expose the query and ingestion interface used by the rest of the stack
- fit within the resource limits of a low-cost deployment target

### vmalert

vmalert evaluates alert conditions over the metrics stored in VictoriaMetrics.

Requirements:

- load project alert rules from version-controlled configuration
- evaluate rules on a fixed interval
- forward alert state to the configured notification path

## Deployment Baseline

The current target shape is a minimal Google Cloud deployment based on one small Compute Engine instance.

Baseline constraints:

- single VM deployment
- small machine profile suitable for low-cost or free-tier-style operation
- standard persistent boot disk
- Docker-based service runtime on Debian

This baseline is an implementation constraint, not a requirement to use any specific provisioning tool.

## Network Requirements

The deployment must provide:

- MQTT ingress for edge devices on port 1883
- VictoriaMetrics access on port 8428 for operational use

Security expectations:

- MQTT ingress should be restricted to known client networks whenever possible
- the metrics endpoint should not be left broadly open unless explicitly required for debugging
- firewall policy must be treated as part of the deployment implementation, not part of this spec

## Runtime Requirements

The cloud host must:

- install and run Docker and Docker Compose compatible tooling
- start the telemetry services automatically after provisioning or reboot
- keep service configuration outside the VM image so it can be updated without redesigning the deployment

## Configuration Boundaries

This specification does not define:

- infrastructure-as-code layout
- secret injection workflow
- local VM or emulator tooling
- provider authentication steps

Those concerns are implementation details and should be maintained in deployment documentation or automation code rather than in the product specification.

## Non-Goals

The initial cloud deployment does not attempt to provide:

- high availability across multiple nodes
- managed Kubernetes or multi-service orchestration
- autoscaling
- long-term infrastructure abstraction across multiple cloud providers

## Acceptance Criteria

The cloud deployment is considered aligned with this specification when:

- an edge device can publish measurements to the MQTT broker
- metrics are persisted and queryable in VictoriaMetrics
- alert rules are evaluated by vmalert
- the stack runs on a single low-cost VM with containerized services

