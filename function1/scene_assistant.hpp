#pragma once

#include "camera/camera.hpp"
#include "vlm/vlm.hpp"
#include "vlm/vlm_model.hpp"
#include "stt/stt.hpp"
#include "tts/tts.hpp"

class SceneAssistant
{
public:
    SceneAssistant();

    bool initialize();

    // 버튼 Short Press
    void describeScene();

    // 버튼 Long Press
    void answerQuestion();

private:
    Camera camera_;
    VLMModel vlm_model_;
    VLM vlm_;
    STT stt_;
    TTS tts_;
};
