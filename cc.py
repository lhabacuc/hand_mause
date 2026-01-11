import cv2
import mediapipe as mp
import pyautogui
import time

# Configurações
SCREEN_WIDTH, SCREEN_HEIGHT = pyautogui.size()
PINCH_THRESHOLD = 0.04  # Ajustado - mais sensível
CLICK_MAX_DURATION = 0.7  # Click rápido
DRAG_MIN_DURATION = 0.8  # Mínimo para começar drag

# Resolução da câmera
CAM_WIDTH = 640
CAM_HEIGHT = 480

# Suavização
SMOOTHING = 0.5
prev_x, prev_y = SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2

# Estados
pinch_active = False
pinch_start_time = 0.5
dragging = False

# MediaPipe setup
BaseOptions = mp.tasks.BaseOptions
HandLandmarker = mp.tasks.vision.HandLandmarker
HandLandmarkerOptions = mp.tasks.vision.HandLandmarkerOptions
VisionRunningMode = mp.tasks.vision.RunningMode

options = HandLandmarkerOptions(
    base_options=BaseOptions(model_asset_path='hand_landmarker.task'),
    running_mode=VisionRunningMode.VIDEO,
    num_hands=1,
    min_hand_detection_confidence=0.5,
    min_hand_presence_confidence=0.5,
    min_tracking_confidence=0.5
)

cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, CAM_WIDTH)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT)
cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)

pyautogui.FAILSAFE = False
pyautogui.PAUSE = 0

def dist(a, b):
    return ((a[0]-b[0])**2 + (a[1]-b[1])**2)**0.5

import urllib.request
import os

model_path = 'hand_landmarker.task'
if not os.path.exists(model_path):
    print("Baixando modelo...")
    url = 'https://storage.googleapis.com/mediapipe-models/hand_landmarker/hand_landmarker/float16/1/hand_landmarker.task'
    urllib.request.urlretrieve(url, model_path)
    print("Modelo baixado!")

print("=" * 50)
print("CONTROLE DE MÃO ATIVO")
print("=" * 50)
print("• Mova o dedo indicador para mover o cursor")
print("• Junte polegar + indicador = CLICK")
print("• Mantenha junto > 0.3s = ARRASTAR")
print("• ESC = Sair")
print("=" * 50)

with HandLandmarker.create_from_options(options) as landmarker:
    frame_count = 0
    
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
            
            # Desenhar pontos na tela (feedback visual)
            thumb_px = (int(thumb.x * w), int(thumb.y * h))
            index_px = (int(index.x * w), int(index.y * h))
            
            cv2.circle(frame, thumb_px, 10, (0, 255, 0), -1)
            cv2.circle(frame, index_px, 10, (255, 0, 0), -1)
            cv2.line(frame, thumb_px, index_px, (255, 255, 0), 2)
            
            # Calcular distância
            hand_size = dist((wrist.x, wrist.y), (middle.x, middle.y))
            
            if hand_size > 0.05:
                # Distância absoluta (em pixels da imagem)
                pinch_dist_px = dist(thumb_px, index_px)
                # Distância normalizada
                pinch_dist = dist((thumb.x, thumb.y), (index.x, index.y))
                
                # Mostrar distância na tela
                cv2.putText(frame, f"Dist: {pinch_dist:.3f}", (10, 30), 
                           cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
                
                # Calcular posição do cursor
                new_x = (index.x) * SCREEN_WIDTH
                new_y = index.y * SCREEN_HEIGHT
                
                smooth_x = int(prev_x * SMOOTHING + new_x * (1 - SMOOTHING))
                smooth_y = int(prev_y * SMOOTHING + new_y * (1 - SMOOTHING))
                
                # Mover cursor
                pyautogui.moveTo(smooth_x, smooth_y, _pause=False)
                prev_x, prev_y = smooth_x, smooth_y
                
                # Detectar pinça
                now = time.time()
                is_pinch = pinch_dist < PINCH_THRESHOLD
                
                # Estado visual
                if is_pinch:
                    cv2.putText(frame, "PINCH ATIVO!", (10, 60), 
                               cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
                
                # Início da pinça
                if is_pinch and not pinch_active:
                    pinch_active = True
                    pinch_start_time = now
                    print(f"[PINCH] Iniciado - dist: {pinch_dist:.3f}")
                
                # Manter pinça - verificar se deve arrastar
                if is_pinch and pinch_active:
                    duration = now - pinch_start_time
                    if duration > DRAG_MIN_DURATION and not dragging:
                        # Iniciar arrasto
                        pyautogui.mouseDown(_pause=False)
                        dragging = True
                        print(f"[DRAG] Iniciado - duração: {duration:.2f}s")
                        cv2.putText(frame, "ARRASTANDO!", (10, 90), 
                                   cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)
                
                # Soltar pinça
                if not is_pinch and pinch_active:
                    duration = now - pinch_start_time
                    
                    if dragging:
                        # Terminar arrasto
                        pyautogui.mouseUp(_pause=False)
                        print(f"[DRAG] Terminado - duração total: {duration:.2f}s")
                        dragging = False
                    elif duration < CLICK_MAX_DURATION:
                        # Click rápido
                        pyautogui.click(_pause=False)
                        print(f"[CLICK] Executado - duração: {duration:.2f}s")
                    
                    pinch_active = False
        
        # Mostrar estado atual
        status = "ARRASTANDO" if dragging else ("PINCH" if pinch_active else "LIVRE")
        cv2.putText(frame, f"Estado: {status}", (10, h - 20), 
                   cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
        
        # Mostrar frame (essencial para debug)
        cv2.imshow("Hand Control", frame)
        
        frame_count += 1
        
        if cv2.waitKey(1) & 0xFF == 27:
            break
    
    # Garantir que solta o mouse ao sair
    if dragging:
        pyautogui.mouseUp()

cap.release()
cv2.destroyAllWindows()
print("Encerrado.")
