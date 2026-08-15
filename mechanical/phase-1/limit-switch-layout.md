# Limit Switch Layout

Crosswind Alpha uses two YL-99 roller limit modules as normal reversal boundaries and safety inputs.

- Mount the YL-99 modules on adjustable brackets attached to the stationary 28" x 28" base plate.
- Mount adjustable flags or tabs on the rotating top plate.
- Place flags near the rotating plate edge or a rear tab where they can be adjusted without disturbing the crank linkage.
- The flags should contact the YL-99 roller arms lightly and repeatably.
- The switch roller arms should not carry mechanism loads.
- The switches command firmware reversal but are not load-bearing physical hard stops.
- Add separate rubber or metal stops just beyond the switch actuation points for independent overtravel protection.
- Route switch wiring so it cannot touch the lazy susan bearing, linkage, crank arm, or rotating plate.
- Use Green for Left OUT, Blue for Right OUT, Black for shared ground, and Red for regulated 3.3 V in accordance with the [Deutsch DT 6-pin harness standard](../../electrical/limit-switch-harness.md).
- Keep the terminated White and Yellow conductors insulated and secured during Alpha.

Firmware stops and reverses away from the single limit reached in the direction of travel. It latches a fault if both switches activate, the departed switch fails to release within `LIMIT_DWELL_MS`, or the next endpoint is not reached within `MAX_TRAVEL_TIME_MS`.
