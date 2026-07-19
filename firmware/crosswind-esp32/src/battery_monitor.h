#pragma once

#include "config.h"

enum BatteryStatus {
  BATTERY_DISABLED,
  BATTERY_UNKNOWN,
  BATTERY_NORMAL,
  BATTERY_LOW,
  BATTERY_CRITICAL,
  BATTERY_RECOVERING,
  BATTERY_SENSOR_ERROR
};

struct BatteryDiagnostics {
  int latestRawAdc;
  float filteredAdc;
  float voltage;
  float minimumVoltage;
  float maximumVoltage;
  float lowestMotorRunningVoltage;
  uint32_t lowWarningCount;
  uint32_t criticalEventCount;
  uint32_t sensorErrorCount;
};

void initBatteryMonitor(bool enabled, BatteryProfile profile, float calibrationMultiplier, float calibrationOffsetV);
void updateBatteryMonitor(bool motorRunning);
void resetBatteryFilter();
int batteryRawAdc();
float batteryFilteredAdc();
float batteryVoltage();
BatteryStatus batteryStatus();
const char* batteryStatusToString(BatteryStatus status);
const char* batteryProfileToString(BatteryProfile profile);
const char* batteryMeasurementPointToString();
int batteryApproximatePercent();
bool batteryPercentageValid();
bool consumeBatteryLowEvent();
bool consumeBatteryCriticalEvent();
bool consumeBatterySensorErrorEvent();
bool batterySensorValid();
bool batteryMonitoringEnabled();
bool batteryAllowsOperation();
BatteryProfile selectedBatteryProfile();
float batteryCalibrationMultiplier();
float batteryCalibrationOffsetV();
bool calibrateBatteryToKnownVoltage(float knownVoltage);
void resetBatteryCalibration();
void setBatteryMonitoringEnabled(bool enabled);
bool setBatteryProfile(BatteryProfile profile);
const BatteryDiagnostics& batteryDiagnostics();
bool batterySimulationEnabled();
void setSimulatedBatteryVoltage(float voltage);
void setSimulatedBatteryInvalid(bool invalid);
