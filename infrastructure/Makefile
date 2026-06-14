# Configurations
SSH_KEY ?= ~/.ssh/google_compute_engine
SSH_USER ?= $(shell id -un)
VM_IP ?= $(shell gcloud compute addresses describe metric-link-ip-us-east1-mqtt --region=us-east1 --format="value(address)" 2>/dev/null)

# Retrieve variables dynamically from 1Password
MQTT_DOMAIN ?= $(shell op read "op://Automation/environment-measures-mqtt-auth/domain" 2>/dev/null)
LE_EMAIL ?= $(shell op read "op://Automation/environment-measures-mqtt-auth/email" 2>/dev/null)
MQTT_USER ?= $(shell op read "op://Automation/environment-measures-mqtt-auth/username" 2>/dev/null)
MQTT_PASS ?= $(shell op read "op://Automation/environment-measures-mqtt-auth/password" 2>/dev/null)

# If email isn't in 1Password, default to admin@<MQTT_DOMAIN>
ifeq ($(LE_EMAIL),)
  ifneq ($(MQTT_DOMAIN),)
    LE_EMAIL = admin@$(MQTT_DOMAIN)
  endif
endif

.PHONY: help
help:
	@echo "Available commands:"
	@echo "  make get-root-ca         Download ISRG Root X1 for Pico firmware compilation"
	@echo "  make hash-passwd         Generate/Append user/pass credentials interactively"
	@echo "  make local-up            Start the local stack with port 1883 exposed"
	@echo "  make local-down          Stop the local stack"
	@echo "  make push                Sync container configurations to the VM"
	@echo "  make deploy              Push configurations and restart the cloud stack"

.PHONY: get-root-ca
get-root-ca:
	@if [ -z "$(MQTT_DOMAIN)" ]; then \
		echo "Error: MQTT_DOMAIN is not set and could not be resolved from 1Password."; \
		exit 1; \
	fi
	@mkdir -p ../firmware/src
	curl -s https://letsencrypt.org/certs/isrgrootx1.pem | awk '{print "\"" $$0 "\\n\""}' > ../firmware/src/ca_cert.inc
	@echo "ISRG Root X1 CA downloaded and formatted into: firmware/src/ca_cert.inc"

.PHONY: hash-passwd
hash-passwd:
	@if [ -z "$(MQTT_USER)" ] || [ -z "$(MQTT_PASS)" ]; then \
		echo "Error: MQTT_USER or MQTT_PASS is not set and could not be resolved from 1Password."; \
		exit 1; \
	fi
	@mkdir -p containers/mosquitto/config
	@rm -f containers/mosquitto/config/password_file
	docker run --rm -v $(shell pwd)/containers/mosquitto/config:/config eclipse-mosquitto:2.1-alpine \
		mosquitto_passwd -b -c /config/password_file "$(MQTT_USER)" "$(MQTT_PASS)"

.PHONY: local-up
local-up:
	@mkdir -p containers/mosquitto/config
	@touch containers/mosquitto/config/password_file
	cd containers && MQTT_DOMAIN=localhost LE_EMAIL=admin@localhost MQTT_USER="$(MQTT_USER)" MQTT_PASS="$(MQTT_PASS)" docker-compose -f docker-compose.yml -f docker-compose.local.yml up -d

.PHONY: local-down
local-down:
	cd containers && docker-compose -f docker-compose.yml -f docker-compose.local.yml down

.PHONY: push
push:
	@if [ -z "$(VM_IP)" ]; then \
		echo "Error: VM_IP is not set and could not be resolved."; \
		exit 1; \
	fi
	@mkdir -p containers/mosquitto/config
	@touch containers/mosquitto/config/password_file
	rsync -avz -e "ssh -i $(SSH_KEY) -o IdentitiesOnly=yes" \
		--exclude="victoriametrics-data/" \
		--exclude="mosquitto/data/" \
		--exclude="mosquitto/log/" \
		containers/ \
		$(SSH_USER)@$(VM_IP):~/containers/

.PHONY: deploy
deploy: push
	@if [ -z "$(MQTT_DOMAIN)" ]; then \
		echo "Error: MQTT_DOMAIN is not set. Log in to 1Password CLI or pass MQTT_DOMAIN=..."; \
		exit 1; \
	fi
	@if [ -z "$(LE_EMAIL)" ]; then \
		echo "Error: LE_EMAIL is not set."; \
		exit 1; \
	fi
	ssh -i $(SSH_KEY) -o IdentitiesOnly=yes $(SSH_USER)@$(VM_IP) \
		"echo 'MQTT_DOMAIN=$(MQTT_DOMAIN)' > ~/containers/.env && \
		 echo 'LE_EMAIL=$(LE_EMAIL)' >> ~/containers/.env && \
		 echo 'MQTT_USER=$(MQTT_USER)' >> ~/containers/.env && \
		 echo 'MQTT_PASS=$(MQTT_PASS)' >> ~/containers/.env && \
		 sudo chmod 644 ~/containers/mosquitto/config/password_file && \
		 cd ~/containers && \
		 sudo docker-compose down && \
		 sudo docker-compose up -d"
