# Telemetry Queries Guide

This document defines the combined VictoriaMetrics queries. Depending on your endpoint, use either **PromQL Expressions** (for dashboards, `/api/v1/query_range`) or **Metric Selectors** (for raw export APIs, `/api/v1/export` via `match[]`).

---

## 1. Machine Statistics

### For Dashboards & Query APIs (`/api/v1/query` / `/api/v1/query_range`)
Normalizes CPU usage, Memory, and Disk to the [0 ... 1] range:
```promql
range_normalize(100 - cpu_usage_idle{cpu="cpu-total"}, mem_used_percent, disk_used_percent)
```

### For Raw Export APIs (`/api/v1/export` via `match[]`)
Selects the raw metrics:
```promql
{__name__=~"cpu_usage_idle|mem_used_percent|disk_used_percent"}
```

---

## 2. Environment Telemetry

### For Dashboards & Query APIs (`/api/v1/query` / `/api/v1/query_range`)
Normalizes Temperature, Humidity, and Pressure to the [0 ... 1] range for visual correlation:
```promql
range_normalize({__name__=~"mqtt_consumer_(pressure|temperature|humidity)"})
```

### For Raw Export APIs (`/api/v1/export` via `match[]`)
Selects the raw environmental metrics:
```promql
{__name__=~"mqtt_consumer_temperature|mqtt_consumer_humidity|mqtt_consumer_pressure"}
```

---

## 3. MQTT Broker Statistics

### For Dashboards & Query APIs (`/api/v1/query` / `/api/v1/query_range`)
Normalizes active and total MQTT clients to the [0 ... 1] range:
```promql
range_normalize(mqtt_consumer_value{topic=~"\\$SYS/broker/clients/(connected|total)"})
```

### For Raw Export APIs (`/api/v1/export` via `match[]`)
Selects the raw MQTT client metrics:
```promql
mqtt_consumer_value{topic=~"\\$SYS/broker/clients/.*"}
```

