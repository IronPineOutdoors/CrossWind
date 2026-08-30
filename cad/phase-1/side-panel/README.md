# Alpha Side Panel Print

`crosswind_alpha_side_panel.stl` is a provisional, flat-print prototype for checking the OLED and control layout. Print it flat on the bed; supports are not required.

## Current geometry

- Panel: 150 x 100 x 3 mm
- Four panel mounting holes: 3.2 mm diameter (M3 clearance), centers 5 mm from each corner
- OLED visible opening: 57 x 28 mm
- Four OLED PCB holes: 3.2 mm diameter; horizontal span includes the measured fit correction (left pair 1 mm outward and right pair 1 mm outward)
- ARM and FIRE openings: 16.5 mm diameter, 70 mm center-to-center
- Recessed `ARM` and `FIRE` labels above their respective buttons
- Encoder opening: 8 mm diameter for the threaded bushing, with recessed `SPEED` and `-` / `+` direction markings
- RGB LED opening: provisional 5.2 mm diameter, lowered 11 mm below the OLED for vertical PCB clearance
- Paired 3 mm-tall rear anti-rotation rails around the encoder and RGB LED modules
- Upper-right recessed wordmark: `CROSSWIND` / `BY IRON PINE OUTDOORS`, 0.6 mm deep

The encoder rails provisionally fit an 18 mm-wide PCB and the LED rails a 15 mm-wide PCB, each with 0.4 mm total clearance. They resist rotation but do not replace the components' panel nuts or other retention hardware. The STL is oriented with the exterior/engraved face at Z=0 and the rear rails rising upward, so its default orientation prints without supports. Keep the rail contact surfaces clear of solder joints and wiring.

The panel size, OLED window placement, button vertical datum, LED diameter, module widths, and interpretation of the reported OLED mounting-hole spans are provisional. Use this print as a fit coupon/prototype, not as the final weather-sealed panel.

The BME280 mounting tab is intentionally omitted until its PCB envelope, hole coordinates, connector orientation, airflow clearance, and desired location are measured. Those details determine whether the tab belongs on the rear face or panel edge.

A master-power toggle location is not cut yet. Record its panel-bushing or rectangular cutout dimensions, anti-rotation feature, rear body envelope, terminal clearance, and DC voltage/current rating before adding it to the printable panel.

The engraved lockup uses a dependency-free block alphabet sized for FDM printing. For the cleanest lettering, print the exterior face upward at 0.2 mm layers or finer. A contrasting paint fill can be wiped into the recess after printing.

## Regeneration

Install the generator dependency and run:

```powershell
python -m pip install manifold3d
python .\generate_stl.py
```

Edit the constants at the top of `generate_stl.py` when measured dimensions replace the provisional assumptions.
