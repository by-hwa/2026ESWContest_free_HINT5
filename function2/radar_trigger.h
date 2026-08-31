#pragma once

#include "ld2450.h"

#include <array>
#include <chrono>
#include <optional>
#include <vector>

struct RadarEvent {
    float x = 0.0f;   // 좌우 [m]
    float y = 0.0f;   // 전방 거리 [m]
};

struct RadarDebug {
    int slot = 0;
    int x = 0;
    int y = 0;
    int speed = 0;
    double distance = 0.0;

    double interval = 0.0;      // 현재 구간의 알림 주기 [s]
    bool stationary = false;    // 정지 판정
    bool triggered = false;
};

struct RadarUpdateResult {
    std::vector<RadarEvent> events;
    std::array<std::optional<RadarDebug>, 3> debug;
};

class RadarTrigger {
public:
    RadarUpdateResult update(
        const std::array<RadarTarget, 3>& targets
    );

private:
    struct SlotState {
        std::optional<int> anchorX;     // 정지 판정 기준 위치
        std::optional<int> anchorY;

        std::chrono::steady_clock::time_point anchorTime {};
        std::chrono::steady_clock::time_point lastPlayed {};
        std::chrono::steady_clock::time_point lastSeen {};

        bool active = false;

        void clear();
    };

    std::array<SlotState, 3> slots_;
};
