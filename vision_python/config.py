import os

CONFIG_PATH = "config/hand_mouse.conf"

def load_config():
    """Carrega config do arquivo"""
    config = {}
    if not os.path.exists(CONFIG_PATH):
        return config
    with open(CONFIG_PATH, "r") as f:
        for line in f:
            line = line.strip()
            if line == "" or line.startswith("#"):
                continue
            if "=" in line:
                key, value = line.split("=", 1)
                key = key.strip()
                value = value.strip()
                try:
                    if "." in value:
                        value = float(value)
                    else:
                        value = int(value)
                except ValueError:
                    pass
                config[key] = value
    return config

def save_config(config):
    """Salva config no arquivo"""
    os.makedirs(os.path.dirname(CONFIG_PATH), exist_ok=True)
    with open(CONFIG_PATH, "w") as f:
        f.write("# Configurações do hand_mouse\n")
        for key, value in config.items():
            f.write(f"{key}={value}\n")
