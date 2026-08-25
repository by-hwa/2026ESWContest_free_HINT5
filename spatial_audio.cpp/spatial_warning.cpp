#include <AL/al.h>
#include <AL/alc.h>

#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>

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

int main(int argc, char* argv[])
{

    if (argc != 6) {
        std::cerr << "Usage: ./spatial_warning x y z sound.wav repeat\n";
        return 1;
    }

    float x = std::stof(argv[1]);
    float y = std::stof(argv[2]);
    float z = std::stof(argv[3]);

    const char* soundFile = argv[4];
    int repeat = std::stoi(argv[5]);

    if (argc != 6) {
        std::cerr << "Usage: ./spatial_warning x y z sound.wav repeat\n";
        return 1;
    }

    std::cout << "Sound position: "
              << x << ", "
              << y << ", "
              << z << '\n';

    // -------------------------
    // OpenAL Device
    // -------------------------

    ALCdevice* device = alcOpenDevice(nullptr);

    if (!device) {
        std::cerr << "Failed to open OpenAL device\n";
        return 1;
    }

    ALCcontext* context = alcCreateContext(device, nullptr);

    if (!context) {
        std::cerr << "Failed to create OpenAL context\n";
        alcCloseDevice(device);
        return 1;
    }

    alcMakeContextCurrent(context);

    // -------------------------
    // Listener
    // -------------------------

    // Rider 위치
    alListener3f(
        AL_POSITION,
        0.0f,
        0.0f,
        0.0f
    );

    // OpenAL 기본:
    // 앞 = -Z
    // 위 = +Y

    float orientation[] = {
        0.0f, 0.0f, -1.0f,   // 바라보는 방향
        0.0f, 1.0f,  0.0f    // 위쪽
    };

    alListenerfv(
        AL_ORIENTATION,
        orientation
    );

    // -------------------------
    // WAV
    // -------------------------

    WavData wav;

    if (!loadWav(soundFile, wav)) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context);
        alcCloseDevice(device);
        return 1;
    }

    // -------------------------
    // Buffer
    // -------------------------

    ALuint buffer;

    alGenBuffers(1, &buffer);

    alBufferData(
        buffer,
        wav.format,
        wav.data.data(),
        static_cast<ALsizei>(wav.data.size()),
        wav.sampleRate
    );

    // -------------------------
    // 3D Source
    // -------------------------

    ALuint source;

    alGenSources(1, &source);

    alSourcei(
        source,
        AL_BUFFER,
        buffer
    );

    alSource3f(
        source,
        AL_POSITION,
        x, y, z
    );

    // 거리 감쇠
    alSourcef(
        source,
        AL_REFERENCE_DISTANCE,
        1.0f
    );

    alSourcef(
        source,
        AL_MAX_DISTANCE,
        30.0f
    );

    alSourcef(
        source,
        AL_ROLLOFF_FACTOR,
        1.0f
    );

    // -------------------------
    // Play
    // -------------------------

    alSourcePlay(source);


    for (int i = 0; i < repeat; ++i) {

        alSourcePlay(source);

        ALint state;

        do {
            alGetSourcei(source, AL_SOURCE_STATE, &state);
        }
        while (state == AL_PLAYING);
    }

    // -------------------------
    // Cleanup
    // -------------------------

    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

    return 0;
}
