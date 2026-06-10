ifneq (,$(wildcard ./.env))
    include .env
    export
endif

# Default values if not specified in .env
RPI_HOST ?= rpi-dev-01
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
		-e ssh ./ $(RPI_HOST):$(RPI_DIR)/

.PHONY: cloud-push
cloud-push:
	rsync -avz --rsync-path="mkdir -p $(CLOUD_DIR) && rsync" \
		--exclude 'mosquitto/data' \
		--exclude 'mosquitto/log' \
		--exclude 'victoriametrics-data' \
		-e ssh ./containers/ $(CLOUD_HOST):$(CLOUD_DIR)/

.PHONY: clear
clear:
	rm -rf containers/mosquitto/data containers/mosquitto/log containers/victoriametrics-data
