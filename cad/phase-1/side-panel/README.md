# Alpha Side Panel Print

`crosswind_alpha_side_panel.stl` is a provisional, flat-print prototype for checking the OLED and control layout. Print it flat on the bed; supports are not required.

## Current geometry

- Panel: 150 x 100 x 3 mm
- Four panel mounting holes: 3.2 mm diameter (M3 clearance), centers 5 mm from each corner
- OLED visible opening: 57 x 28 mm
- Four OLED PCB holes: 3.2 mm diameter
- ARM and FIRE openings: 16.5 mm diameter, 70 mm center-to-center
- Encoder opening: 6 mm diameter
- RGB LED opening: provisional 5.2 mm diameter
- Upper-right recessed wordmark: `CROSSWIND` / `BY IRON PINE OUTDOORS`, 0.6 mm deep

The panel size, OLED window placement, button vertical datum, LED diameter, and interpretation of the reported OLED mounting-hole spans are provisional. Use this print as a fit coupon/prototype, not as the final weather-sealed panel.

The BME280 mounting tab is intentionally omitted until its PCB envelope, hole coordinates, connector orientation, airflow clearance, and desired location are measured. Those details determine whether the tab belongs on the rear face or panel edge.

The engraved lockup uses a dependency-free block alphabet sized for FDM printing. For the cleanest lettering, print the exterior face upward at 0.2 mm layers or finer. A contrasting paint fill can be wiped into the recess after printing.

## Regeneration

Install the generator dependency and run:

```powershell
python -m pip install manifold3d
python .\generate_stl.py
```

Edit the constants at the top of `generate_stl.py` when measured dimensions replace the provisional assumptions.
