# Fault Matrix

| Fault | Source | Motor action | Relay action | Armed state | RGB/OLED | Clear condition |
| --- | --- | --- | --- | --- | --- | --- |
| `LIMIT` | Either YL-99 active during motor operation, or active limit at startup | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault, `FAULT: LIMIT` | Both limits released, then ARM/encoder/CLEAR_FAULT |
| `BOTH_LIMITS` | Both YL-99 switches active during motor operation | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault, `FAULT: BOTH` | Both limits released, then ARM/encoder/CLEAR_FAULT |
| `STARTUP_BOTH_LIMITS` | Both YL-99 switches active at boot | Motor remains stopped | Trigger blocked | SAFE | Red fault | Both limits released, then ARM/encoder/CLEAR_FAULT |
| `TEMP_FAULT` | Environment temperature at or above `TEMP_FAULT_F` | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault / warning status | Temperature below fault threshold, then clear fault |
| `ESTOP` | Optional `ESTOP_PIN` pulled active LOW when configured | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault | Release E-stop, then clear fault |
| `RUN_TIMEOUT` | Optional motor session timeout when enabled | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault | Investigate run duration/session setup, then clear fault |
| `OVERCURRENT` | Optional motor current sense threshold when enabled | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault | Investigate motor/load/wiring, then clear fault |
| `TRAVEL_TIMEOUT` | Legacy/shared mode fault code, not expected in current crank-driven Phase 1 sweep | Stop BTS7960 output | Cancel/block trigger | Disarm | Red fault | Investigate cause, then clear fault |
| `BUTTON_STUCK` | Reserved fault code | Stop BTS7960 output if latched by future code | Cancel/block trigger | Disarm | Red fault | Release/repair button, then clear fault |

Fault clearing is intentionally conservative. Limit-related faults remain latched until both limit switches read clear. E-stop faults remain latched until the E-stop input is released.
