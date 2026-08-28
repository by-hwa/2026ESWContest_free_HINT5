#include "radar_trigger.h"

#include <cmath>

namespace {

// 알림 거리 경계 [mm]
const int BOUNDARIES[] = {
    6000, 5000, 4000, 3000, 2000, 1750, 1500, 1250, 1000
};

constexpr int BOUNDARY_COUNT =
    sizeof(BOUNDARIES) / sizeof(BOUNDARIES[0]);

constexpr int HYSTERESIS = 150;         // [mm]
constexpr int MAX_RANGE = 6000;         // [mm]
constexpr double CLEAR_SEC = 1.5;       // [s]

}  // namespace


RadarTrigger::RadarTrigger(const char* port)
    : radar_(port)
{
}


std::vector<RadarEvent> RadarTrigger::poll()
{
    std::vector<RadarEvent> events;

    Target targets[3];

    if (!radar_.readFrame(targets)) {
        return events;
    }

    auto now = std::chrono::steady_clock::now();

    for (int i = 0; i < 3; ++i) {
        SlotState& state = slots_[i];
        const Target& target = targets[i];

        // Target 미검출
        if (!target.valid) {
            if (state.hasPrevious) {
                double elapsed = std::chrono::duration<double>(
                    now - state.lastSeen).count();

                if (elapsed >= CLEAR_SEC) {
                    state.clear();
                }
            }
            continue;
        }

        double distance = std::hypot(
            static_cast<double>(target.x),
            static_cast<double>(target.y));

        bool hasPrevious = state.hasPrevious;
        double previous = state.previousDistance;

        state.lastSeen = now;

        // 충분히 멀어진 경계는 Unlock
        for (auto it = state.lockedBoundaries.begin();
             it != state.lockedBoundaries.end(); ) {

            if (distance >= *it + HYSTERESIS) {
                it = state.lockedBoundaries.erase(it);
            } else {
                ++it;
            }
        }

        // 6m 초과는 판정하지 않음
        // 1m 미만은 통과할 경계가 없어 자동으로 제외됨
        if (distance <= MAX_RANGE && hasPrevious && distance < previous) {

            std::vector<int> crossed;

            for (int b = 0; b < BOUNDARY_COUNT; ++b) {
                int boundary = BOUNDARIES[b];

                if (previous > boundary && boundary >= distance &&
                    state.lockedBoundaries.count(boundary) == 0) {
                    crossed.push_back(boundary);
                }
            }

            if (!crossed.empty()) {
                // 한 Frame에 여러 경계를 통과한 경우
                // 가장 안쪽 경계로 알림, 통과한 경계는 모두 Lock
                // BOUNDARIES가 내림차순이므로 마지막이 가장 작은 값
                int triggered = crossed.back();

                for (int b : crossed) {
                    state.lockedBoundaries.insert(b);
                }

                state.lastBoundary = triggered;

                events.push_back({
                    target.x / 1000.0f,
                    target.y / 1000.0f
                });
            }
        }

        // 범위 밖에서도 이전 거리는 계속 갱신
        state.previousDistance = distance;
        state.hasPrevious = true;
    }

    return events;
}