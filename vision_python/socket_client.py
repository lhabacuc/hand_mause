"""
Hand Mouse Controller - Vision Module
Controla o mouse usando detecção de mãos via MediaPipe
"""
import socket
import time
import cv2
import mediapipe as mp
import os
import urllib.request
from math import sqrt
import config


class HandMouseController:
    """Controlador principal do mouse por gestos de mão"""
    
    # Índices dos landmarks da mão
    THUMB_TIP = 4
    INDEX_TIP = 8
    WRIST = 0
    MIDDLE_FINGER = 12
    
    def __init__(self):
        """Inicializa o controlador com configurações"""
        self.config = config.load_config()
        
        # Configurações
        self.screen_width = self.config.get("SCREEN_WIDTH", 1920)
        self.screen_height = self.config.get("SCREEN_HEIGHT", 1080)
        self.pinch_threshold = self.config.get("PINCH_THRESHOLD", 0.04)
        self.click_max_duration = self.config.get("CLICK_MAX_DURATION", 0.3)
        self.drag_min_duration = self.config.get("DRAG_MIN_DURATION", 0.5)
        self.smoothing = self.config.get("SMOOTHING", 0.5)
        
        # Estado do cursor
        self.prev_x = self.screen_width // 2
        self.prev_y = self.screen_height // 2
        
        # Estado da pinça
        self.pinch_active = False
        self.pinch_start_time = 0.0
        self.dragging = False
        
        # Socket
        self.sock_path = "/tmp/hand_mouse.sock"
        self.sock = None
        
        # MediaPipe
        self.landmarker = None
        self.cap = None
        
    def connect_socket(self):
        """Conecta ao socket do hand_mouse_core"""
        try:
            self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            self.sock.connect(self.sock_path)
            print(f"✓ Conectado ao socket: {self.sock_path}")
        except Exception as e:
            print(f"✗ Erro ao conectar ao socket: {e}")
            raise
            
    def send_command(self, cmd):
        """Envia comando para o hand_mouse_core"""
        try:
            self.sock.sendall((cmd + "\n").encode())
        except Exception as e:
            print(f"✗ Erro ao enviar comando '{cmd}': {e}")
            
    @staticmethod
    def distance(point_a, point_b):
        """Calcula distância euclidiana entre dois pontos"""
        return sqrt((point_a[0] - point_b[0])**2 + (point_a[1] - point_b[1])**2)
        
    def download_model(self):
        """Baixa o modelo do MediaPipe se não existir"""
        model_path = 'hand_landmarker.task'
        if not os.path.exists(model_path):
            print("📥 Baixando modelo MediaPipe...")
            url = 'https://storage.googleapis.com/mediapipe-models/hand_landmarker/hand_landmarker/float16/1/hand_landmarker.task'
            urllib.request.urlretrieve(url, model_path)
            print("✓ Modelo baixado com sucesso!")
        return model_path
        
    def setup_mediapipe(self):
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
        
    def setup_camera(self):
        """Configura a câmera"""
        self.cap = cv2.VideoCapture(0)
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
        self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
        print("✓ Câmera configurada")
        
    def process_hand_landmarks(self, hand, frame_width, frame_height):
        """Processa landmarks da mão detectada"""
        thumb = hand[self.THUMB_TIP]
        index = hand[self.INDEX_TIP]
        wrist = hand[self.WRIST]
        middle = hand[self.MIDDLE_FINGER]
        
        # Calcular distância da pinça e tamanho da mão
        pinch_dist = self.distance((thumb.x, thumb.y), (index.x, index.y))
        hand_size = self.distance((wrist.x, wrist.y), (middle.x, middle.y))
        
        return {
            'thumb': thumb,
            'index': index,
            'pinch_dist': pinch_dist,
            'hand_size': hand_size,
            'thumb_px': (int(thumb.x * frame_width), int(thumb.y * frame_height)),
            'index_px': (int(index.x * frame_width), int(index.y * frame_height))
        }
        
    def handle_cursor_movement(self, index_landmark):
        """Atualiza posição do cursor com suavização"""
        new_x = int(index_landmark.x * self.screen_width)
        new_y = int(index_landmark.y * self.screen_height)
        
        # Aplicar suavização
        smooth_x = int(self.prev_x * self.smoothing + new_x * (1 - self.smoothing))
        smooth_y = int(self.prev_y * self.smoothing + new_y * (1 - self.smoothing))
        
        self.prev_x, self.prev_y = smooth_x, smooth_y
        self.send_command(f"MOVE {smooth_x} {smooth_y}")
        
    def handle_pinch_gesture(self, is_pinch):
        """Gerencia detecção de click/drag baseado em pinça"""
        now = time.time()
        
        if is_pinch and not self.pinch_active:
            # Início da pinça
            self.pinch_active = True
            self.pinch_start_time = now
            
        elif is_pinch and self.pinch_active:
            # Manter pinça - verificar se vira drag
            duration = now - self.pinch_start_time
            if duration > self.drag_min_duration and not self.dragging:
                self.send_command("DOWN")
                self.dragging = True
                
        elif not is_pinch and self.pinch_active:
            # Soltar pinça
            duration = now - self.pinch_start_time
            if self.dragging:
                self.send_command("UP")
                self.dragging = False
            elif duration < self.click_max_duration:
                self.send_command("CLICK")
            self.pinch_active = False
            
    def draw_debug_info(self, frame, hand_data):
        """Desenha informações de debug no frame"""
        thumb_px = hand_data['thumb_px']
        index_px = hand_data['index_px']
        
        # Desenhar pontos
        cv2.circle(frame, thumb_px, 10, (0, 255, 0), -1)
        cv2.circle(frame, index_px, 10, (255, 0, 0), -1)
        cv2.line(frame, thumb_px, index_px, (255, 255, 0), 2)
        
        # Mostrar status
        status = "DRAG" if self.dragging else "PINCH" if self.pinch_active else "MOVE"
        cv2.putText(frame, status, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        
    def run(self):
        """Loop principal de execução"""
        print("=" * 50)
        print("CONTROLE DE MÃO ATIVO")
        print("Pressione ESC para sair")
        print("=" * 50)
        
        try:
            self.connect_socket()
            self.setup_mediapipe()
            self.setup_camera()
            
            frame_count = 0
            
            while True:
                ret, frame = self.cap.read()
                if not ret:
                    continue
                    
                frame = cv2.flip(frame, 1)
                h, w, _ = frame.shape
                
                # Converter para RGB e processar com MediaPipe
                rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb)
                timestamp_ms = frame_count * 33
                
                results = self.landmarker.detect_for_video(mp_image, timestamp_ms)
                
                if results.hand_landmarks:
                    hand = results.hand_landmarks[0]
                    hand_data = self.process_hand_landmarks(hand, w, h)
                    
                    # Mover cursor se mão for grande o suficiente
                    if hand_data['hand_size'] > 0.05:
                        self.handle_cursor_movement(hand_data['index'])
                    
                    # Detectar e processar pinça
                    is_pinch = hand_data['pinch_dist'] < self.pinch_threshold
                    self.handle_pinch_gesture(is_pinch)
                    
                    # Debug visual
                    self.draw_debug_info(frame, hand_data)
                
                # Mostrar frame
                cv2.imshow("Hand Control", frame)
                frame_count += 1
                
                # Sair com ESC
                if cv2.waitKey(1) & 0xFF == 27:
                    break
                    
        except KeyboardInterrupt:
            print("\n⚠ Interrompido pelo usuário")
        except Exception as e:
            print(f"✗ Erro: {e}")
        finally:
            self.cleanup()
            
    def cleanup(self):
        """Limpa recursos"""
        if self.cap:
            self.cap.release()
        cv2.destroyAllWindows()
        if self.sock:
            self.sock.close()
        print("✓ Recursos liberados")


def main():
    """Função principal"""
    controller = HandMouseController()
    controller.run()


if __name__ == "__main__":
    main()

