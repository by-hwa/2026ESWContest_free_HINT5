# 2026ESWContest_free_hint5

## Function 1 실행 준비

기능 1은 `llama-mtmd-cli`, `whisper-cli`, `arecord`, `rpicam-still`, `espeak-ng`를 사용한다.
모델과 실행 파일 경로는 실행 전에 환경변수로 지정한다.

```bash
export VLM_CLI=/path/to/llama-mtmd-cli
export VLM_MODEL_PATH=/path/to/smolvlm.gguf
export VLM_MMPROJ_PATH=/path/to/mmproj.gguf
export WHISPER_CLI=/path/to/whisper-cli
export WHISPER_MODEL_PATH=/path/to/ggml-small.bin
export AUDIO_DEVICE=default
export NOTIFICATION_SOUND_PATH=/path/to/message_sound.wav
cmake -S . -B build
cmake --build build
./build/ai_assistive
```

짧은 버튼은 영어 장면 설명, 긴 버튼은 1초 동안 누르는 즉시 알림음 후
3초 영어 녹음과 이미지 기반 질의응답을 실행한다.



## 빌드 · 실행 방식 정리

CMakeLists에 function2를 추가해서 빌드는 한 번에, 실행은 따로.
레이더는 상시 감시, VLM은 요청 시 동작이라 처리 지연이 서로 영향을 주지 않도록 독립 프로세스로 분리

### 빌드

```bash
cmake -S . -B build
cmake --build build
```

→ `build/ai_assistive`, `build/rear_warning` 생성

### 실행

**기능 1 · 장면 설명**

```bash
export VLM_CLI=/path/to/llama-mtmd-cli
export VLM_MODEL_PATH=/path/to/smolvlm.gguf
export VLM_MMPROJ_PATH=/path/to/mmproj.gguf
export WHISPER_CLI=/path/to/whisper-cli
export WHISPER_MODEL_PATH=/path/to/ggml-small.bin
export AUDIO_DEVICE=default
export NOTIFICATION_SOUND_PATH=/path/to/message_sound.wav

./build/ai_assistive
```

**기능 2 · 후방 경고**

```bash
./build/rear_warning          # 알림 발생 시에만 출력
./build/rear_warning -v       # 거리 · 주기 상태 표시
```

