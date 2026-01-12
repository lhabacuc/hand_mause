import mediapipe as mp

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
