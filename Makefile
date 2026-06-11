ifneq (,$(wildcard ./.env))
    include .env
    export
endif

# Default values if not specified in .env
RPI_HOST ?= 192.168.1.95
RPI_DIR ?= ~/projects/environment-measures
CLOUD_HOST ?= santiago_hurtado@cloud-dev-01
CLOUD_DIR ?= ~/environment-measures/containers

.PHONY: push
push:
	rsync -avz --rsync-path="mkdir -p $(RPI_DIR) && rsync" \
		--exclude '.git' \
		--exclude '.venv' \
		--exclude '__pycache__' \
		--exclude '.pytest_cache' \
		--exclude '.ruff_cache' \
		--exclude 'build' \
		--exclude '.env' \
		--exclude 'infrastructure/containers/victoriametrics-data' \
		-e ssh ./ $(RPI_HOST):$(RPI_DIR)/

.PHONY: cloud-push
cloud-push:
	rsync -avz --rsync-path="mkdir -p $(CLOUD_DIR) && rsync" \
		--exclude 'mosquitto/data' \
		--exclude 'mosquitto/log' \
		--exclude 'victoriametrics-data' \
		-e ssh ./infrastructure/containers/ $(CLOUD_HOST):$(CLOUD_DIR)/

.PHONY: clear
clear:
	rm -rf infrastructure/containers/mosquitto/data infrastructure/containers/mosquitto/log infrastructure/containers/victoriametrics-data

.PHONY: build
build:
	west build -p always -b rpi_pico/rp2040/w firmware
