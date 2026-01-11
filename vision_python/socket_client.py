"""
Hand Mouse Controller - Main Application
Controla o mouse usando detecção de mãos via MediaPipe
"""
import cv2
import mediapipe as mp
from math import sqrt

import config
from hand_detector import HandDetector
from gesture_processor import GestureProcessor
from cursor_controller import CursorController
from socket_client_comm import SocketClient
from video_display import VideoDisplay


class HandMouseController:
    """Controlador principal do mouse por gestos de mão"""
    
    def __init__(self):
        """Inicializa o controlador com configurações"""
        # Carregar configurações
        self.config = config.load_config()
        
        # Inicializar componentes
        self.socket_client = SocketClient()
        self.hand_detector = HandDetector()
        self.video_display = VideoDisplay()
        
        # Componentes que precisam do socket
        self.cursor = CursorController(self.config, self.socket_client.send_command)
        self.gesture = GestureProcessor(self.config, self.socket_client.send_command)
        
    def process_hand_data(self, hand, frame_width, frame_height):
        """
        Processa dados da mão detectada
        
        Args:
            hand: Landmarks da mão
            frame_width: Largura do frame
            frame_height: Altura do frame
            
        Returns:
            Dicionário com dados processados
        """
        landmarks = self.hand_detector.get_landmarks(hand)
        
        thumb = landmarks['thumb']
        index = landmarks['index']
        wrist = landmarks['wrist']
        middle = landmarks['middle']
        
        # Converter para pixels
        thumb_px = (int(thumb.x * frame_width), int(thumb.y * frame_height))
        index_px = (int(index.x * frame_width), int(index.y * frame_height))
        
        # Calcular tamanho da mão
        hand_size = sqrt((wrist.x - middle.x)**2 + (wrist.y - middle.y)**2)
        
        return {
            'thumb': thumb,
            'index': index,
            'thumb_px': thumb_px,
            'index_px': index_px,
            'hand_size': hand_size
        }
        
    def run(self):
        """Loop principal de execução"""
        print("=" * 50)
        print("CONTROLE DE MÃO ATIVO")
        print("Pressione ESC para sair")
        print("=" * 50)
        
        try:
            # Conectar e configurar componentes
            if not self.socket_client.connect():
                return
                
            self.hand_detector.setup()
            self.video_display.setup_camera()
            
            frame_count = 0
            
            # Loop principal
            while True:
                # Ler frame
                ret, frame = self.video_display.read_frame()
                if not ret:
                    continue
                
                h, w, _ = frame.shape
                
                # Converter para MediaPipe
                rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb)
                timestamp_ms = frame_count * 33
                
                # Detectar mão
                results = self.hand_detector.detect(mp_image, timestamp_ms)
                
                if results.hand_landmarks:
                    hand = results.hand_landmarks[0]
                    hand_data = self.process_hand_data(hand, w, h)
                    
                    # Mover cursor se mão for grande o suficiente
                    if hand_data['hand_size'] > 0.05:
                        self.cursor.update_position(hand_data['index'])
                    
                    # Processar gestos de pinça
                    is_pinch = self.gesture.is_pinching(
                        hand_data['thumb'], 
                        hand_data['index']
                    )
                    self.gesture.process_pinch(is_pinch)
                    
                    # Desenhar debug
                    self.video_display.draw_hand_landmarks(
                        frame, 
                        hand_data['thumb_px'], 
                        hand_data['index_px']
                    )
                    self.video_display.draw_status(frame, self.gesture.get_status())
                
                # Mostrar frame
                self.video_display.show_frame(frame)
                frame_count += 1
                
                # Verificar saída
                if self.video_display.should_quit():
                    break
                    
        except KeyboardInterrupt:
            print("\n⚠ Interrompido pelo usuário")
        except Exception as e:
            print(f"✗ Erro: {e}")
            import traceback
            traceback.print_exc()
        finally:
            self.cleanup()
            
    def cleanup(self):
        """Limpa recursos"""
        self.video_display.cleanup()
        self.socket_client.close()
        print("✓ Recursos liberados")


def main():
    """Função principal"""
    controller = HandMouseController()
    controller.run()


if __name__ == "__main__":
    main()

