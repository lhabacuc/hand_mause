import cv2
import pyautogui
import time
import mediapipe as mp
from camera import init_camera
from utils import dist

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
