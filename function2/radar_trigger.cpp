#include "radar_trigger.h"

#include <cmath>

namespace {

struct Zone {
    int maxDistance;
    double interval;
};

// 가까운 순서
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

// Anchor 갱신을 위한 최소 위치 변화 [mm]
constexpr double MOVE_THRESHOLD = 200.0;

// 접근/이탈 방향을 변경하기 위한 최소 거리 변화 [mm]
constexpr double DIRECTION_THRESHOLD = 100.0;

// Anchor가 유지되는 시간이 길면 정지로 판단 [s]
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
        // Anchor 설정 및 접근/이탈 판정
        // ------------------------------------------------
        if (!state.anchorX.has_value()) {

            // 첫 감지에서는 기준점만 설정
            // 실제 이동이 확인되기 전에는 접근으로 판단하지 않음
            state.anchorX = x;
            state.anchorY = y;
            state.anchorDistance = distance;
            state.anchorTime = now;

            state.approaching = false;
        }
        else {

            // 기존 Anchor에서 얼마나 이동했는지 계산
            const double moved = std::hypot(
                static_cast<double>(x - *state.anchorX),
                static_cast<double>(y - *state.anchorY)
            );


            // 200 mm 이상 이동했을 때만 방향 판정
            if (moved > MOVE_THRESHOLD) {

                // 양수  : 멀어짐
                // 음수  : 가까워짐
                const double distanceChange =
                    distance - *state.anchorDistance;


                // 100 mm 이상 가까워졌으면 접근
                if (distanceChange <= -DIRECTION_THRESHOLD) {
                    state.approaching = true;
                }

                // 100 mm 이상 멀어졌으면 이탈
                else if (distanceChange >= DIRECTION_THRESHOLD) {
                    state.approaching = false;
                }

                // ±100 mm 이내 변화는
                // 기존 접근/이탈 상태 유지


                // 현재 위치를 새로운 Anchor로 갱신
                state.anchorX = x;
                state.anchorY = y;
                state.anchorDistance = distance;
                state.anchorTime = now;
            }
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


        // 접근 중이며 정지 상태가 아닐 때만 경고
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

                // 거리 구간별 알림 주기가 지나면 반복 경고
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

            // 이탈 또는 정지 상태에서는 경고 비활성화
            // 이후 다시 접근하면 즉시 첫 경고 발생
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