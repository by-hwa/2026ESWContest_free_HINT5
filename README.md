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
