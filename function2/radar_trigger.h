#pragma once

#include "ld2450.h"

#include <array>
#include <chrono>
#include <optional>
#include <set>
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

    std::optional<int> lastBoundary;
    std::optional<int> triggered;

    std::vector<int> locked;
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
        std::optional<double> previousDistance;
        std::optional<int> lastBoundary;

        std::chrono::steady_clock::time_point lastSeen {};
        std::set<int> lockedBoundaries;

        void clear();
    };

    std::array<SlotState, 3> slots_;
};