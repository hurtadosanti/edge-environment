ifneq (,$(wildcard ./.env))
    include .env
    export
endif

# Default values if not specified in .env
RPI_HOST ?= rpi-dev-01
RPI_DIR ?= ~/projects/environment-measures/firmware

.PHONY: push
push:
	rsync -avz --rsync-path="mkdir -p $(RPI_DIR) && rsync" --exclude 'build' --exclude '__pycache__' -e ssh ./firmware/ $(RPI_HOST):$(RPI_DIR)
