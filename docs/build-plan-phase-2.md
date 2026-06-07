# Build Plan: Phase 2

## Tilt Cradle Concept

Add a cradle or subframe that supports the thrower and allows controlled pitch movement. The yaw platform should remain responsible for left/right presentation.

## Linear Actuator Concept

Use a 12V linear actuator or geared mechanism to change thrower pitch. The actuator should have mechanical stops and should not overload the thrower frame.

## Pitch Limit Switches

Add upper and lower pitch limits or a position feedback sensor. The pitch axis should fault independently from yaw if limits disagree or travel timeout occurs.

## ESP32 Control Expansion

The ESP32 firmware should grow from one axis to two coordinated axes. Keep `motor.*` and `modes.*` modular so pitch control can be added without rewriting Phase 1 sweep logic.

## Battery And Power Considerations

Add current budget for the pitch actuator. Consider branch fusing for the thrower, yaw motor, pitch actuator, and 5V electronics supply.
