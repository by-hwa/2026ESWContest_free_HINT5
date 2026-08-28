#pragma once

#include "camera/camera.hpp"
#include "vlm/vlm.hpp"
#include "stt/stt.hpp"
#include "tts/tts.hpp"

class SceneAssistant
{
public:
    SceneAssistant();

    // 버튼 Short Press
    void describeScene();

    // 버튼 Long Press
    void answerQuestion();

private:
    Camera camera_;
    VLM vlm_;
    STT stt_;
    TTS tts_;
};