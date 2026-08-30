# Side Panel Layout

This document records measured hardware and the intended control layout for the Crosswind Alpha side panel. All dimensions are millimetres.

## Viewing convention

Locations are described while looking at the finished outside/front face of the panel. The OLED connector is on the viewer's left. Use the OLED horizontal centerline as the vertical datum for the encoder. The final panel origin and overall panel dimensions are not yet defined.

## OLED assembly

| Feature | Dimension or location |
| --- | --- |
| PCB envelope | 73 wide x 43 high |
| PCB mounting holes | M3, four holes |
| Left-to-right mounting-hole span | 70 outside-to-outside as initially measured; fit print requires each side moved 1 outward (2 total horizontal correction) |
| Top-to-bottom mounting-hole span | 42 outside-to-outside |
| Display body | 57 wide x 28 high x 2 proud of PCB |
| Connector | Left side when viewed from the panel front |
| Vertical placement | 15 below panel top; datum requires verification |

The mounting-hole spans above were reported as outside-to-outside, not center-to-center. Verify the hole diameter and hole-center coordinates directly from the PCB before generating the drilling pattern. Also verify whether the 15 mm top offset is to the OLED PCB edge, display-body edge, or another datum.

## Controls and indicators

| Feature | Panel feature | Intended location |
| --- | --- | --- |
| ARM button | 16.5 diameter barrel/cutout, verify fit allowance | 13 below the OLED |
| FIRE button | Same button type unless otherwise specified | 70 right of ARM, center-to-center |
| Rotary encoder | 8 diameter threaded-bushing hole; recessed `SPEED` and `-` / `+` markings; paired rear anti-rotation rails provisionally fit an 18-wide PCB | 30 right of OLED, vertically centered on OLED |
| RGB status LED | 5.2 diameter provisional hole; paired rear anti-rotation rails provisionally fit a vertically oriented 15-wide PCB | Horizontally centered between ARM and FIRE; 11 below OLED, with the connector facing down |
| BME280 | Mounting tab with hole | Tab location and clearance envelope not yet specified; sensor mounting hole is 2 diameter |
| Master power toggle | Cutout not yet specified | Reserve an accessible location after switch body and electrical ratings are known |

Unless later measurements say otherwise, relative offsets in this table should be interpreted as center-to-center. Do not release the side panel for cutting until the ambiguous OLED datums and all missing clearances below are resolved.

## Measurements still needed

- Overall side-panel width and height, edge radii, thickness, and usable internal clearance.
- OLED visible-window size and offset relative to the PCB, plus the required bezel overlap.
- OLED mounting-hole diameter and center-to-center coordinates.
- Meaning of the OLED's 15 mm top offset and of the 13 mm/11 mm below-OLED offsets (edge gap or center datum).
- ARM and FIRE button thread/body diameter, anti-rotation feature, flange diameter, and rear clearance.
- Encoder body, washer, anti-rotation tab, knob, and rear connector clearances.
- Power-toggle bushing/cutout, anti-rotation feature, body and terminal envelope, and DC voltage/current rating.
- RGB LED lens diameter, retaining hardware, PCB envelope, mounting holes, and rear clearance.
- BME280 PCB width/height/thickness, exact mounting-hole coordinates, sensing-port airflow needs, connector side, and desired tab position.
- Five-pin OLED connector envelope, cable exit direction, bend radius, strain relief, and service loop.
