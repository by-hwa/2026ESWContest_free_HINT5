#include "spatial_audio.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace {

struct WavData {
    ALenum format = 0;
    ALsizei sampleRate = 0;
    std::vector<char> data;
};

WavData loadWav(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        throw std::runtime_error("Cannot open WAV file: " + filename);
    }

    char riff[4];
    std::uint32_t fileSize;
    char wave[4];

    file.read(riff, 4);
    file.read(reinterpret_cast<char*>(&fileSize), 4);
    file.read(wave, 4);

    if (std::strncmp(riff, "RIFF", 4) != 0 ||
        std::strncmp(wave, "WAVE", 4) != 0) {
        throw std::runtime_error("Not a WAV file: " + filename);
    }

    std::uint16_t channels = 0;
    std::uint32_t sampleRate = 0;
    std::uint16_t bitsPerSample = 0;

    WavData wav;

    while (file) {
        char chunkId[4];
        std::uint32_t chunkSize;

        file.read(chunkId, 4);
        file.read(reinterpret_cast<char*>(&chunkSize), 4);

        if (!file) {
            break;
        }

        if (std::strncmp(chunkId, "fmt ", 4) == 0) {
            std::uint16_t audioFormat;

            file.read(reinterpret_cast<char*>(&audioFormat), 2);
            file.read(reinterpret_cast<char*>(&channels), 2);
            file.read(reinterpret_cast<char*>(&sampleRate), 4);

            file.seekg(6, std::ios::cur);

            file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

            file.seekg(chunkSize - 16, std::ios::cur);

            if (audioFormat != 1) {
                throw std::runtime_error("Only PCM WAV is supported");
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
        throw std::runtime_error("Only 16-bit mono/stereo WAV is supported");
    }

    wav.sampleRate = static_cast<ALsizei>(sampleRate);

    return wav;
}

}  // namespace


SpatialAudio::SpatialAudio(const std::string& soundFile)
{
    device_ = alcOpenDevice(nullptr);

    if (!device_) {
        throw std::runtime_error("Failed to open OpenAL device");
    }

    context_ = alcCreateContext(device_, nullptr);

    if (!context_) {
        cleanup();
        throw std::runtime_error("Failed to create OpenAL context");
    }

    alcMakeContextCurrent(context_);

    // Listener
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);

    // OpenAL 기본: 앞 = -Z, 위 = +Y
    const float orientation[] = {
        0.0f, 0.0f, -1.0f,
        0.0f, 1.0f,  0.0f
    };

    alListenerfv(AL_ORIENTATION, orientation);

    try {
        const WavData wav = loadWav(soundFile);

        alGenBuffers(1, &buffer_);

        alBufferData(
            buffer_,
            wav.format,
            wav.data.data(),
            static_cast<ALsizei>(wav.data.size()),
            wav.sampleRate
        );
    }
    catch (...) {
        cleanup();
        throw;
    }

    // 동시 재생을 위해 Source 여러 개
    alGenSources(SOURCE_COUNT, sources_.data());

    for (ALuint source : sources_) {
        alSourcei(source, AL_BUFFER, static_cast<ALint>(buffer_));
        alSourcef(source, AL_REFERENCE_DISTANCE, 1.0f);
        alSourcef(source, AL_MAX_DISTANCE, 30.0f);
        alSourcef(source, AL_ROLLOFF_FACTOR, 1.0f);
    }
}


SpatialAudio::~SpatialAudio()
{
    if (sources_[0] != 0) {
        alDeleteSources(SOURCE_COUNT, sources_.data());
    }

    if (buffer_ != 0) {
        alDeleteBuffers(1, &buffer_);
    }

    cleanup();
}


void SpatialAudio::cleanup()
{
    if (context_) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context_);
        context_ = nullptr;
    }

    if (device_) {
        alcCloseDevice(device_);
        device_ = nullptr;
    }
}


void SpatialAudio::playRear(float x, float y)
{
    // 레이더가 후방을 향하므로 좌우 반전
    //   센서 기준 오른쪽 = 사용자 기준 왼쪽
    // 레이더 전방 거리(y) → OpenAL 후방(+Z)
    play(-x, 0.0f, y);
}


void SpatialAudio::play(float x, float y, float z)
{
    // 재생 중이 아닌 Source를 찾아 사용
    for (ALuint source : sources_) {
        ALint state = 0;
        alGetSourcei(source, AL_SOURCE_STATE, &state);

        if (state != AL_PLAYING) {
            alSource3f(source, AL_POSITION, x, y, z);
            alSourcePlay(source);
            return;
        }
    }

    // 모든 Source가 재생 중이면 이번 Event는 생략
}
