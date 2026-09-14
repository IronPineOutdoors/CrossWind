# Branch consolidation: 2026-09-13

## Accepted baseline

Main advances from `4fd82fb` to the complete Alpha history through `dc6def9`, including the context cleanup. This is a fast-forward: no competing firmware implementations were combined and no Alpha source, wiring assignments, safety settings or CAD geometry were changed by consolidation. GitHub main was at `c093f05`, two commits behind the previous local main.

The former Alpha branch's 46 commits contain bounded motion/centering, subsequent motor-direction correction, CAD and connector development, test records and context documentation. They are retained in main's normal history. The existing release tag `v1.12.0-rc.1` remains at its original commit; the current source still uses that version string, so record the flashed SHA during testing.

## Older feature work reviewed

The July branches are stacked: motion engine -> battery monitor -> display menu. They forked before the later IBT-2, SSD1309/reset, I2C and local-control fixes. Their three unique commits are preserved under annotated archive tags; they are not merged or represented as current features.

| Former branch | Tip | Archive tag | Disposition |
| --- | --- | --- | --- |
| feature/motion-engine | `7c12f7c` | `archive/2026-09-13/motion-engine` | Current Alpha modes supersede its basic motion implementation. Preserve distinct Random/Flush, calibration, unexpected-limit handling and simulation ideas for deliberate future review. |
| feature/battery-monitor | `e80013e` | `archive/2026-09-13/battery-monitor` | Unique deferred implementation, not obsolete functionality. Preserve ADC filtering, calibration, voltage-state logic and simulation material for a separate port after hardware verification. |
| feature/display-menu | `d403904` | `archive/2026-09-13/display-menu` | Current SSD1309/encoder menus supersede its display/control integration. Preserve edit/confirmation, timeout and simulation ideas for future UX work. |

Concrete incompatibilities: the old motor output function enables only one bridge half per direction, predating `92c20dd`; the old display uses Adafruit SSD1306 with reset disabled, predating the installed SSD1309/U8g2 and GPIO5 reset. The battery/menu commits depend on the older motion implementation and introduce different settings and application gates. Cherry-picking them wholesale would not preserve the current hardware contract.

Archive recovery example (read-only):

```sh
git show archive/2026-09-13/battery-monitor:firmware/crosswind-esp32/battery-monitor.md
```

The archive retains the original source, documentation and test scripts. Those scripts target the old architecture and do not validate main. Battery monitoring, distinct modes and advanced menu behavior are not silently enabled by this cleanup. Current gaps remain in [STATUS](STATUS.md), with future integration tracked in [roadmap](roadmap.md).

## Verification

- ESP32 PlatformIO release build passed on the consolidated Alpha source: Espressif32 7.0.1, Arduino-ESP32 2.0.17, U8g2 2.36.18, BME280 2.3.0 and Unified Sensor 1.1.15.
- RAM: 41,188 / 327,680 bytes; flash: 1,200,277 / 1,310,720 bytes (91.6%). These are build results, not a new hardware acceptance.
- Repository-local Markdown links and whitespace checks are checked during closeout.
- AVR source is unchanged; no local Arduino CLI was available for an AVR build. Existing CI still compiles both controllers.
- No firmware upload, motor/relay exercise, electrical acceptance or CAD regeneration was performed. Direction correction, retention and loaded testing remain pending.

The unrelated user-staged plate-spacer 3MF is preserved outside this consolidation commit.
