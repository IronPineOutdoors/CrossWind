# Power System

## Shared 12V Battery

Use one 12V battery as the main source for the thrower, Crosswind yaw motor, future actuator, and 5V electronics supply.

## Main Fuse

Place a main fuse close to the battery positive terminal. Size it for the expected total current and wiring gauge.

## Branch Fuse Block

Use a small branch fuse block so each load can be protected and serviced independently.

## Thrower Circuit

The thrower should have its own fused branch. Keep its high-current wiring physically separated from low-voltage control wiring where practical.

The thrower trigger relay does not power the thrower. It only closes a dry contact across the same two wires used by the factory foot pedal.

## Wobbler Motor Circuit

The BTS7960 motor supply branch powers the Mitsubishi Outlander rear wiper motor. Start with conservative fuse sizing based on measured current.

## Actuator Circuit

Reserve a fused branch for the future pitch actuator. Expect startup and stall current to be higher than running current.

## 5V Buck Converter Circuit

The buck converter powers the ESP32 or Arduino. Its input comes from the 12V system through a fused branch. Its output ground must tie to the motor driver logic ground.

## Trigger Relay Circuit

The ESP32 powers only the relay module input side. The relay contact side is isolated and connects to the VEVOR NH113 pedal circuit as `COM` and `NO`. Keep this circuit dry-contact only.
