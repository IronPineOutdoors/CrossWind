# CrossWind status

Evidence snapshot: 2026-09-13, repository reviewed through `606cace`. This distinguishes source implementation, reported physical observations and plans; it is not a new bench acceptance record.

## Confirmed / implemented

Phase 1 yaw is the active prototype. ESP32 source identifies itself as `v1.12.0-rc.1`, with later changes under that same version string. Phase 2 pitch and IPC-101 panel integration are future work. See [configuration](../firmware/crosswind-esp32/src/config.h), [firmware overview](../firmware/crosswind-esp32/README.md) and [adapter plan](../electrical/ipc101-adapter-plan.md).

## Bench-tested

- [2026-08-14 sweep record](../testing/sweep-test-log.md) records no-thrower passes for endpoint reversal, STOP, stuck-limit, both-limit, travel-timeout and timed centering on the Alpha mechanism (`24c9aa8`, `946dc68`). This does not validate every later revision.
- [2026-09-10 results](../testing/2026-09-10-bench-results.md) report motor motion/reversal, both/stuck-limit faults and throwing already working. These are user reports without flashed SHA, PWM, supply, current or runtime. The later follow-up identifies reversed physical motor direction, superseding the initial suspected limit-wiring mismatch.
- `3563fba` corrects output direction in source. The same record explicitly says flashing and physical verification are pending. Field readiness remains unaccepted.

## Mechanically completed

An operating bench mechanism and physical panel/connector fit trials are evidenced, but no complete mechanical acceptance record exists. September 10 records feet mounting and spacer installation pending, plus possible rod-to-motor or flange-to-rod slippage. [Connector notes](../cad/phase-1/controller-connector/README.md) record accepted cable/header coupon fits and user-confirmed hook mating, not final carrier retention. An [assembled header fit prototype](../cad/phase-1/controller-connector/PCB_Header_Assembly_Instructions.md) exists (`e9f0994`); assembly acceptance is pending. The [OLED window correction](../cad/phase-1/side-panel/README.md) (`606cace`) still needs a reprint/physical check.

## Electrically completed

Powered YL-99 module levels were recorded at approximately 3.2 V clear and 0 V pressed in the [limit harness standard](../electrical/limit-switch-harness.md). Motor/limit operation is reported above; the SSD1309 is documented as the verified Alpha display in the [OLED standard](../electrical/oled-lid-harness.md). These do not establish completion of every harness, fuse, enclosure or power-system checklist. Exact OLED JST series/colors and final branch fuse sizes remain unrecorded. No IPC-101 adapter electrical validation is recorded.

## Firmware implemented

- [motor.cpp](../firmware/crosswind-esp32/src/motor.cpp): IBT-2 dual-enable drive, mutually exclusive PWM, zero-output stop, ramp and reversal dead time (`92c20dd`). Current inverted mapping drives logical RIGHT via LPWM and LEFT via RPWM (`3563fba`).
- [modes.cpp](../firmware/crosswind-esp32/src/modes.cpp): bounded sweep and timed centering. RANDOM and FLUSH use the same sweep routine; distinct behavior is explicitly deferred in the September bench record. Centering estimates time, not sensed position.
- [main.cpp](../firmware/crosswind-esp32/src/main.cpp) and [inputs.cpp](../firmware/crosswind-esp32/src/inputs.cpp): encoder menus (Motor, Mode, Environment, Diagnostics, About), speed edit, stopped-only local mode/direction selection, ARM/FIRE gating, fault handling, BLE STOP/disconnect output cancellation (`b3b4dac`, `c093f05`). Boot is stopped/unarmed; encoder press does not fire.
- [display.cpp](../firmware/crosswind-esp32/src/display.cpp) and [environment.cpp](../firmware/crosswind-esp32/src/environment.cpp): SSD1309 reset/retry and 100 kHz shared I2C handling; BME280 dual-address initialization/retry and rejection of implausible readings (`1f1f832`, `a95ddf6`, `accb1ac`). RGB status and nonblocking relay pulses are present.
- [storage.cpp](../firmware/crosswind-esp32/src/storage.cpp): Preferences mode, speed and last-fault history. Stored last-fault history is not automatically restored as an active fault at boot.
- E-stop, battery/current sensing and pitch pins are disabled placeholders; session timeout, overcurrent protection and IBT-2 diagnostic test mode are disabled. Automatic triggering is disabled. AVR remains a [legacy fallback without Alpha parity](../firmware/crosswind-arduino/README.md).

