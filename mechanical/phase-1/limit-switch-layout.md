# Limit Switch Layout

Crosswind Alpha uses two YL-99 roller limit switch modules as safety and calibration inputs.

- Mount the YL-99 modules on adjustable brackets attached to the stationary 28" x 28" base plate.
- Mount adjustable flags or tabs on the rotating top plate.
- The flags should contact the YL-99 roller arms lightly and repeatably.
- The switch roller arms should not carry mechanism loads.
- The switches are not physical hard stops and are not normal travel controls.
- Add separate rubber or metal hard stops if the top plate needs mechanical end-of-travel protection.
- Route switch wiring so it cannot touch the lazy susan bearing, linkage, crank arm, or rotating plate.

Firmware treats either active switch during motor operation as a latched limit fault. Both switches active together latch a separate both-limits fault.
