#pragma once

#include <array>
#include <string>

#include <AL/al.h>
#include <AL/alc.h>

class SpatialAudio {
public:
    explicit SpatialAudio(const std::string& soundFile);
    ~SpatialAudio();

    SpatialAudio(const SpatialAudio&) = delete;
    SpatialAudio& operator=(const SpatialAudio&) = delete;

    // 후방 Target 경고
    //   x : 좌우 [m], 오른쪽이 +
    //   y : 거리 [m]
    void playRear(float x, float y);

private:
    static constexpr std::size_t SOURCE_COUNT = 3;

    // OpenAL 좌표계로 직접 재생
    //   +X 오른쪽, +Y 위, -Z 앞
    void play(float x, float y, float z);

    void cleanup();

    ALCdevice* device_ = nullptr;
    ALCcontext* context_ = nullptr;
    ALuint buffer_ = 0;
    std::array<ALuint, SOURCE_COUNT> sources_ {};
};