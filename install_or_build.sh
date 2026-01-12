#!/bin/bash
set -e

PROJECT_NAME="hand_mouse"
BIN_NAME="hand_mouse"
DOCKER_SCRIPT="run_hand_mouse_docker.sh"

echo "=== Verificando ambiente ==="

if ! command -v python3 >/dev/null 2>&1; then
    echo "Python3 não encontrado! Instalando..."
    sudo apt update
    sudo apt install -y python3 python3-venv python3-pip
fi

if [ ! -d "venv" ]; then
    python3 -m venv venv
fi
source venv/bin/activate

check_or_install_pkg() {
    PACKAGE=$1
    IMPORT_NAME=$2
    if ! python3 -c "import $IMPORT_NAME" >/dev/null 2>&1; then
        echo "Biblioteca $PACKAGE não encontrada. Tentando instalar..."
        if ! pip install --user $PACKAGE; then
            echo "Falha ao instalar $PACKAGE com pip."
            echo "Escolha uma alternativa:"
            echo "1) Criar imagem Docker com projeto"
            echo "2) Tentar instalar dependências via sudo"
            read -p "Escolha [1/2]: " alt
            if [ "$alt" = "1" ]; then
                echo "Construindo imagem Docker..."
                bash docker_build.sh

                echo "#!/bin/bash" > $DOCKER_SCRIPT
                echo "xhost +local:docker" >> $DOCKER_SCRIPT
                echo "docker run --rm -e DISPLAY=\$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix hand_mouse" >> $DOCKER_SCRIPT
                chmod +x $DOCKER_SCRIPT
                echo "Script de inicialização criado: ./$DOCKER_SCRIPT"
                echo "Você pode rodar o container com GUI usando: ./$DOCKER_SCRIPT"
                exit 0
            elif [ "$alt" = "2" ]; then
                sudo apt install -y python3-$PACKAGE || echo "Não foi possível instalar via sudo. Abortando."
                exit 1
            else
                echo "Opção inválida. Abortando."
                exit 1
            fi
        fi
    fi
}

check_or_install_pkg opencv-python cv2
check_or_install_pkg mediapipe mediapipe
check_or_install_pkg pyautogui pyautogui
check_or_install_pkg pyinstaller PyInstaller

echo "=== Tudo pronto ==="

# Pergunta se só quer compilar ou instalar tudo
echo "Deseja apenas compilar o binário ou instalar tudo e compilar?"
echo "1) Apenas compilar"
echo "2) Instalar tudo e compilar"
read -p "Escolha [1/2]: " opt

if [ "$opt" = "2" ]; then
    echo "Atualizando sistema e instalando dependências básicas..."
    sudo apt update
    sudo apt install -y build-essential ffmpeg libgl1-mesa-glx libglib2.0-0 wget curl
fi

if [ ! -d "$PROJECT_NAME" ]; then
    mkdir "$PROJECT_NAME"
fi
cp -r *.py *.ini "$PROJECT_NAME"/
cd "$PROJECT_NAME"

python3 - <<EOF
import os, urllib.request
model_path = 'hand_landmarker.task'
if not os.path.exists(model_path):
    print("Baixando modelo...")
    url = 'https://storage.googleapis.com/mediapipe-models/hand_landmarker/hand_landmarker/float16/1/hand_landmarker.task'
    urllib.request.urlretrieve(url, model_path)
    print("Modelo baixado!")
EOF

pyinstaller --onefile --name "$BIN_NAME" hand_mouse.py

read -p "Deseja instalar o binário em /usr/local/bin para rodar de qualquer lugar? [y/N]: " inst
if [[ "$inst" =~ ^[Yy]$ ]]; then
    sudo cp dist/$BIN_NAME /usr/local/bin/
    sudo chmod +x /usr/local/bin/$BIN_NAME
fi

rm -rf build __pycache__ *.spec
deactivate
echo "=== Concluído! Você pode rodar: ==="
echo "./dist/$BIN_NAME --background"
