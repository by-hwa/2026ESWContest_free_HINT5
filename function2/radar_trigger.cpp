#include "radar_trigger.h"

#include <algorithm>
#include <cmath>

namespace {

// 알림 거리 경계 [mm]
constexpr std::array<int, 9> BOUNDARIES = {
    6000,
    5000,
    4000,
    3000,
    2000,
    1750,
    1500,
    1250,
    1000,
};

constexpr int HYSTERESIS = 150;    // [mm]
constexpr int MAX_RANGE = 6000;    // [mm]
constexpr double CLEAR_SEC = 1.5;  // [s]

}  // namespace


void RadarTrigger::SlotState::clear()
{
    previousDistance.reset();
    lastBoundary.reset();
    lockedBoundaries.clear();
}


RadarUpdateResult RadarTrigger::update(
    const std::array<RadarTarget, 3>& targets
)
{
    const auto now = std::chrono::steady_clock::now();

    RadarUpdateResult result;

    for (std::size_t i = 0; i < targets.size(); ++i) {

        const auto& target = targets[i];
        auto& state = slots_[i];

        // Target 미검출
        if (!target.valid) {

            if (state.previousDistance.has_value()) {

                double elapsed = std::chrono::duration<double>(
                    now - state.lastSeen
                ).count();

                if (elapsed >= CLEAR_SEC) {
                    state.clear();
                }
            }

            result.debug[i] = std::nullopt;
            continue;
        }

        const int x = target.x;
        const int y = target.y;

        const double distance = std::hypot(
            static_cast<double>(x),
            static_cast<double>(y)
        );

        const auto previous = state.previousDistance;

        state.lastSeen = now;

        // 충분히 멀어진 경계 Unlock
        for (auto it = state.lockedBoundaries.begin();
             it != state.lockedBoundaries.end(); ) {

            if (distance >= *it + HYSTERESIS) {
                it = state.lockedBoundaries.erase(it);
            }
            else {
                ++it;
            }
        }

        std::optional<int> triggered;

        // 6m 이하만 Trigger 판정
        // 1m 미만은 통과할 경계가 없어 자동으로 제외됨
        const bool inRange = distance <= MAX_RANGE;

        if (inRange && previous.has_value()) {

            // 접근 중
            if (distance < *previous) {

                std::vector<int> crossed;

                for (int boundary : BOUNDARIES) {
                    if (*previous > boundary
                        && boundary >= distance
                        && state.lockedBoundaries.count(boundary) == 0) {
                        crossed.push_back(boundary);
                    }
                }

                if (!crossed.empty()) {

                    // 여러 경계를 한 번에 통과하면
                    // 가장 안쪽 경계 하나만 Event
                    triggered = *std::min_element(
                        crossed.begin(),
                        crossed.end()
                    );

                    // 통과한 경계 Lock
                    for (int boundary : crossed) {
                        state.lockedBoundaries.insert(boundary);
                    }

                    state.lastBoundary = triggered;

                    result.events.push_back({
                        x / 1000.0f,
                        y / 1000.0f,
                    });
                }
            }
        }

        // 범위 밖에서도 이전 거리 갱신
        state.previousDistance = distance;

        RadarDebug debug;

        debug.slot = static_cast<int>(i + 1);
        debug.x = x;
        debug.y = y;
        debug.speed = target.speed;
        debug.distance = distance;
        debug.lastBoundary = state.lastBoundary;
        debug.triggered = triggered;

        debug.locked.assign(
            state.lockedBoundaries.begin(),
            state.lockedBoundaries.end()
        );

        result.debug[i] = debug;
    }

    return result;
}