# Build Plan: Phase 1

## Base Construction

Build a 28" x 28" base from 3/4" plywood. Seal edges if the prototype will be used outdoors. Leave enough edge margin for stake tabs, handles, or tie-down points.

## Top Plate

Cut a 20" x 20" rotating top plate. The thrower rails or temporary fitment blocks mount to this plate.

## Bearing Install

Center a 12" lazy susan bearing between the base and top plate. Mark the centerlines before drilling so the plate rotates squarely.

## VEVOR NH113 Test Fit

Use the VEVOR NH113 as the first fitment target. Keep rails or blocks adjustable instead of drilling a single-purpose pattern into the top plate.

## Wiper Motor Mount

Mount the Mitsubishi Outlander rear wiper motor to the fixed base. The motor should be serviceable and should not interfere with the rotating top plate.

## Linkage

Use a simple crank/linkage from the motor output to the rotating plate. Start with conservative travel and low PWM before widening the sweep angle.

## Roller Switch Placement

Mount left and right roller switches on adjustable stationary brackets. A single striker tab on the rotating plate should actuate the switches near each travel end.

## Controller Box

Mount the ESP32, BTS7960, buck converter, fuse block, and wiring terminals in a waterproof electronics box. Add strain relief for motor, battery, switch, and control wiring.

Add an opto-isolated trigger relay if thrower launch control is being tested. Keep the factory VEVOR NH113 pedal connected and wire the relay contacts in parallel with the pedal switch.

## First Powered Test

Remove the thrower. Power the ESP32 first, verify Serial diagnostics, verify switch states, then power the motor driver and test low-PWM sweep.

Test trigger output only after confirming the thrower is unloaded, pointed safely, and the black/white pedal wires behave like a simple open/short momentary switch.
