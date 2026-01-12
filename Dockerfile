FROM python:3.11-slim

ENV DEBIAN_FRONTEND=noninteractive

ENV PROJECT_DIR=/app
ENV BIN_NAME=hand_mouse

RUN apt-get update && apt-get install -y \
    build-essential \
    libgl1-mesa-glx \
    libglib2.0-0 \
    wget \
    curl \
    ffmpeg \
    xvfb \
    x11-utils \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR $PROJECT_DIR

COPY . $PROJECT_DIR

RUN pip install --no-cache-dir --upgrade pip
RUN pip install --no-cache-dir opencv-python mediapipe pyautogui pyinstaller

RUN python3 -c "\
import os, urllib.request;\
model_path = 'hand_landmarker.task';\
if not os.path.exists(model_path):\
    print('Baixando modelo...');\
    url = 'https://storage.googleapis.com/mediapipe-models/hand_landmarker/hand_landmarker/float16/1/hand_landmarker.task';\
    urllib.request.urlretrieve(url, model_path);\
    print('Modelo baixado!')"

RUN pyinstaller --onefile --name $BIN_NAME hand_mouse.py

RUN rm -rf build __pycache__ *.spec

ENTRYPOINT ["./dist/hand_mouse"]
CMD ["--background"]
