import os
import urllib.request

def download_model(model_path: str):
	if not os.path.exists(model_path):
		print("Baixando modelo...")
		url = 'https://storage.googleapis.com/mediapipe-models/hand_landmarker/hand_landmarker/float16/1/hand_landmarker.task'
		urllib.request.urlretrieve(url, model_path)
		print("Modelo baixado!")

def dist(a, b):
	return ((a[0]-b[0])**2 + (a[1]-b[1])**2)**0.5
