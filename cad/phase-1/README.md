# CAD Phase 1

Use this folder for the yaw base, top plate, switch brackets, motor mount, and electronics box layout.

Printable adjustable YL-99 switch holders are in [`limit-switch-holder`](limit-switch-holder/README.md).
Matching printable striker flags are in [`limit-switch-flag`](limit-switch-flag/README.md).

## Controller Lid

The official Alpha controller lid uses the Hosyond 2.42-inch 128x64 SSD1309 OLED and its five-conductor JST harness: GND, 3.3 V, SDA, SCL, and GPIO5 RESET. Future enclosure and wiring-layout CAD must reserve routing, bend clearance, strain relief, and service-loop space for all five conductors and the five-position connector.

The current measured side-panel layout is recorded in [side-panel-layout.md](side-panel-layout.md). The OLED module and primary control locations are measured; dimensions explicitly marked for verification must not be treated as fabrication-ready.

TODO: measure the JST housing, visible OLED window, standoffs, bezel overlap, cable bend radius, service loop, RGB LED lens/body/PCB, and BME280 board envelope before producing a final dimensional drawing. Do not infer missing dimensions from a generic part name.
