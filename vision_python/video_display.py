"""
Video Display - Gerencia visualização e debug do vídeo
"""
import cv2


class VideoDisplay:
    """Gerencia câmera e visualização do vídeo"""
    
    def __init__(self, width=640, height=480):
        """
        Inicializa o display de vídeo
        
        Args:
            width: Largura da captura
            height: Altura da captura
        """
        self.width = width
        self.height = height
        self.cap = None
        self.window_name = "Hand Control"
        
    def setup_camera(self):
        """Configura a câmera"""
        self.cap = cv2.VideoCapture(0)
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, self.width)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, self.height)
        self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
        print("✓ Câmera configurada")
        
    def read_frame(self):
        """
        Lê e processa um frame da câmera
        
        Returns:
            Tupla (sucesso, frame) ou (False, None)
        """
        if not self.cap:
            return False, None
            
        ret, frame = self.cap.read()
        if ret:
            frame = cv2.flip(frame, 1)
            return True, frame
        return False, None
        
    def draw_hand_landmarks(self, frame, thumb_px, index_px):
        """
        Desenha landmarks da mão no frame
        
        Args:
            frame: Frame onde desenhar
            thumb_px: Posição do polegar em pixels
            index_px: Posição do indicador em pixels
        """
        
        cv2.circle(frame, thumb_px, 10, (0, 255, 0), -1) 
        cv2.circle(frame, index_px, 10, (255, 0, 0), -1) 
        cv2.line(frame, thumb_px, index_px, (255, 255, 0), 2) 
        
    def draw_status(self, frame, status):
        """
        Desenha status atual no frame
        
        Args:
            frame: Frame onde desenhar
            status: String com status (MOVE, PINCH, DRAG)
        """
        color = (0, 255, 0)
        if status == "PINCH":
            color = (0, 255, 255)
        elif status == "DRAG":
            color = (0, 0, 255)
            
        cv2.putText(frame, status, (10, 30), 
                   cv2.FONT_HERSHEY_SIMPLEX, 1, color, 2)
        
    def show_frame(self, frame):
        """
        Mostra o frame na janela
        
        Args:
            frame: Frame a ser mostrado
        """
        cv2.imshow(self.window_name, frame)
        
    def should_quit(self):
        """
        Verifica se o usuário pressionou ESC
        
        Returns:
            True se deve sair, False caso contrário
        """
        return cv2.waitKey(1) & 0xFF == 27
        
    def cleanup(self):
        """Libera recursos de vídeo"""
        if self.cap:
            self.cap.release()
        cv2.destroyAllWindows()
