# Limit Switch Layout

Crosswind Alpha uses two YL-99 roller limit switch modules as firmware travel endpoints and safety/calibration inputs.

- Mount the YL-99 modules on adjustable brackets attached to the stationary 28" x 28" base plate.
- Mount adjustable flags or tabs on the rotating top plate.
- Place flags near the rotating plate edge or a rear tab where they can be adjusted without disturbing the crank linkage.
- The flags should contact the YL-99 roller arms lightly and repeatably.
- The switch roller arms should not carry mechanism loads.
- The switches command normal endpoint stop/dwell/reversal, but are not physical hard stops.
- Add separate rubber or metal hard stops if the top plate needs mechanical end-of-travel protection.
- Route switch wiring so it cannot touch the lazy susan bearing, linkage, crank arm, or rotating plate.

Firmware treats the expected single switch as a normal endpoint. Both switches, an unexpected switch, a switch that fails to release while departing, or travel without an endpoint are faults.
