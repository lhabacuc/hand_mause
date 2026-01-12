VENV_DIR = venv
BIN_NAME = hand_mouse
DOCKER_IMAGE = lhabacuc/hand_mouse:latest
REQUIREMENTS = requirements.txt
MODEL = hand_landmarker.task

.PHONY: all install deps model build docker clean docker-run

all: build

tests:
	./$(VENV_DIR)/bin/python hand_mouse.py

install: $(VENV_DIR)/bin/activate
	@echo "Ativando ambiente virtual e instalando dependências..."
	$(VENV_DIR)/bin/pip install --upgrade pip
	$(VENV_DIR)/bin/pip install -r $(REQUIREMENTS)

$(VENV_DIR)/bin/activate:
	@echo "Criando ambiente virtual..."
	python3 -m venv $(VENV_DIR)

model:
	@echo "Verificando modelo MediaPipe..."
	python3 - <<EOF
import os, urllib.request
model_path = '$(MODEL)'
if not os.path.exists(model_path):
    print("Baixando modelo...")
    url = 'https://storage.googleapis.com/mediapipe-models/hand_landmarker/hand_landmarker/float16/1/hand_landmarker.task'
    urllib.request.urlretrieve(url, model_path)
    print("Modelo baixado!")
else:
    print("Modelo já existe.")
EOF

build: install model
	@echo "Compilando binário com PyInstaller..."
	$(VENV_DIR)/bin/pyinstaller --onefile --name $(BIN_NAME) hand_mouse.py
	@echo "Binário gerado em dist/$(BIN_NAME)"

docker:
	@echo "Construindo imagem Docker..."
	docker build -t $(DOCKER_IMAGE) .

docker-run:
	@echo "Rodando container Docker..."
	@echo "Certifique-se de ter executado: xhost +local:docker"
	xhost +local:docker
	docker run --rm -e DISPLAY=$$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix $(DOCKER_IMAGE)

clean:
	@echo "Removendo arquivos temporários..."
	rm -rf build __pycache__ *.spec dist/$(BIN_NAME)
	rm -rf $(VENV_DIR)
