"""
Gesture Processor - Processa gestos da mão e envia comandos
"""
import time
from math import sqrt


class GestureProcessor:
    """Processa gestos de pinça para click e drag"""
    
    def __init__(self, config, command_sender):
        """
        Inicializa o processador de gestos
        
        Args:
            config: Dicionário com configurações
            command_sender: Função para enviar comandos
        """
        self.pinch_threshold = config.get("PINCH_THRESHOLD", 0.04)
        self.click_max_duration = config.get("CLICK_MAX_DURATION", 0.3)
        self.drag_min_duration = config.get("DRAG_MIN_DURATION", 0.5)
        self.send_command = command_sender
        
        # Estado da pinça
        self.pinch_active = False
        self.pinch_start_time = 0.0
        self.dragging = False
        
    @staticmethod
    def distance(point_a, point_b):
        """Calcula distância euclidiana entre dois pontos"""
        return sqrt((point_a[0] - point_a[1])**2 + (point_b[0] - point_b[1])**2)
        
    def is_pinching(self, thumb, index):
        """Verifica se há pinça entre polegar e indicador"""
        pinch_dist = sqrt((thumb.x - index.x)**2 + (thumb.y - index.y)**2)
        return pinch_dist < self.pinch_threshold
        
    def process_pinch(self, is_pinch):
        """
        Processa estado de pinça e gera comandos de click/drag
        
        Args:
            is_pinch: Se está fazendo pinça agora
        """
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
            
    def get_status(self):
        """Retorna status atual do gesto"""
        if self.dragging:
            return "DRAG"
        elif self.pinch_active:
            return "PINCH"
        else:
            return "MOVE"
