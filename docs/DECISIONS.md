# Engineering decisions

Durable choices supported by repository evidence or the explicit context task. Status distinguishes an accepted direction from completed hardware. Current verification gaps belong in [STATUS](STATUS.md); future work belongs in [roadmap](roadmap.md). Revisit with new evidence, preserving the reason for any superseding decision.

## Decision: ESP32 is the Alpha controller; retain AVR as legacy

Status: Established for Alpha.

Date or commit if known: `946dc68`.

Decision: Use the modular ESP32 PlatformIO project for Alpha; retain AVR separately without assuming feature or safety parity.

Reason: The Alpha motion record and AVR README explicitly distinguish validated ESP32 motion from the older fallback.

Consequences: AVR requires separate endpoint, timeout and centering review/porting before Alpha use. Production fallback policy remains open.

Revisit when: A concrete controller requirement or separately validated AVR port warrants reconsideration.

Evidence: [ESP32 overview](../firmware/crosswind-esp32/README.md), [AVR limitations](../firmware/crosswind-arduino/README.md).

## Decision: IBT-2 drive and installed direction mapping

Status: Implemented; latest mapping awaits physical verification.

Date or commit if known: `92c20dd`, `3563fba`.

Decision: Use both bridge enables during drive, one PWM output at a time, zero all outputs on stop, and preserve ramp/dead time. Invert the output mapping for the reported installed motor direction.

Reason: The bridge needs both halves enabled; the later bench report identified logical/physical travel reversal.

Consequences: Logical limits keep their identities. Do not also swap motor leads; verify both physical endpoints after flashing.

Revisit when: Motor/linkage/wiring changes or new measured direction evidence.

Evidence: [motor.cpp](../firmware/crosswind-esp32/src/motor.cpp), [bench follow-up](../testing/2026-09-10-bench-results.md).

## Decision: Powered limits define normal reversal boundaries

Status: Established Alpha standard.

Date or commit if known: `4fd82fb`, `24c9aa8`, `946dc68`.

Decision: Use powered 3.3 V active-LOW YL-99 endpoints with the Deutsch DT six-pin harness; reserve lower/upper conductors for future pitch. Use separate mechanical stops.

Reason: Module levels were bench-verified; normal endpoint reversal and fault protection are both required.

Consequences: GPIO34/35 rely on module pull-ups. Open wires are not automatically detected. Maintain release/both-limit/travel protections and inspect continuity.

Revisit when: Different switch hardware, supervised wiring requirements or the pitch implementation requires an explicit revision.

Evidence: [Harness standard](../electrical/limit-switch-harness.md), [switch layout](../mechanical/phase-1/limit-switch-layout.md).

## Decision: SSD1309 five-signal harness and shared BME280 bus

Status: Established Alpha standard.

Date or commit if known: `3d319ab`, `1f1f832`, `a95ddf6`.

Decision: Retain the verified Hosyond 2.42-inch SSD1309, 3.3 V supply, GPIO5 reset and canonical five-position OLED harness; share I2C with BME280.

Reason: Cold-start and shared-bus fixes establish reset, timing/recovery and plausible environmental readings as working behavior to preserve.

Consequences: Use the numbered harness table, not prose signal order. Record exact connector family before buying replacements; measure enclosure interfaces. BME280 is telemetry, not a certified safety instrument.

Revisit when: A measured replacement interface or tested panel migration requires a deliberate change.

Evidence: [OLED standard](../electrical/oled-lid-harness.md), [safety](safety-notes.md), [display source](../firmware/crosswind-esp32/src/display.cpp).

## Decision: Local control and scoped Alpha modes

Status: Implemented; distinct Random/Flush deferred.

Date or commit if known: `c093f05`; 2026-09-10 record.

Decision: Operate locally with encoder menus and dedicated ARM/FIRE; BLE is optional. Keep RANDOM/FLUSH as sweep aliases for scoped Alpha testing and timed centering as an estimate.

Reason: Local operation is a requirement; the September record explicitly accepts shared sweep behavior until Alpha X2.

Consequences: Encoder selection never fires. Do not claim distinct patterns, position feedback or automatic throws; preserve trigger interlocks.

Revisit when: Defined Alpha X2 presentation behavior and validation criteria are available.

Evidence: [Requirements](product-requirements.md), [mode implementation](../firmware/crosswind-esp32/src/modes.cpp), [bench scope](../testing/2026-09-10-bench-results.md).

## Decision: Fused power buses and dry-contact triggering

Status: Documented architecture; assembly acceptance pending.

Date or commit if known: Date not established.

Decision: Plan tool-battery input through a main fuse and high-current 12 V buck, with branch fuses and a separate 5 V electronics buck; keep the thrower relay dry-contact in parallel with the factory pedal.

Reason: Separate load branches support protection/service; the relay emulates a switch without injecting voltage into the pedal circuit.

Consequences: Measure current before final fuse sizing, maintain common power/logic ground and reachable disconnect, and continuity-check the actual pedal before connection.

Revisit when: Measured loads or confirmed thrower interface changes require a revised power/trigger design.

Evidence: [Power](../electrical/power-system.md), [fuses](../electrical/fuse-plan.md), [trigger](../electrical/thrower-trigger.md).

## Decision: Thrower deck and adjustable fitment

Status: Required terminology and established fitment strategy.

Date or commit if known: Terminology: this context task, 2026-09-13; fitment date not established.

Decision: Call the moving thrower-support platform the thrower deck. Retain adjustable mounting/rails with NH113 as the first fitment target.

Reason: The task defines the term; requirements and fitment notes call for support beyond a single thrower footprint.

Consequences: Older top-plate wording means thrower deck in this context. Nominal dimensions are not proof of a measured interface or universal compatibility; preserve adjustability and verify load paths.

Revisit when: Measured fitment or structural evidence requires a deliberate mechanical revision.

Evidence: [Requirements](product-requirements.md), [NH113 fitment](../mechanical/phase-1/vevor-nh113-fitment.md).

## Decision: IPC-101 migration uses a separate adapter

Status: Selected development direction; not integrated.

Date or commit if known: `bd85eef`, `8922fa8`, `a77c183`, `e9f0994`.

Decision: Develop IPC-101 with four-position controls and six-position display connections using matching hook-retained PCB headers, plus a distinct isolated STOP connector. Preserve the current Alpha harness.

Reason: The recorded selection supports a replaceable equipment adapter for Alpha and later IPC-100; user-confirmed mating supersedes the loose middle-header concept.

Consequences: No TCA8418 driver or STOP integration exists in current firmware. Preserve independent panel branches; verify cavity orientation, continuity and provisional geometry before releasing hardware. Superseded fit coupons remain diagnostic history.

Revisit when: Assembly/PCB arrival testing or verified interface evidence changes the design.

Evidence: [Adapter plan](../electrical/ipc101-adapter-plan.md), [connector development](../cad/phase-1/controller-connector/README.md).
