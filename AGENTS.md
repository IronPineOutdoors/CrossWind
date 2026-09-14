# CrossWind working guide

## Purpose and architecture

CrossWind is Iron Pine Outdoors' adjustable clay-thrower wobbler base. Alpha / Phase 1 is single-axis yaw; Phase 2 adds pitch. ESP32 Arduino-framework firmware in PlatformIO controls a BTS7960 / IBT-2 and reversing wiper motor, powered YL-99 endpoints, local encoder/menu and ARM/FIRE controls, SSD1309 OLED, BME280, RGB status and dry-contact relay. BLE is optional for operation. AVR is a legacy fallback, not Alpha-equivalent. IPC-101 is a planned panel migration, not the installed input implementation.

## Branch workflow

Use `main` for routine project work unless the user explicitly requests an isolated branch. Check the current branch and staged/unstaged changes before editing. Keep feature branches short-lived, reconcile them into main after verification, and do not leave ordinary follow-up work on a release-candidate branch. Preserve unique deferred work with an archive tag before removing its branch; an archive is not an accepted implementation. Do not silently merge old hardware assumptions into current firmware.

## Start with evidence

Read [STATUS](docs/STATUS.md), [decisions](docs/DECISIONS.md), [safety notes](docs/safety-notes.md), and affected subsystem documentation before editing. Inspect actual implementation, configuration, relevant test records and recent Git history before substantial changes.

Use this source hierarchy within its scope:

1. Current task instructions establish requested scope and terminology.
2. Dated measurements and test records establish only what was physically observed on the recorded hardware/revision. Source code establishes implemented behavior, not what is flashed or tested.
3. Canonical subsystem specifications establish intended wiring, interfaces and geometry; compare them with code and measured hardware before changing either.
4. STATUS summarizes evidence; DECISIONS records rationale. Requirements and build plans describe intent. Roadmap describes future work; README is an entry point, not acceptance evidence.

Repository evidence wins over stale prose. If sources conflict, identify both, their dates/revisions and the missing verification in STATUS. Do not arbitrarily select a dimension, pinout or safety interpretation. Never silently invent dimensions, pin assignments, connector orientation, interfaces or compatibility. Ask for a required measurement or decision before dependent fabrication; continue independent work where possible.

## Terminology and layout

Use **thrower deck** for the moving platform/top plate supporting the thrower; distinguish it from the fixed base and electronics plates. LEFT/RIGHT mean deck travel toward the named physical endpoints from a consistent operator viewpoint, not motor shaft rotation. Alpha is the current yaw prototype; Alpha X2 / Phase 2 denotes future expansion where referenced in existing notes.

Preserve the existing organization:

- `firmware/`: ESP32 implementation and separate AVR fallback.
- `electrical/`: canonical pinout, harnesses, power, fuses and trigger interfaces.
- `mechanical/`: dimensions, fitment, linkage and build notes by phase.
- `cad/`: parametric sources, generators, exports, fit coupons and assembly references.
- `testing/`: procedures and dated physical results; blank logs are not tests.
- `docs/`: context, requirements, plans, safety and history. Nested subsystem aliases point to canonical files; do not duplicate their content.
- `branding/` and `assets/`: identity and supporting media.

## Firmware constraints

Prefer incremental, testable changes. Preserve working motor, display, input, stop, limit, ARM/FIRE and fault behavior unless deliberately changing it. Never silently change pins, harness mappings, safety flags, thresholds or persistent settings. Document hardware-dependent assumptions and settings migration implications.

Read [config.h](firmware/crosswind-esp32/src/config.h), affected modules and [firmware guidance](firmware/crosswind-esp32/README.md). Keep motor outputs owned by motor.cpp: both enables on for drive, only one PWM active; stop clears both PWM outputs and enables; preserve ramping and reversal dead time. The installed direction correction changes output mapping, not limit identities; do not also swap leads without evidence.

Preserve SAFE/unarmed boot, relay-off startup, trigger qualification and timing, fault stop/disarm/cancel behavior and conservative recovery. Local controls must work without BLE. Preserve OLED reset/cold-start recovery, shared I2C timing/timeouts and BME280 plausibility/retry handling. Disabled E-stop, sensing and diagnostic hooks are not installed protections. Do not infer parity from AVR's matching mode names.

## Electrical and motion safety

Follow [pinout](electrical/pinout.md), [limit harness](electrical/limit-switch-harness.md), [OLED harness](electrical/oled-lid-harness.md), [power](electrical/power-system.md), [fuses](electrical/fuse-plan.md) and [trigger wiring](electrical/thrower-trigger.md). Confirm connector cavity orientation and power-off continuity before energizing; do not infer OLED pin order from a prose signal list.

Retain main fuse near battery, branch fusing, common logic/power ground, strain relief and reachable power disconnect. Never apply more than 3.3 V to ESP32 GPIO or feed motor power from the 5 V electronics branch. Relay contacts remain dry-contact only; verify the factory pedal interface before connection.

YL-99 modules define reversal boundaries, not load-bearing stops. Retain separate mechanical stops beyond switch actuation. Their powered active-LOW wiring does not detect broken wires automatically. Preserve both-limit, release-timeout and travel-timeout fault handling. Keep people clear of linkage, deck and thrower arm; secure the base. Bench-test without the thrower mounted after firmware or wiring changes. Test fault simulations without driving into or holding against a hard stop. Trigger testing requires an unloaded thrower pointed safely, with everyone clear.

## Mechanical and CAD work

Favor practical, buildable solutions and adjustable rails/flags for multiple throwers; NH113 is the first fitment target, not proof of universal compatibility. Mark measured, derived, provisional and clearance dimensions distinctly, with units and datums. Make important dimensions parametric where practical. Do not generate final geometry around an unverified interface; explicitly labeled fit coupons may explore it.

Inspect existing generators and later fit feedback before editing geometry. Preserve useful native sources, exports and visual references/renders; add assembly views when useful. Regenerate affected exports and run the generator's existing geometry checks. Verify units, orientation, fit, fastener access, load paths, travel/pinch clearances and cable routing. A watertight STL or accepted coupon does not prove structural strength, electrical mating or finished assembly fit.

## Verification and documentation

For ESP32 changes run `pio run` in `firmware/crosswind-esp32/`; AVR changes use `arduino-cli compile --fqbn arduino:avr:uno firmware/crosswind-arduino/Crosswind_Controller`, matching CI. Compilation is not hardware validation. Follow the [firmware checklist](firmware/crosswind-esp32/firmware-test-checklist.md), [fault matrix](firmware/crosswind-esp32/fault-matrix.md), [bench checklist](testing/bench-test-checklist.md) and latest dated results for affected hardware. Any motion/trigger change must explicitly report how safe behavior was preserved and what was tested or remains untested. Record flashed commit, wiring/configuration, load, supply, PWM, runtime and measurements for physical tests.

Update affected subsystem docs with implementation changes. Keep current evidence in STATUS, future tasks in roadmap, durable rationale in DECISIONS and history in changelog. Link to canonical detail rather than repeating tables. Preserve useful historical files; flag superseded content instead of deleting it merely for age. Check touched Markdown links and documented paths, run `git diff --check` and relevant existing checks, and review the final diff. For documentation-only work, do not claim firmware or hardware validation.

At every natural pause or handoff after project changes, create a Git commit containing only this task's changes. Preserve unrelated user edits and exclude them from the commit. If committing is impossible, report the exact blocker before pausing.
