FROM python:3.11-slim

ENV DEBIAN_FRONTEND=noninteractive

ENV PROJECT_DIR=/app
ENV BIN_NAME=hand_mouse

RUN apt-get update && apt-get install -y wget curl x11-utils \
    build-essential 

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    apt-utils \
    ca-certificates \
    libglib2.0-0 \
    ffmpeg \
    xvfb && \
    rm -rf /var/lib/apt/lists/*

WORKDIR $PROJECT_DIR

COPY . $PROJECT_DIR

RUN pip install --no-cache-dir --upgrade pip
RUN pip install --no-cache-dir opencv-python mediapipe pyautogui pyinstaller

RUN apt-get update && apt-get install -y python3-tk python3-dev

RUN python3 - <<EOF
import os
import urllib.request

model_path = 'hand_landmarker.task'
if not os.path.exists(model_path):
    print("Baixando modelo...")
    url = 'https://storage.googleapis.com/mediapipe-models/hand_landmarker/hand_landmarker/float16/1/hand_landmarker.task'
    urllib.request.urlretrieve(url, model_path)
    print("Modelo baixado!")
EOF


RUN pyinstaller --onefile --name $BIN_NAME hand_mouse.py

RUN rm -rf build __pycache__ *.spec

ENTRYPOINT ["./dist/hand_mouse"]
CMD ["--background"]
