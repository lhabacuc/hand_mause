"""
Hand Detector - MediaPipe hand tracking wrapper
"""
import mediapipe as mp
import os
import urllib.request


class HandDetector:
    """Gerencia detecção de mãos usando MediaPipe"""

    THUMB_TIP = 4
    INDEX_TIP = 8
    WRIST = 0
    MIDDLE_FINGER = 12
    
    def __init__(self):
        """Inicializa o detector de mãos"""
        self.landmarker = None
        self.model_path = 'hand_landmarker.task'
        
    def download_model(self):
        """Baixa o modelo do MediaPipe se não existir"""
        if not os.path.exists(self.model_path):
            print("📥 Baixando modelo MediaPipe...")
            url = 'https://storage.googleapis.com/mediapipe-models/hand_landmarker/hand_landmarker/float16/1/hand_landmarker.task'
            urllib.request.urlretrieve(url, self.model_path)
            print("✓ Modelo baixado com sucesso!")
        return self.model_path
        
    def setup(self):
        """Configura o MediaPipe HandLandmarker"""
        model_path = self.download_model()
        
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
        
        self.landmarker = HandLandmarker.create_from_options(options)
        print("✓ MediaPipe configurado")
        
    def detect(self, mp_image, timestamp_ms):
        """Detecta mãos no frame"""
        return self.landmarker.detect_for_video(mp_image, timestamp_ms)
        
    def get_landmarks(self, hand):
        """Extrai landmarks importantes da mão"""
        return {
            'thumb': hand[self.THUMB_TIP],
            'index': hand[self.INDEX_TIP],
            'wrist': hand[self.WRIST],
            'middle': hand[self.MIDDLE_FINGER]
        }
