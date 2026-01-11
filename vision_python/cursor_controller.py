"""
Cursor Controller - Gerencia movimento do cursor com suavização
"""


class CursorController:
    """Controla movimento do cursor com suavização"""
    
    def __init__(self, config, command_sender):
        """
        Inicializa o controlador de cursor
        
        Args:
            config: Dicionário com configurações
            command_sender: Função para enviar comandos
        """
        self.screen_width = config.get("SCREEN_WIDTH", 1920)
        self.screen_height = config.get("SCREEN_HEIGHT", 1080)
        self.smoothing = config.get("SMOOTHING", 0.5)
        self.send_command = command_sender
        
        # Estado do cursor
        self.prev_x = self.screen_width // 2
        self.prev_y = self.screen_height // 2
        
    def update_position(self, index_landmark):
        """
        Atualiza posição do cursor com suavização
        
        Args:
            index_landmark: Landmark do dedo indicador
        """
        # Converter coordenadas normalizadas para pixels
        new_x = int(index_landmark.x * self.screen_width)
        new_y = int(index_landmark.y * self.screen_height)
        
        # Aplicar suavização
        smooth_x = int(self.prev_x * self.smoothing + new_x * (1 - self.smoothing))
        smooth_y = int(self.prev_y * self.smoothing + new_y * (1 - self.smoothing))
        
        # Atualizar posição anterior
        self.prev_x, self.prev_y = smooth_x, smooth_y
        
        # Enviar comando
        self.send_command(f"MOVE {smooth_x} {smooth_y}")
        
    def get_position(self):
        """Retorna posição atual do cursor"""
        return self.prev_x, self.prev_y
