#include "scene_assistant.hpp"

#include <iostream>

SceneAssistant::SceneAssistant()
{
    std::cout << "[SceneAssistant] Initialized\n";
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

    // 3. 음성 출력
    tts_.speak(description);

    std::cout << "[Function1] Scene description finished\n";
}

void SceneAssistant::answerQuestion()
{
    std::cout << "[Function1] Question answering started\n";

    // TODO: 시작 알림음으로 대체
    // 1. 녹음 시작 안내
    tts_.speak("질문을 말씀해주세요.");

    // 2. 음성 → 텍스트
    std::string question = stt_.recordAndTranscribe();

    if (question.empty())
    {
        std::cerr << "[Function1] STT failed\n";
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

    // 5. 답변 음성 출력
    tts_.speak(answer);

    std::cout << "[Function1] Question answering finished\n";
}