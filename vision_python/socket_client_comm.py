"""
Socket Client - Gerencia comunicação com hand_mouse_core
"""
import socket


class SocketClient:
    """Cliente de socket para comunicação com hand_mouse_core"""
    
    def __init__(self, sock_path="/tmp/hand_mouse.sock"):
        """
        Inicializa o cliente de socket
        
        Args:
            sock_path: Caminho do socket Unix
        """
        self.sock_path = sock_path
        self.sock = None
        
    def connect(self):
        """Conecta ao socket do hand_mouse_core"""
        try:
            self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            self.sock.connect(self.sock_path)
            print(f"✓ Conectado ao socket: {self.sock_path}")
            return True
        except Exception as e:
            print(f"✗ Erro ao conectar ao socket: {e}")
            return False
            
    def send_command(self, cmd):
        """
        Envia comando para o hand_mouse_core
        
        Args:
            cmd: Comando a ser enviado (ex: "MOVE 100 200", "CLICK")
        """
        try:
            if self.sock:
                self.sock.sendall((cmd + "\n").encode())
        except Exception as e:
            print(f"✗ Erro ao enviar comando '{cmd}': {e}")
            
    def close(self):
        """Fecha a conexão do socket"""
        if self.sock:
            self.sock.close()
            self.sock = None
