#pragma once

#include "config.h"

struct StoredSettings {
  Mode mode;
  FaultCode lastFault;
  uint8_t lastSpeed;
};

void beginStorage();
StoredSettings loadSettings();
void saveSettings(const ControllerState& state);
