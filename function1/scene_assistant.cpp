#include "scene_assistant.hpp"

#include <iostream>

SceneAssistant::SceneAssistant()
    : vlm_model_("", ""),
      vlm_(vlm_model_)
{
    std::cout << "[SceneAssistant] Initialized\n";
}

bool SceneAssistant::initialize()
{
    const bool vlm_ready = vlm_model_.initialize();
    const bool stt_ready = stt_.initialize();
    return vlm_ready && stt_ready;
}

void SceneAssistant::describeScene()
{
    std::cout << "[Function1] Scene description started\n";

    // 1. 현재 장면 촬영
    std::string image_path = camera_.capture();

    if (image_path.empty())
    {
        std::cerr << "[Function1] Camera capture failed\n";
        return;
    }

    // 2. VLM으로 장면 설명
    std::string description = vlm_.describe(image_path);

    if (description.empty())
    {
        std::cerr << "[Function1] VLM failed\n";
        return;
    }

    std::cout << "[Function1] VLM result: "
              << description << '\n';

    // 3. 음성 출력
    tts_.speak(description);

    std::cout << "[Function1] Scene description finished\n";
}

void SceneAssistant::answerQuestion()
{
    std::cout << "[Function1] Question answering started\n";

    // 1. 음성 → 텍스트
    std::string question = stt_.recordAndTranscribe();

    if (question.empty())
    {
        std::cerr << "[Function1] No question was recognized\n";
        tts_.speak("I could not understand the question. I will describe the scene.");
        describeScene();
        return;
    }

    std::cout << "[Function1] Question: "
              << question << '\n';

    // 3. 현재 장면 촬영
    std::string image_path = camera_.capture();

    if (image_path.empty())
    {
        std::cerr << "[Function1] Camera capture failed\n";
        return;
    }

    // 4. 이미지 + 질문 → VLM
    std::string answer =
        vlm_.answer(image_path, question);

    if (answer.empty())
    {
        std::cerr << "[Function1] VLM failed\n";
        return;
    }

    std::cout << "[Function1] VLM result: "
              << answer << '\n';

    // 5. 답변 음성 출력
    tts_.speak(answer);

    std::cout << "[Function1] Question answering finished\n";
}
