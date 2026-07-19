# OLED Display and Menu

The 128×64 SSD1306 at I2C address `0x3C` is split into two responsibilities: `menu.cpp` owns navigation, edit/confirmation state, timeouts, notifications, and validated intents; `display.cpp` renders the current model. `main.cpp` alone accepts or rejects intents and performs motion, trigger, calibration, fault clear, and persistence operations.

## Controls

- Rotate on Status: adjust runtime speed within 0–255. This does not write flash per detent.
- Short encoder press: enter/select/confirm.
- Long encoder press (800 ms): cancel an edit or return to Status.
- ARM: remains a dedicated safety control on GPIO16.
- FIRE / TEST: remains a dedicated gated trigger control on GPIO17.

## Menu map

```text
Status
Mode -> SWEEP / RANDOM / FLUSH / CENTERING
Motion -> Speed / Start or Stop
Trigger -> availability / ARM state / pulse / interval
Battery -> voltage / status / profile / minimum / calibration
Environment -> BME280 temperature / humidity / pressure / status
Calibration -> Center / Motion calibration / Battery known voltage / Reset battery calibration
Diagnostics -> system / motion / battery and I/O pages
Settings -> Contrast / Status timeout / Save / Restore defaults / Firmware info
Back
```

Motion mode changes are rejected while running. Movement calibration and centering require a warning confirmation. Battery calibration uses a working known-voltage value in 0.1 V steps and requires confirmation. CENTERING never exposes a trigger action.

Edits use a RAM working copy. Short press confirms; long press cancels without saving. Contrast and status timeout become dirty settings and are written only through Save. Restore Defaults requires confirmation and restores UI defaults, SWEEP, and default speed while stopped.

## Screen behavior

The boot screen is rendered without delaying initialization or safety checks. The UI returns from menus to Status after the configured inactivity timeout, dims after 60 seconds, and turns the OLED off after 120 seconds. Encoder, ARM, FIRE, notifications, and faults wake it. Motion keeps it awake. Display sleep never changes controller operation.

A latched fault always replaces the current screen and shows the fault, disabled outputs, and blocked-clear reason. Short press only requests the existing safe clear pathway; it cannot bypass limits, E-stop, or battery recovery.

BLE and local controls share the existing controller/settings state. BLE updates generate a local notification and current values render immediately. If BLE changes a value during a local edit, the local working copy is deliberately retained until confirm or cancel; confirming then becomes the newest authoritative value.

## Text examples

```text
CROSSWIND STATUS
SAFE  STOP SWEEP
IDLE RIGHT
SPD 120 PWM 0
BAT 20.1V BLE ON
WARN OK
```

```text
!!! FAULT !!!
LIMIT_STUCK
MOTION: DISABLED
TRIGGER: DISABLED
Release limit first
Press:Attempt clear
```
