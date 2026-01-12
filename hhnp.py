#!/usr/bin/env python3
import cv2
import mediapipe as mp
import pyautogui
import time
import configparser
import argparse
import os
import sys
import urllib.request

CONFIG_FILE = "hand_mouse.ini"

def download_model(model_path: str):
	if not os.path.exists(model_path):
		print("Baixando modelo...")
		url = 'https://storage.googleapis.com/mediapipe-models/hand_landmarker/hand_landmarker/float16/1/hand_landmarker.task'
		urllib.request.urlretrieve(url, model_path)
		print("Modelo baixado!")

def load_config(config_path: str):
	config = configparser.ConfigParser()
	if not os.path.exists(config_path):
		# Cria arquivo padrão
		config['HAND_MOUSE'] = {
			'pinch_threshold': '0.04',
			'click_max_duration': '0.7',
			'drag_min_duration': '0.8',
			'smoothness': '0.5',
			'camera_width': '640',
			'camera_height': '480',
			'speed_multiplier': '1.0'
		}
		with open(config_path, 'w') as f:
			config.write(f)
	else:
		config.read(config_path)
	return config['HAND_MOUSE']

def parse_args():
	parser = argparse.ArgumentParser(description="Controle do mouse com a mão usando MediaPipe")
	parser.add_argument('--background', action='store_true', help="Executa sem abrir janela de vídeo")
	return parser.parse_args()

def setup_mediapipe(model_path: str):
	BaseOptions = mp.tasks.BaseOptions
	HandLandmarker = mp.tasks.vision.HandLandmarker
	HandLandmarkerOptions = mp.tasks.vision.HandLandmarkerOptions
	VisionRunningMode = mp.tasks.vision.RunningMode

	options = HandLandmarkerOptions(
		base_options=BaseOptions(model_asset_path=model_path),
		running_mode=VisionRunningMode.VIDEO,
		num_hands=1,
		min_hand_detection_confidence=0.5,
		min_hand_presence_confidence=0.5,
		min_tracking_confidence=0.5
	)
	return HandLandmarker.create_from_options(options)

def dist(a, b):
	return ((a[0]-b[0])**2 + (a[1]-b[1])**2)**0.5

def init_camera(width, height):
	cap = cv2.VideoCapture(0)
	cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
	cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)
	cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
	return cap

def control_loop(landmarker, config, background=False):
	SCREEN_WIDTH, SCREEN_HEIGHT = pyautogui.size()
	prev_x, prev_y = SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2
	pinch_active = False
	dragging = False
	pinch_start_time = 0

	CAM_WIDTH = int(config.get('camera_width', 640))
	CAM_HEIGHT = int(config.get('camera_height', 480))
	PINCH_THRESHOLD = float(config.get('pinch_threshold', 0.04))
	CLICK_MAX_DURATION = float(config.get('click_max_duration', 0.7))
	DRAG_MIN_DURATION = float(config.get('drag_min_duration', 0.8))
	SMOOTHING = float(config.get('smoothness', 0.5))
	SPEED = float(config.get('speed_multiplier', 1.0))

	cap = init_camera(CAM_WIDTH, CAM_HEIGHT)
	pyautogui.FAILSAFE = False
	pyautogui.PAUSE = 0

	frame_count = 0
	print("Controle de mão ativo! Pressione ESC para sair.")

	while True:
		ret, frame = cap.read()
		if not ret:
			continue

		frame = cv2.flip(frame, 1)
		h, w, _ = frame.shape
		rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
		mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb)
		timestamp_ms = frame_count * 33
		results = landmarker.detect_for_video(mp_image, timestamp_ms)

		if results.hand_landmarks:
			hand = results.hand_landmarks[0]
			thumb = hand[4]
			index = hand[8]
			wrist = hand[0]
			middle = hand[12]

			thumb_px = (int(thumb.x * w), int(thumb.y * h))
			index_px = (int(index.x * w), int(index.y * h))
			cv2.circle(frame, thumb_px, 10, (0, 255, 0), -1)
			cv2.circle(frame, index_px, 10, (255, 0, 0), -1)
			cv2.line(frame, thumb_px, index_px, (255, 255, 0), 2)

			pinch_dist = dist((thumb.x, thumb.y), (index.x, index.y))
			new_x = int(index.x * SCREEN_WIDTH * SPEED)
			new_y = int(index.y * SCREEN_HEIGHT * SPEED)
			smooth_x = int(prev_x * SMOOTHING + new_x * (1 - SMOOTHING))
			smooth_y = int(prev_y * SMOOTHING + new_y * (1 - SMOOTHING))
			pyautogui.moveTo(smooth_x, smooth_y, _pause=False)
			prev_x, prev_y = smooth_x, smooth_y

			now = time.time()
			is_pinch = pinch_dist < PINCH_THRESHOLD

			if is_pinch and not pinch_active:
				pinch_active = True
				pinch_start_time = now
				print(f"[PINCH] Iniciado")

			if is_pinch and pinch_active:
				duration = now - pinch_start_time
				if duration > DRAG_MIN_DURATION and not dragging:
					pyautogui.mouseDown(_pause=False)
					dragging = True
					print(f"[DRAG] Iniciado")

			if not is_pinch and pinch_active:
				duration = now - pinch_start_time
				if dragging:
					pyautogui.mouseUp(_pause=False)
					dragging = False
					print(f"[DRAG] Terminado")
				elif duration < CLICK_MAX_DURATION:
					pyautogui.click(_pause=False)
					print(f"[CLICK] Executado")
				pinch_active = False

		if not background:
			status = "ARRASTANDO" if dragging else ("PINCH" if pinch_active else "LIVRE")
			cv2.putText(frame, f"Estado: {status}", (10, h - 20), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
			cv2.imshow("Hand Control", frame)

		frame_count += 1
		if cv2.waitKey(1) & 0xFF == 27:
			break

	if dragging:
		pyautogui.mouseUp()
	cap.release()
	cv2.destroyAllWindows()
	print("Encerrado.")

def main():
	args = parse_args()
	download_model('hand_landmarker.task')
	config = load_config(CONFIG_FILE)
	with setup_mediapipe('hand_landmarker.task') as landmarker:
		control_loop(landmarker, config, background=args.background)

if __name__ == "__main__":
	main()
