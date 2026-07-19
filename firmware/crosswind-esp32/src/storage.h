#pragma once

#include "config.h"

struct StoredSettings {
  Mode mode;
  FaultCode lastFault;
  uint8_t lastSpeed;
  uint32_t fullTravelTimeMs;
  uint16_t storageVersion;
  bool batteryMonitorEnabled;
  BatteryProfile batteryProfile;
  float batteryCalibrationMultiplier;
  float batteryCalibrationOffsetV;
  uint8_t displayContrast;
  uint16_t displayStatusTimeoutSeconds;
};

void beginStorage();
StoredSettings loadSettings();
void saveSettings(const ControllerState& state);
void saveCalibration(uint32_t fullTravelTimeMs);
void saveBatterySettings(bool enabled, BatteryProfile profile, float calibrationMultiplier, float calibrationOffsetV);
void saveUiSettings(uint8_t contrast, uint16_t statusTimeoutSeconds);
