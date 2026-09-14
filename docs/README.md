# Docs

Start with [CrossWind overview](../README.md), then use:

- [STATUS](STATUS.md): current implementation, test evidence and uncertainty.
- [Roadmap](roadmap.md): future work.
- [DECISIONS](DECISIONS.md): important choices and why.
- [AGENTS](../AGENTS.md): how Codex should work in this repository.
- [Product requirements](product-requirements.md): intended capabilities and constraints.
- [Phase 1 build plan](build-plan-phase-1.md) and [Phase 2 build plan](build-plan-phase-2.md): construction plans, not completion records.
- [Safety notes](safety-notes.md): required precautions.
- [Changelog](changelog.md): historical changes.

Detailed implementation belongs with its subsystem:

- [Pinout](../electrical/pinout.md) and [wiring checklist](../electrical/wiring-checklist.md).
- [OLED lid harness](../electrical/oled-lid-harness.md) and [limit harness](../electrical/limit-switch-harness.md).
- [Wiper linkage](../mechanical/phase-1/linkage-notes.md) and [limit layout](../mechanical/phase-1/limit-switch-layout.md).
- [ESP32 firmware](../firmware/crosswind-esp32/README.md), [CAD](../cad/phase-1/README.md) and [test records](../testing/README.md).

Existing nested electrical/mechanical aliases are retained for compatibility; update the canonical subsystem files rather than duplicating detail here.
