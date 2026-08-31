#include "radar_trigger.h"

#include <cmath>

namespace {

// 거리 구간별 알림 주기
struct Zone {
    int maxDistance;    // [mm]
    double interval;    // [s]
};

// 가까운 순서
constexpr Zone ZONES[] = {
    {1200, 0.15},   // 1.0 ~ 1.2 m
    {1500, 0.3},    // 1.2 ~ 1.5 m
    {2000, 0.6},    // 1.5 ~ 2.0 m
    {3000, 1.2},    // 2.0 ~ 3.0 m
    {6000, 2.5},    // 3.0 ~ 6.0 m
};

constexpr int MIN_RANGE = 1000;     // 이보다 가까우면 알림 없음 [mm]
constexpr int MAX_RANGE = 6000;     // 이보다 멀면 알림 없음 [mm]

// 정지 판정 : MOVE_THRESHOLD 이내 이동이 STATIONARY_SEC 이상 지속
constexpr double MOVE_THRESHOLD = 200.0;    // [mm]
constexpr double STATIONARY_SEC = 3.0;      // [s]

constexpr double CLEAR_SEC = 1.5;   // 미검출 지속 시 상태 초기화 [s]


// 거리에 해당하는 알림 주기 반환. 범위 밖이면 0
double intervalFor(double distance)
{
    if (distance < MIN_RANGE || distance > MAX_RANGE) {
        return 0.0;
    }

    for (const auto& zone : ZONES) {
        if (distance <= zone.maxDistance) {
            return zone.interval;
        }
    }

    return 0.0;
}

}  // namespace


void RadarTrigger::SlotState::clear()
{
    anchorX.reset();
    anchorY.reset();
    active = false;
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

            if (state.anchorX.has_value()) {

                const double elapsed = std::chrono::duration<double>(
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

        state.lastSeen = now;

        const double interval = intervalFor(distance);

        RadarDebug debug;

        debug.slot = static_cast<int>(i + 1);
        debug.x = x;
        debug.y = y;
        debug.speed = target.speed;
        debug.distance = distance;
        debug.interval = interval;

        // 알림 범위 밖이면 상태 초기화
        // 다시 범위 안으로 들어올 때 첫 알림이 정지 판정으로 막히지 않도록
        if (interval == 0.0) {

            state.clear();

            debug.stationary = false;
            debug.triggered = false;

            result.debug[i] = debug;
            continue;
        }

        // 정지 판정 : 기준 위치에서 얼마나 이동했는지
        if (!state.anchorX.has_value()) {

            state.anchorX = x;
            state.anchorY = y;
            state.anchorTime = now;
        }
        else {

            const double moved = std::hypot(
                static_cast<double>(x - *state.anchorX),
                static_cast<double>(y - *state.anchorY)
            );

            // 충분히 움직였으면 기준 위치 갱신
            if (moved > MOVE_THRESHOLD) {
                state.anchorX = x;
                state.anchorY = y;
                state.anchorTime = now;
            }
        }

        const double heldFor = std::chrono::duration<double>(
            now - state.anchorTime
        ).count();

        const bool stationary = heldFor >= STATIONARY_SEC;

        bool triggered = false;

        if (!stationary) {

            // 새로 감지된 Target은 즉시 알림
            if (!state.active) {
                triggered = true;
            }
            else {

                const double elapsed = std::chrono::duration<double>(
                    now - state.lastPlayed
                ).count();

                if (elapsed >= interval) {
                    triggered = true;
                }
            }

            if (triggered) {
                state.lastPlayed = now;

                result.events.push_back({
                    x / 1000.0f,
                    y / 1000.0f,
                });
            }

            state.active = true;
        }

        debug.stationary = stationary;
        debug.triggered = triggered;

        result.debug[i] = debug;
    }

    return result;
}
