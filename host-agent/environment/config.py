from __future__ import annotations

import os


def load_yaml_config(file_path: str) -> dict[str, any]:
    """Loads a simple YAML config file containing key-value pairs."""
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"Config file not found: {file_path}")

    config: dict[str, any] = {}
    with open(file_path, "r", encoding="utf-8") as f:
        for line in f:
            # Remove comments
            line = line.split("#")[0].strip()
            if not line:
                continue

            # Expect key: value
            if ":" not in line:
                continue

            key, value_str = line.split(":", 1)
            key = key.strip()
            val = value_str.strip()

            # Handle quotes
            if (val.startswith('"') and val.endswith('"')) or (
                val.startswith("'") and val.endswith("'")
            ):
                val = val[1:-1]
            elif val.lower() == "true":
                val = True
            elif val.lower() == "false":
                val = False
            elif val.lower() in ("null", "none"):
                val = None
            else:
                try:
                    if "." in val:
                        val = float(val)
                    else:
                        val = int(val)
                except ValueError:
                    # Keep as string
                    pass

            config[key] = val

    return config
