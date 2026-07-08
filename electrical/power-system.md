# Power System

## Tool Battery Input

Current Alpha power planning assumes a 20V/24V tool battery input. Do not feed that voltage directly to the ESP32, relay module, sensors, BTS7960 logic, or 12V loads.

## Main Fuse

Place a main fuse close to the tool battery positive terminal, before the high-current buck converter. Size it for the expected total current and wiring gauge.

## Branch Fuse Block

Use a small branch fuse block so each load can be protected and serviced independently.

## 12V Bus

A high-current buck converter steps the 20V/24V input down to a fused 12V bus. The 12V bus feeds BTS7960 motor power and the thrower power output. Do not power the wiper motor from the 5V buck.

## Thrower Circuit

The thrower should have its own fused 12V branch. Keep its high-current wiring physically separated from low-voltage control wiring where practical.

The thrower trigger relay does not power the thrower. It only closes a dry contact across the same two wires used by the factory foot pedal.

## Wobbler Motor Circuit

The BTS7960 motor supply branch comes from the 12V bus and powers the Mitsubishi Outlander rear wiper motor. Start with conservative fuse sizing based on measured current.

## Actuator Circuit

Reserve a fused branch for the future pitch actuator. Expect startup and stall current to be higher than running current.

## 5V Buck Converter Circuit

A fused 5V buck converter is fed from the 12V bus. The 5V bus feeds ESP32 `VIN`/`5V`, the relay module, BTS7960 logic, and support modules as appropriate. ESP32 `3V3` feeds OLED, BME280, DHT11, and small logic sensors. All grounds must be common.

## Trigger Relay Circuit

The ESP32 powers only the relay module input side. The relay contact side is isolated and connects to the VEVOR NH113 pedal circuit as `COM` and `NO`. Keep this circuit dry-contact only; never inject ESP32 or bus voltage into the thrower foot-pedal trigger circuit.
