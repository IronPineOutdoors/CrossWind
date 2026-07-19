#include "storage.h"

#include <Preferences.h>

static Preferences prefs;
static const char* NAMESPACE = "crosswind";

static bool knownFaultCode(FaultCode fault) {
  switch (fault) {
    case FAULT_NONE:
    case FAULT_TRAVEL_TIMEOUT:
    case FAULT_BOTH_LIMITS:
    case FAULT_BUTTON_STUCK:
    case FAULT_STARTUP_BOTH_LIMITS:
    case FAULT_TEMP:
    case FAULT_LIMIT:
    case FAULT_ESTOP:
    case FAULT_RUN_TIMEOUT:
    case FAULT_OVERCURRENT:
    case FAULT_LIMIT_STUCK:
    case FAULT_UNEXPECTED_LIMIT:
    case FAULT_CALIBRATION_FAILED:
      return true;
    default:
      return false;
  }
}

void beginStorage() {
  prefs.begin(NAMESPACE, false);
}

StoredSettings loadSettings() {
  StoredSettings settings;
  settings.mode = (Mode)prefs.getUChar("mode", SWEEP);
  settings.lastFault = (FaultCode)prefs.getUChar("fault", FAULT_NONE);
  settings.lastSpeed = prefs.getUChar("speed", DEFAULT_PWM);
  settings.fullTravelTimeMs = prefs.getULong("travelMs", 0);

  if (settings.mode > CENTERING) {
    settings.mode = SWEEP;
  }
  if (!knownFaultCode(settings.lastFault)) {
    settings.lastFault = FAULT_UNKNOWN;
  }
  if (settings.lastSpeed > MAX_PWM) {
    settings.lastSpeed = DEFAULT_PWM;
  }
  if (settings.fullTravelTimeMs < MIN_VALID_CALIBRATION_TIME_MS || settings.fullTravelTimeMs > MAX_VALID_CALIBRATION_TIME_MS) {
    settings.fullTravelTimeMs = 0;
  }

  return settings;
}

void saveCalibration(uint32_t fullTravelTimeMs) {
  if (fullTravelTimeMs >= MIN_VALID_CALIBRATION_TIME_MS && fullTravelTimeMs <= MAX_VALID_CALIBRATION_TIME_MS) {
    prefs.putULong("travelMs", fullTravelTimeMs);
  }
}

void saveSettings(const ControllerState& state) {
  prefs.putUChar("mode", (uint8_t)state.mode);
  prefs.putUChar("fault", (uint8_t)state.lastFault);
  prefs.putUChar("speed", state.speed);
}
