#pragma once

#include "config.h"

struct StoredSettings {
  Mode mode;
  FaultCode lastFault;
  uint8_t lastSpeed;
  uint32_t fullTravelTimeMs;
};

void beginStorage();
StoredSettings loadSettings();
void saveSettings(const ControllerState& state);
void saveCalibration(uint32_t fullTravelTimeMs);