## Not yet verified

Corrected physical direction and both endpoints/start-on-limit behavior; shaft/coupling retention; feet/spacers/stops and complete enclosure/harness installation; loaded NH113 fit, stability and sweep; motor startup/running current, fuse selection, runtime and field durability. [Motor](../testing/motor-test-log.md), [battery](../testing/battery-runtime-log.md) and [field](../testing/field-test-notes.md) logs are blank templates. Repeat control/relay/fault and cold-start OLED/BME280 checks on the actual flashed revision and enclosed wiring. IPC-101 key scanning, RGB and independent STOP integration are not implemented here.

## Known issues / open questions

- Older [pre-motor readiness report](../firmware/crosswind-esp32/docs/testing/Alpha_PreMotor_Bench_Readiness.md) maps RIGHT to RPWM, predating `3563fba`; use current motor source and firmware README for output mapping. Its historical build result is not a current build result.
- [Fault matrix](../firmware/crosswind-esp32/fault-matrix.md) says temperature must fall before clearing. main.cpp's clear gate checks limits/E-stop only; an ongoing temperature fault is subsequently relatched. `ENV ERR` is a warning, not a temperature fault. Clarify/test this behavior before relying on sensor loss as protection.
- Software STOP/BLE disconnect cancels motion and active relay but does not explicitly clear `systemArmed`; it is not an installed hardware E-stop. The unexpected opposite endpoint is not an immediate sweep fault (September record and modes.cpp). These require deliberate safety review, not a documentation-only code change.
- [Mechanical dimensions](../mechanical/phase-1/dimensions.md) and [cut list](../mechanical/phase-1/cut-list.md) agree on nominal 28-inch base, 20-inch thrower deck and 12-inch bearing, but are not measured as-built acceptance. No conflicting nominal base/deck dimensions were found. Motor thread, engagement and retention interface remain unverified.
- Broad measure-the-OLED TODOs in the [mechanical overview](../mechanical/phase-1/README.md) lag the partial measurements in [side-panel layout](../cad/phase-1/side-panel-layout.md). That layout leaves overall size undefined while the prototype uses 150 x 100 x 3 mm provisionally. Hole datums/clearances and the latest window shift still need fit verification; do not turn these into final dimensions.
- The [IPC-101 adapter plan](../electrical/ipc101-adapter-plan.md) preserves successive fit reports: later two-dot tail holes and three-dot body openings supersede earlier choices; raised dots, not confirmed tightness, explained tilt. Later measurements partly supersede its initial missing-dimensions list. Provisional pitch/offsets do not release a PCB footprint. Its sibling-project interface references are not self-contained CrossWind evidence and need verification before implementation.
- The existing 6+6+4 Alpha connector arrangement, official five-signal OLED standard and planned IPC-101 4+6 plus separate STOP are different contexts. Installed cavity mapping/adapter continuity must be recorded before migration; do not merge these pinouts. The OLED standard lists SCL at cavity 3 and SDA at 4; unordered signal lists elsewhere are not pin numbering.
- Old calibration messages in [limits.cpp](../firmware/crosswind-esp32/src/limits.cpp) say safety-only and remain placeholders; current endpoints also command reversal. AVR's normally-closed limit wiring is legacy, not the Alpha powered-module standard. Historical diagnostics, fit coupons and fallback code are retained; age alone does not establish dead code.

## Immediate next milestone

Close out unloaded Alpha motion: inspect and secure both shaft interfaces, verify limit identities with motor power off, flash/record the direction-corrected revision, verify low-speed travel/reversal and starts at both endpoints, then record STOP, trigger inhibition and fault recovery. Confirm independent stops, mounted feet/spacers and accessible cutoff before loaded tests. Follow [September closeout](../testing/2026-09-10-bench-results.md) and [safety notes](safety-notes.md); simulate timeouts without stalling against a stop. Future work is tracked in [roadmap](roadmap.md).
