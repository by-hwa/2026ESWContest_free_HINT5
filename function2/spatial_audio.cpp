#include "spatial_audio.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

namespace {

struct WavData {
    ALenum format;
    ALsizei sampleRate;
    std::vector<char> data;
};

bool loadWav(const char* filename, WavData& wav)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "Cannot open WAV file\n";
        return false;
    }

    char riff[4];
    uint32_t fileSize;
    char wave[4];

    file.read(riff, 4);
    file.read(reinterpret_cast<char*>(&fileSize), 4);
    file.read(wave, 4);

    if (std::strncmp(riff, "RIFF", 4) != 0 ||
        std::strncmp(wave, "WAVE", 4) != 0) {
        std::cerr << "Not a WAV file\n";
        return false;
    }

    uint16_t channels = 0;
    uint32_t sampleRate = 0;
    uint16_t bitsPerSample = 0;

    while (file) {
        char chunkId[4];
        uint32_t chunkSize;

        file.read(chunkId, 4);
        file.read(reinterpret_cast<char*>(&chunkSize), 4);

        if (!file)
            break;

        if (std::strncmp(chunkId, "fmt ", 4) == 0) {
            uint16_t audioFormat;

            file.read(reinterpret_cast<char*>(&audioFormat), 2);
            file.read(reinterpret_cast<char*>(&channels), 2);
            file.read(reinterpret_cast<char*>(&sampleRate), 4);

            file.seekg(6, std::ios::cur);

            file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

            file.seekg(chunkSize - 16, std::ios::cur);

            if (audioFormat != 1) {
                std::cerr << "Only PCM WAV is supported\n";
                return false;
            }
        }
        else if (std::strncmp(chunkId, "data", 4) == 0) {
            wav.data.resize(chunkSize);
            file.read(wav.data.data(), chunkSize);
            break;
        }
        else {
            file.seekg(chunkSize, std::ios::cur);
        }
    }

    if (channels == 1 && bitsPerSample == 16) {
        wav.format = AL_FORMAT_MONO16;
    }
    else if (channels == 2 && bitsPerSample == 16) {
        wav.format = AL_FORMAT_STEREO16;
    }
    else {
        std::cerr << "Only 16-bit mono/stereo WAV is supported\n";
        return false;
    }

    wav.sampleRate = sampleRate;

    return true;
}

}  // namespace


bool SpatialAudio::init(const char* soundFile)
{
    device_ = alcOpenDevice(nullptr);

    if (!device_) {
        std::cerr << "Failed to open OpenAL device\n";
        return false;
    }

    context_ = alcCreateContext(device_, nullptr);

    if (!context_) {
        std::cerr << "Failed to create OpenAL context\n";
        alcCloseDevice(device_);
        device_ = nullptr;
        return false;
    }

    alcMakeContextCurrent(context_);

    // Listener
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);

    // OpenAL 기본: 앞 = -Z, 위 = +Y
    float orientation[] = {
        0.0f, 0.0f, -1.0f,
        0.0f, 1.0f,  0.0f
    };
    alListenerfv(AL_ORIENTATION, orientation);

    // WAV
    WavData wav;

    if (!loadWav(soundFile, wav)) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context_);
        alcCloseDevice(device_);
        context_ = nullptr;
        device_ = nullptr;
        return false;
    }

    alGenBuffers(1, &buffer_);
    alBufferData(
        buffer_,
        wav.format,
        wav.data.data(),
        static_cast<ALsizei>(wav.data.size()),
        wav.sampleRate
    );

    // 동시 재생을 위해 Source 여러 개
    alGenSources(SOURCE_COUNT, sources_);

    for (int i = 0; i < SOURCE_COUNT; ++i) {
        alSourcei(sources_[i], AL_BUFFER, buffer_);
        alSourcef(sources_[i], AL_REFERENCE_DISTANCE, 1.0f);
        alSourcef(sources_[i], AL_MAX_DISTANCE, 30.0f);
        alSourcef(sources_[i], AL_ROLLOFF_FACTOR, 1.0f);
    }

    ready_ = true;
    return true;
}


void SpatialAudio::playRear(float x, float y)
{
    // 레이더는 후방을 향하므로
    // 레이더 전방 거리(y) → OpenAL 후방(+Z)
    play(x, 0.0f, y);
}


void SpatialAudio::play(float x, float y, float z)
{
    if (!ready_) {
        return;
    }

    // 재생 중이 아닌 Source를 찾아 사용
    for (int i = 0; i < SOURCE_COUNT; ++i) {
        ALint state;
        alGetSourcei(sources_[i], AL_SOURCE_STATE, &state);

        if (state != AL_PLAYING) {
            alSource3f(sources_[i], AL_POSITION, x, y, z);
            alSourcePlay(sources_[i]);
            return;
        }
    }

    // 모든 Source가 재생 중이면 이번 Event는 생략
}


SpatialAudio::~SpatialAudio()
{
    if (ready_) {
        alDeleteSources(SOURCE_COUNT, sources_);
        alDeleteBuffers(1, &buffer_);
    }

    if (context_) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context_);
    }

    if (device_) {
        alcCloseDevice(device_);
    }
}