import configparser
import os

def load_config(config_path: str):
	config = configparser.ConfigParser()
	if not os.path.exists(config_path):
		config['HAND_MOUSE'] = {
			'pinch_threshold': '0.04',
			'click_max_duration': '0.7',
			'drag_min_duration': '0.8',
			'smoothness': '0.5',
			'camera_width': '640',
			'camera_height': '480',
			'speed_multiplier': '1.5'
		}
		with open(config_path, 'w') as f:
			config.write(f)
	else:
		config.read(config_path)
	return config['HAND_MOUSE']
