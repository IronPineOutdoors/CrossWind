#include "storage.h"

#include <Preferences.h>

static Preferences prefs;
static const char* NAMESPACE = "crosswind";

void beginStorage() {
  prefs.begin(NAMESPACE, false);
}

StoredSettings loadSettings() {
  StoredSettings settings;
  settings.mode = (Mode)prefs.getUChar("mode", SWEEP);
  settings.lastFault = (FaultCode)prefs.getUChar("fault", FAULT_NONE);
  settings.lastSpeed = prefs.getUChar("speed", DEFAULT_PWM);

  if (settings.mode > CENTERING) {
    settings.mode = SWEEP;
  }
  if (settings.lastSpeed > MAX_PWM) {
    settings.lastSpeed = DEFAULT_PWM;
  }

  return settings;
}

void saveSettings(const ControllerState& state) {
  prefs.putUChar("mode", (uint8_t)state.mode);
  prefs.putUChar("fault", (uint8_t)state.lastFault);
  prefs.putUChar("speed", state.speed);
}
