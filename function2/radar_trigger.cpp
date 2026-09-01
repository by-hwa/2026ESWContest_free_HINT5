#include "radar_trigger.h"

#include <cmath>

namespace {

struct Zone {
    int maxDistance;
    double interval;
};

// 거리 구간별 알림 주기
constexpr Zone ZONES[] = {
    {1000, 0.1},     // 0.3 ~ 1.0 m
    {1200, 0.15},    // 1.0 ~ 1.2 m
    {1500, 0.3},     // 1.2 ~ 1.5 m
    {2000, 0.6},     // 1.5 ~ 2.0 m
    {3000, 1.2},     // 2.0 ~ 3.0 m
    {6000, 2.5},     // 3.0 ~ 6.0 m
};

constexpr int MIN_RANGE = 300;
constexpr int MAX_RANGE = 6000;

// 작은 위치 변화는 무시 [mm]
constexpr double MOVE_THRESHOLD = 200.0;

// 접근 / 비접근 상태를 변경하기 위한 거리 변화 [mm]
constexpr double DIRECTION_THRESHOLD = 200.0;

// 이보다 큰 순간 위치 변화는
// 동일 Target의 연속 움직임으로 판단하지 않음 [mm]
constexpr double CONTINUITY_THRESHOLD = 1000.0;

// Anchor가 일정 시간 유지되면 정지로 판단 [s]
constexpr double STATIONARY_SEC = 3.0;

// Target 미검출 후 상태 초기화 시간 [s]
constexpr double CLEAR_SEC = 1.5;


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
    anchorDistance.reset();

    approaching = false;
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


        // ------------------------------------------------
        // Target 미검출
        // ------------------------------------------------
        if (!target.valid) {

            if (state.anchorX.has_value()) {

                const double elapsed =
                    std::chrono::duration<double>(
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


        // ------------------------------------------------
        // 알림 범위 밖
        // ------------------------------------------------
        if (interval == 0.0) {

            state.clear();

            debug.approaching = false;
            debug.stationary = false;
            debug.triggered = false;

            result.debug[i] = debug;
            continue;
        }


        // ------------------------------------------------
        // Anchor 설정 / 연속성 확인 / 접근 판정
        // ------------------------------------------------
        if (!state.anchorX.has_value()) {

            // 첫 감지에서는 기준점만 설정
            state.anchorX = x;
            state.anchorY = y;
            state.anchorDistance = distance;
            state.anchorTime = now;

            // 실제 움직임이 확인되기 전까지 경고하지 않음
            state.approaching = false;
            state.active = false;
        }
        else {

            // 기존 Anchor와 현재 위치의 XY 이동량
            const double moved = std::hypot(
                static_cast<double>(x - *state.anchorX),
                static_cast<double>(y - *state.anchorY)
            );


            // ------------------------------------------------
            // 1. 너무 큰 순간 이동
            // ------------------------------------------------
            if (moved >= CONTINUITY_THRESHOLD) {

                // 센서값 급변 또는 Target slot 변경 가능성
                // 동일 Target의 연속 움직임으로 사용하지 않고
                // 현재 값을 새로운 기준점으로 설정
                state.anchorX = x;
                state.anchorY = y;
                state.anchorDistance = distance;
                state.anchorTime = now;

                state.approaching = false;
                state.active = false;
            }

            // ------------------------------------------------
            // 2. 정상적인 이동 범위
            // ------------------------------------------------
            else if (moved > MOVE_THRESHOLD) {

                // 양수 : 멀어짐
                // 음수 : 가까워짐
                const double distanceChange =
                    distance - *state.anchorDistance;


                // 100 mm 이상 가까워짐
                if (distanceChange <= -DIRECTION_THRESHOLD) {
                    state.approaching = true;
                }

                // 100 mm 이상 멀어짐
                else if (distanceChange >= DIRECTION_THRESHOLD) {
                    state.approaching = false;
                }

                // ±100 mm 이내이면
                // 기존 approaching 상태 유지


                // 현재 위치를 다음 판정을 위한 Anchor로 갱신
                state.anchorX = x;
                state.anchorY = y;
                state.anchorDistance = distance;
                state.anchorTime = now;
            }

            // MOVE_THRESHOLD 이하의 작은 변화는 무시
        }


        // ------------------------------------------------
        // 정지 판정
        // ------------------------------------------------
        const double heldFor =
            std::chrono::duration<double>(
                now - state.anchorTime
            ).count();

        const bool stationary =
            heldFor >= STATIONARY_SEC;


        // ------------------------------------------------
        // 경고 판단
        // ------------------------------------------------
        bool triggered = false;


        if (state.approaching && !stationary) {

            // 새롭게 접근 상태가 된 경우 즉시 경고
            if (!state.active) {
                triggered = true;
            }
            else {

                const double elapsed =
                    std::chrono::duration<double>(
                        now - state.lastPlayed
                    ).count();

                // 거리 구간별 반복 주기
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
        else {

            // 비접근 또는 정지 상태
            state.active = false;
        }


        // ------------------------------------------------
        // Debug
        // ------------------------------------------------
        debug.approaching = state.approaching;
        debug.stationary = stationary;
        debug.triggered = triggered;

        result.debug[i] = debug;
    }


    return result;
}
