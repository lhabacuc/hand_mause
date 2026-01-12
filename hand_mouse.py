#!/usr/bin/env python3
import argparse
from config import load_config
from mediapipe_utils import setup_mediapipe
from control import control_loop
from utils import download_model

CONFIG_FILE = "hand_mouse.ini"

def parse_args():
	import argparse
	parser = argparse.ArgumentParser(description="Controle do mouse com a mão usando MediaPipe")
	parser.add_argument('--background', action='store_true', help="Executa sem abrir janela de vídeo")
	return parser.parse_args()

def main():
	args = parse_args()
	download_model('hand_landmarker.task')
	config = load_config(CONFIG_FILE)
	with setup_mediapipe('hand_landmarker.task') as landmarker:
		control_loop(landmarker, config, background=args.background)

if __name__ == "__main__":
	main()
