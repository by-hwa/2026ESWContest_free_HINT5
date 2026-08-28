#pragma once

#include <AL/al.h>
#include <AL/alc.h>

class SpatialAudio {
public:
    SpatialAudio() = default;
    ~SpatialAudio();

    bool init(const char* soundFile);
    bool isReady() const { return ready_; }

    // 후방 타겟 경고
    //   x : 좌우 (m), 오른쪽이 +
    //   y : 거리 (m)
    void playRear(float x, float y);

private:
    static constexpr int SOURCE_COUNT = 3;

    // OpenAL 좌표계로 직접 재생
    //   +X 오른쪽, +Y 위, -Z 앞
    void play(float x, float y, float z);

    ALCdevice* device_ = nullptr;
    ALCcontext* context_ = nullptr;
    ALuint buffer_ = 0;
    ALuint sources_[SOURCE_COUNT] = {0};
    bool ready_ = false;
};