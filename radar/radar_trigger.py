"""
HLK-LD2450 레이더 거리 트리거 모듈

사용법:
    from radar_trigger import RadarTrigger

    radar = RadarTrigger()

    for events in radar.run():
        for e in events:
            x = e["x"]      # 좌우 (m)
            y = e["y"]      # 전방 거리 (m)

동작:
    타겟이 접근하며 지정 거리 경계를 통과할 때만 이벤트 발생
    경계: 6.0 5.0 4.0 3.0 2.0 1.75 1.5 1.25 1.0 m
    1m 경계까지 알림하며, 이후 추가 알림은 없음

단독 실행:
    python radar_trigger.py         # Trigger 확인
    python radar_trigger.py raw     # 센서 원본값 확인
"""

import math
import time

from ld2450 import read_targets


# ---- Trigger 설정 ----

BOUNDARIES = [
    6000,
    5000,
    4000,
    3000,
    2000,
    1750,
    1500,
    1250,
    1000,
]

HYSTERESIS = 150       # [mm]
MAX_RANGE = 6000       # [mm]
CLEAR_SEC = 1.5        # [s]


class SlotState:
    def __init__(self):
        self.previous_distance = None
        self.last_boundary = None
        self.last_seen = 0.0
        self.locked_boundaries = set()

    def clear(self):
        self.previous_distance = None
        self.last_boundary = None
        self.last_seen = 0.0
        self.locked_boundaries.clear()


class RadarTrigger:
    def __init__(self):
        self.slots = [SlotState() for _ in range(3)]

    def update(self, targets):
        """한 프레임 처리 → (events, debug)"""

        now = time.monotonic()

        events = []
        debug = []

        for i, target in enumerate(targets):
            state = self.slots[i]

            # Target 미검출
            if target is None:
                if (
                    state.previous_distance is not None
                    and now - state.last_seen >= CLEAR_SEC
                ):
                    state.clear()

                debug.append(None)
                continue

            x = target["x"]
            y = target["y"]

            distance = math.hypot(x, y)
            previous = state.previous_distance

            state.last_seen = now

            # 충분히 멀어진 경계는 Unlock
            state.locked_boundaries = {
                boundary
                for boundary in state.locked_boundaries
                if distance < boundary + HYSTERESIS
            }

            triggered = None

            # 6m 초과는 판정하지 않음
            # 1m 미만은 통과할 경계가 없어 자동으로 제외됨
            in_range = distance <= MAX_RANGE

            if in_range and previous is not None:

                # 접근 중일 때만 검사
                if distance < previous:

                    crossed = [
                        boundary
                        for boundary in BOUNDARIES
                        if previous > boundary >= distance
                        and boundary not in state.locked_boundaries
                    ]

                    if crossed:
                        # 한 프레임에 여러 경계를 통과한 경우
                        # 가장 안쪽 경계로 알림, 통과한 경계는 모두 Lock
                        triggered = min(crossed)

                        state.locked_boundaries.update(crossed)
                        state.last_boundary = triggered

                        events.append({
                            "x": x / 1000,
                            "y": y / 1000,
                        })

            # 범위 밖에서도 이전 거리는 계속 갱신
            state.previous_distance = distance

            debug.append({
                "slot": i + 1,
                "x": x,
                "y": y,
                "speed": target["speed"],
                "distance": distance,
                "boundary": state.last_boundary,
                "triggered": triggered,
                "locked": sorted(state.locked_boundaries),
            })

        return events, debug

    def run(self):
        """통합 모듈용: Trigger Event 리스트를 계속 반환"""

        for targets in read_targets():
            events, _ = self.update(targets)
            yield events


# ---------------- 검증용 ----------------

def monitor_raw():
    print("Raw Monitor (Ctrl+C to stop)\n")

    frame = 0

    for targets in read_targets():
        frame += 1

        if frame % 5:
            continue

        print("-" * 76)

        for i, target in enumerate(targets):

            if target is None:
                print(f"T{i + 1} | no target")
                continue

            distance = math.hypot(target["x"], target["y"])
            angle = math.degrees(
                math.atan2(target["x"], target["y"])
            )

            print(
                f"T{i + 1} | "
                f"x {target['x']:6d} | "
                f"y {target['y']:6d} | "
                f"v {target['speed']:5d} | "
                f"dist {distance:6.0f} | "
                f"angle {angle:+6.1f}"
            )


def monitor_trigger():
    print("Trigger Monitor (Ctrl+C to stop)\n")

    radar = RadarTrigger()
    frame = 0

    for targets in read_targets():
        frame += 1

        events, debug = radar.update(targets)

        # Event 발생 순간
        for data in debug:

            if data and data["triggered"]:
                angle = math.degrees(
                    math.atan2(data["x"], data["y"])
                )

                print(
                    f">>> EVENT "
                    f"T{data['slot']} | "
                    f"Boundary {data['triggered'] / 1000:.2f} m | "
                    f"X {data['x'] / 1000:+.2f} m | "
                    f"Y {data['y'] / 1000:.2f} m | "
                    f"Angle {angle:+.1f}"
                )

        # 상태 확인
        if frame % 10:
            continue

        print("-" * 76)

        for i, data in enumerate(debug):

            if data is None:
                print(f"T{i + 1} | no target")
                continue

            last = data["boundary"] or "-"
            locked = ",".join(
                str(b) for b in data["locked"]
            ) or "-"

            print(
                f"T{i + 1} | "
                f"x {data['x']:6d} | "
                f"y {data['y']:6d} | "
                f"dist {data['distance']:6.0f} | "
                f"v {data['speed']:5d} | "
                f"last {last:>5} | "
                f"locked {locked}"
            )


if __name__ == "__main__":
    import sys

    try:
        if len(sys.argv) > 1 and sys.argv[1] == "raw":
            monitor_raw()
        else:
            monitor_trigger()

    except KeyboardInterrupt:
        print("\nStopped")