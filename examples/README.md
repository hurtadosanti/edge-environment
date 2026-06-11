# Telegram Bot Test

A standalone testing environment to verify that your Telegram Bot token and Chat ID work correctly before integrating them with Alertmanager.

---

## Instructions

### 1. Create a Telegram Bot (if you haven't already)
1. Search for **@BotFather** in Telegram.
2. Send `/newbot` and follow the prompts to name it.
3. Save the **API Token** provided (e.g., `123456789:ABCdefGHIjklMNOpqrSTUvwxYZ`).

### 2. Get your Chat ID
1. Search for your bot in Telegram and start a chat.
2. Send a new message to it (e.g., "Hello").
3. Retrieve your Chat ID by running:
   ```bash
   export BOT_TOKEN="op://Automation/environment-measures-telegram/credential"
   curl "https://api.telegram.org/bot$(op read "$BOT_TOKEN")/getUpdates"
   ```
   *Note: If you get `{"ok":true,"result":[]}`, send another message to the bot and rerun the command immediately.*
4. Look for `"chat":{"id":XXXXXXXXX}` in the JSON response. That number is your **Chat ID**.

### 3. Run the Test Script

Make the script executable:
```bash
chmod +x send_test.sh
```

Export the environment variables and run the script:
```bash
export BOT_TOKEN="op://Automation/environment-measures-telegram/credential"
export CHAT_ID="op://Automation/environment-measures-telegram/username"
export MESSAGE="hello" # Optional

./send_test.sh
```
