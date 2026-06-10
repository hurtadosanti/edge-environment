# Alerting Framework & Telegram Integration

This document describes how to set up and manage alerts for the environment monitoring system using **vmalert**, **Alertmanager**, and **Telegram**.

## Architecture Overview

1.  **VictoriaMetrics**: Stores the time-series data from sensors.
2.  **vmalert**: Periodically executes MetricsQL queries against VictoriaMetrics to detect "interesting events".
3.  **Alertmanager**: Receives alerts from vmalert, handles grouping/deduplication, and sends notifications to Telegram.

## Configuration Files

*   **`containers/docker-compose.yml`**: Defines the `vmalert` and `alertmanager` services.
*   **`containers/vmalert/rules.yml`**: Contains the alert rules (MetricsQL expressions).
*   **`containers/alertmanager/alertmanager.yml`**: Contains the notification routing and Telegram credentials.

## Setting up Telegram Notifications

To receive alerts on Telegram, you need to create a bot and obtain a Chat ID.

### 1. Create a Telegram Bot
1.  Open Telegram and search for **@BotFather**.
2.  Send the command `/newbot`.
3.  Follow the instructions to name your bot.
4.  Copy the **API Token** provided (e.g., `123456789:ABCdefGHIjklMNOpqrSTUvwxYZ`).

### 2. Get your Chat ID
1.  Start a chat with your newly created bot.
2.  Send any message to it (e.g., "Hello").
3.  Run the following command in your terminal (replace `<YOUR_BOT_TOKEN>` with your actual token):
    ```bash
    curl https://api.telegram.org/bot<YOUR_BOT_TOKEN>/getUpdates
    ```
4.  In the JSON response, look for `"chat":{"id":XXXXXXXXX...}`. That number is your **Chat ID**.

### 3. Update Alertmanager Configuration
Open `containers/alertmanager/alertmanager.yml` and update the `telegram_configs` section:

```yaml
receivers:
- name: 'telegram'
  telegram_configs:
  - bot_token: 'PASTE_YOUR_TOKEN_HERE'
    chat_id: PASTE_YOUR_CHAT_ID_HERE
```

## Creating Alert Rules

Rules are defined in `containers/vmalert/rules.yml` using **MetricsQL**.

### Example Rules

*   **Threshold Alert**: Triggers when temperature exceeds 30°C.
    ```yaml
    expr: temperature > 30
    ```
*   **Sudden Change Alert**: Triggers if temperature changes by more than 2°C in 5 minutes.
    ```yaml
    expr: abs(changes(temperature[5m])) > 2
    ```
*   **Health Check**: Triggers if no data is received for 5 minutes.
    ```yaml
    expr: absent(temperature)
    ```

## Monitoring & UI

*   **vmalert UI**: Visit `http://localhost:8880` to see the status of your rules, their last execution time, and whether they are currently firing.
*   **Alertmanager UI**: Visit `http://localhost:9093` to see active alerts and manage silences.

## Troubleshooting

1.  **Check Logs**: Run `docker-compose logs -f vmalert` or `docker-compose logs -f alertmanager`.
2.  **Verify Connectivity**: Ensure `vmalert` can reach `victoriametrics:8428` and `alertmanager:9093` within the Docker network.
