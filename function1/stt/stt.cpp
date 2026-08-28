#include "stt.hpp"

#include <iostream>

STT::STT()
{
    std::cout << "[STT] Whisper initialized\n";
}

std::string STT::recordAndTranscribe()
{
    std::cout << "[STT] Recording...\n";

    // TODO:
    // 1. microphone recording
    // 2. whisper.cpp inference

    return "앞에 무엇이 있나요?";
}