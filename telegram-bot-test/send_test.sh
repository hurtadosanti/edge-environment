#!/bin/bash
set -e

# Resolve 1Password secret references if provided
if [[ "$BOT_TOKEN" =~ ^op:// ]]; then
    echo "Resolving BOT_TOKEN from 1Password..."
    BOT_TOKEN=$(op read "$BOT_TOKEN")
fi
if [[ "$CHAT_ID" =~ ^op:// ]]; then
    echo "Resolving CHAT_ID from 1Password..."
    CHAT_ID=$(op read "$CHAT_ID")
fi

if [ -z "$BOT_TOKEN" ] || [ -z "$CHAT_ID" ]; then
    echo "Error: BOT_TOKEN and CHAT_ID environment variables must be set."
    exit 1
fi

MESSAGE="${MESSAGE:-"Hello! This is a test message from the environment-measures Telegram bot test script."}"

echo "Sending test message to Telegram chat ${CHAT_ID}..."
RESPONSE=$(curl -s -X POST "https://api.telegram.org/bot${BOT_TOKEN}/sendMessage" \
  -d "chat_id=${CHAT_ID}" \
  -d "text=${MESSAGE}")

if echo "$RESPONSE" | grep -q '"ok":true'; then
    echo "Success! Message sent successfully."
else
    echo "Failed to send message. Telegram API response:"
    echo "$RESPONSE"
    exit 1
fi
