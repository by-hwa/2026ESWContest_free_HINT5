# 2026ESWContest_free_hint5

# Echo Sense

> 카메라와 레이더로 감지한 주변 정보를 소리로 전달하는 온디바이스 웨어러블 보조 시스템

Echo Sense는 시각장애인·저시력자의 이동 상황 인지를 돕기 위해, 카메라 기반 장면 이해와 레이더 기반 후방 위험 감지를 결합한 프로젝트입니다. 화면을 확인하지 않아도 주변 장면을 음성으로 듣고, 후방 접근 물체의 방향과 거리를 공간음향 경보로 파악할 수 있도록 설계했습니다.

## 핵심 기능

### 1. AI 장면 설명 및 음성 질의응답

하나의 버튼으로 현재 장면을 확인하거나 장면에 대해 질문할 수 있습니다.

- 짧게 누르기: 카메라로 촬영한 현재 장면을 SmolVLM으로 분석하고, 짧은 음성 설명을 제공합니다.
- 길게 누르기(1초 이상): 3초간 음성을 녹음한 뒤 Whisper로 질문을 인식합니다. 이후 현재 장면과 질문을 함께 SmolVLM에 전달하고, 답변을 음성으로 출력합니다.

```text
Camera → SmolVLM → TTS
Voice → Whisper STT + Camera Image → SmolVLM → TTS
```

### 2. 레이더 기반 후방 위험 감지

HLK-LD2450 24 GHz mmWave 레이더로 후방의 움직이는 객체를 추적합니다.

- 객체의 X/Y 좌표와 속도 수신
- 거리와 이동 상태를 바탕으로 위험도 계산
- 감시 범위: 1~6 m
- 가까워질수록 더 빠른 주기로 경고
- 객체의 좌우 위치를 OpenAL 기반 스테레오 공간음향으로 전달

문장 안내 대신 실제 위험 방향에서 경고음이 들리도록 하여, 사용자가 방향을 빠르게 인지할 수 있도록 했습니다.

## 시스템 구성

```text
Raspberry Pi 4
├── Function 1: AI Scene Assistant
│   ├── Camera capture
│   ├── SmolVLM scene understanding
│   ├── Whisper speech-to-text
│   └── eSpeak-NG text-to-speech
│
└── Function 2: Rear Hazard Alert
    ├── LD2450 radar tracking
    ├── Hazard evaluation
    └── OpenAL spatial audio
```

Function 1의 AI 추론과 Function 2의 위험 경보를 분리해, 장면 분석 중에도 후방 위험 감지가 지속되도록 구성했습니다.

## 하드웨어

| 구분 | 구성 |
| --- | --- |
| 메인 보드 | Raspberry Pi 4 |
| 카메라 | Raspberry Pi Camera |
| 거리 센서 | HLK-LD2450 24 GHz mmWave Radar |
| 입력 장치 | Push Button, Microphone |
| 출력 장치 | Stereo Earphone / Speaker |
| 폼팩터 | 조끼형 웨어러블 프로토타입 |

## 소프트웨어

| 영역 | 사용 기술 |
| --- | --- |
| 운영체제 | Raspberry Pi OS Lite |
| 언어 및 빌드 | C++, CMake |
| 장면 이해 | SmolVLM, llama.cpp |
| 음성 인식 | Whisper, whisper.cpp |
| 음성 출력 | eSpeak-NG |
| 오디오 | ALSA, OpenAL |
| 카메라 캡처 | rpicam-still |
| GPIO 제어 | libgpiod |

## 프로젝트 구조

```text
.
├── controller/
│   ├── main.cpp
│   ├── system_controller.cpp
│   └── button.cpp
├── function1/
│   ├── scene_assistant.cpp
│   ├── camera/
│   ├── vlm/
│   ├── stt/
│   └── tts/
├── function2/
│   ├── main.cpp
│   ├── ld2450.cpp
│   ├── radar_trigger.cpp
│   ├── spatial_audio.cpp
│   └── sounds/
├── llama.cpp/
├── whisper.cpp/
└── CMakeLists.txt
```

## 실행 준비

Function 1 실행 전, 사용 중인 모델과 실행 파일 경로를 환경 변수로 지정합니다.

```bash
export VLM_CLI=/path/to/llama-mtmd-cli
export VLM_MODEL_PATH=/path/to/smolvlm.gguf
export VLM_MMPROJ_PATH=/path/to/mmproj.gguf
export WHISPER_CLI=/path/to/whisper-cli
export WHISPER_MODEL_PATH=/path/to/ggml-small.bin
export AUDIO_DEVICE=default
export NOTIFICATION_SOUND_PATH=/path/to/message_sound.wav
```

## 빌드 및 실행

```bash
cmake -S . -B build
cmake --build build
./build/ai_assistive
```

## 기술적 특징

- Multimodal AI: 이미지와 음성 질문을 함께 해석해 장면의 맥락을 전달합니다.
- Active + Passive Safety: 사용자의 요청에는 장면 설명·질의응답으로 반응하고, 위험은 자동으로 감지해 알립니다.
- Directional Audio: 문장 해석 대신 소리의 위치로 위험 방향을 직관적으로 전달합니다.
- Edge AI: 카메라, 레이더, STT, VLM을 Raspberry Pi에서 로컬로 처리해 네트워크 의존성과 영상 외부 전송을 줄였습니다.
- Wearable Interface: 조끼형 구성과 하나의 버튼으로 손을 자유롭게 유지한 채 사용할 수 있습니다.

## 안내

Echo Sense는 이동 보조를 위한 프로토타입입니다. 실제 이동 환경에서는 주변 상황을 함께 확인하고 본 시스템만을 유일한 안전 수단으로 사용하지 마세요.
