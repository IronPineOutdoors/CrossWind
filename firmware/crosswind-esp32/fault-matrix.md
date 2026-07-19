# Fault Matrix

| Fault | Source | Motor action | Relay action | Armed state | RGB/OLED | Clear condition |
| --- | --- | --- | --- | --- | --- | --- |
| `LIMIT` | Legacy stored fault from firmware before the motion engine | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault, `FAULT: LIMIT` | Both limits released, then ARM/encoder/CLEAR_FAULT |
| `BOTH_LIMITS` | Both YL-99 switches active during motor operation | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault, `FAULT: BOTH` | Both limits released, then ARM/encoder/CLEAR_FAULT |
| `STARTUP_BOTH_LIMITS` | Both YL-99 switches active at boot | Motor remains stopped | Trigger blocked | SAFE | Red fault | Both limits released, then ARM/encoder/CLEAR_FAULT |
| `TEMP_FAULT` | Environment temperature at or above `TEMP_FAULT_F` | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault / warning status | Temperature below fault threshold, then clear fault |
| `ESTOP` | Optional `ESTOP_PIN` pulled active LOW when configured | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault | Release E-stop, then clear fault |
| `RUN_TIMEOUT` | Optional motor session timeout when enabled | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault | Investigate run duration/session setup, then clear fault |
| `OVERCURRENT` | Optional motor current sense threshold when enabled | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault | Investigate motor/load/wiring, then clear fault |
| `TRAVEL_TIMEOUT` | Motion exceeds `MAX_TRAVEL_TIME_MS` without its expected endpoint | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault | Investigate travel/linkage/switches, then clear fault |
| `LIMIT_STUCK` | Departure endpoint does not release within `LIMIT_RELEASE_TIMEOUT_MS` | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault | Repair/release switch, then clear fault |
| `UNEXPECTED_LIMIT` | Opposite limit activates for the commanded direction, or a command drives into an active endpoint | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault | Investigate wiring/direction, release limits, then clear |
| `CALIBRATION_FAILED` | Calibration measurement is incomplete or outside configured bounds | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault | Investigate motion and switches, then clear fault |
| `BATTERY_CRITICAL` | Filtered voltage remains at/below critical for the configured confirmation interval | Stop/block motor | Cancel/block trigger | Disarm | Red fault | Voltage recovers stably above recovery threshold, then clear fault |
| `BATTERY_SENSOR` | Sustained impossible/disconnected reading when sensor fault latching is enabled | Stop/block motor | Cancel/block trigger | Disarm | Red fault | Repair divider/input, establish stable valid readings, then clear |
| `BUTTON_STUCK` | Reserved fault code | Stop BTS7960 output if latched by future code | Cancel/block trigger | Disarm | Red fault | Release/repair button, then clear fault |

One expected endpoint switch is normal during automatic travel and does not latch a fault. Fault clearing remains conservative: limit-related faults remain latched until both switches read clear, and E-stop faults remain latched until the E-stop is released.
