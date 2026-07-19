#include "battery_monitor.h"

#include <esp_arduino_version.h>

static bool monitorEnabled = false;
static BatteryProfile profile = BATTERY_DEFAULT_PROFILE;
static BatteryStatus status = BATTERY_DISABLED;
static BatteryDiagnostics diagnostics = { -1, NAN, NAN, NAN, NAN, NAN, 0, 0, 0 };
static float calibrationMultiplier = BATTERY_DEFAULT_CALIBRATION_MULTIPLIER;
static float calibrationOffsetV = BATTERY_DEFAULT_CALIBRATION_OFFSET_V;
static float filteredPinMillivolts = NAN;
static unsigned long initializedAt = 0;
static unsigned long lastSampleAt = 0;
static uint8_t validSampleCount = 0;
static uint8_t lowSamples = 0;
static uint8_t criticalSamples = 0;
static uint8_t recoverySamples = 0;
static uint8_t invalidSamples = 0;
static bool lowEvent = false;
static bool criticalEvent = false;
static bool sensorErrorEvent = false;
static bool lastSampleValid = false;
#if CROSSWIND_BATTERY_SIMULATION
static float simulatedVoltage = BATTERY_SIMULATION_DEFAULT_V;
static bool simulatedInvalid = false;
#endif

static float dividerRatio() {
  return (BATTERY_DIVIDER_UPPER_OHMS + BATTERY_DIVIDER_LOWER_OHMS) / BATTERY_DIVIDER_LOWER_OHMS;
}

static bool validProfile(BatteryProfile candidate) {
  return candidate == BATTERY_PROFILE_TOOL_20V || candidate == BATTERY_PROFILE_12V_BUS || candidate == BATTERY_PROFILE_CUSTOM;
}

static void profileThresholds(float& full, float& nominal, float& low, float& critical, float& recovery, float& minimum, float& maximum) {
  if (profile == BATTERY_PROFILE_12V_BUS) {
    full = BATTERY_12V_FULL_V; nominal = BATTERY_12V_NOMINAL_V;
    low = BATTERY_12V_LOW_WARNING_V; critical = BATTERY_12V_CRITICAL_V;
    recovery = BATTERY_12V_RECOVERY_V; minimum = BATTERY_12V_ABSOLUTE_MIN_VALID_V;
    maximum = BATTERY_12V_ABSOLUTE_MAX_VALID_V;
    return;
  }
  full = BATTERY_FULL_V; nominal = BATTERY_NOMINAL_V; low = BATTERY_LOW_WARNING_V;
  critical = BATTERY_CRITICAL_V; recovery = BATTERY_RECOVERY_V;
  minimum = BATTERY_ABSOLUTE_MIN_VALID_V; maximum = BATTERY_ABSOLUTE_MAX_VALID_V;
}

static void clearDebounceCounts() {
  lowSamples = criticalSamples = recoverySamples = invalidSamples = 0;
}

void resetBatteryFilter() {
  diagnostics.latestRawAdc = -1;
  diagnostics.filteredAdc = diagnostics.voltage = NAN;
  filteredPinMillivolts = NAN;
  validSampleCount = 0;
  lastSampleValid = false;
  clearDebounceCounts();
  initializedAt = millis();
  status = monitorEnabled ? BATTERY_UNKNOWN : BATTERY_DISABLED;
}

void initBatteryMonitor(bool enabled, BatteryProfile selectedProfile, float multiplier, float offsetV) {
  monitorEnabled = enabled;
  profile = validProfile(selectedProfile) ? selectedProfile : BATTERY_DEFAULT_PROFILE;
  calibrationMultiplier = multiplier >= BATTERY_CALIBRATION_MULTIPLIER_MIN && multiplier <= BATTERY_CALIBRATION_MULTIPLIER_MAX
    ? multiplier : BATTERY_DEFAULT_CALIBRATION_MULTIPLIER;
  calibrationOffsetV = isfinite(offsetV) && offsetV >= -3.0F && offsetV <= 3.0F ? offsetV : BATTERY_DEFAULT_CALIBRATION_OFFSET_V;
#if !CROSSWIND_BATTERY_SIMULATION
  if (BATTERY_VOLTAGE_PIN >= 0) {
    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(BATTERY_VOLTAGE_PIN, BATTERY_ADC_ATTENUATION);
  }
#endif
  resetBatteryFilter();
}

static bool readSample(int& raw, float& pinMillivolts) {
#if CROSSWIND_BATTERY_SIMULATION
  if (simulatedInvalid) return false;
  float dividerPinV = simulatedVoltage / dividerRatio();
  raw = constrain((int)roundf(dividerPinV / BATTERY_ADC_MAX_PIN_VOLTAGE * BATTERY_ADC_MAX_RAW), 0, (int)BATTERY_ADC_MAX_RAW);
  pinMillivolts = dividerPinV * 1000.0F;
#else
  if (BATTERY_VOLTAGE_PIN < 0) return false;
  raw = analogRead(BATTERY_VOLTAGE_PIN);
  pinMillivolts = (float)analogReadMilliVolts(BATTERY_VOLTAGE_PIN);
#endif
  return raw > 0 && raw < BATTERY_ADC_MAX_RAW && pinMillivolts > 0.0F && pinMillivolts <= BATTERY_ADC_MAX_PIN_VOLTAGE * 1000.0F;
}

static void setSensorError() {
  if (status != BATTERY_SENSOR_ERROR) {
    diagnostics.sensorErrorCount++;
    sensorErrorEvent = true;
  }
  status = BATTERY_SENSOR_ERROR;
}

static void updateStatus(float voltage) {
  float full, nominal, low, critical, recovery, minimum, maximum;
  profileThresholds(full, nominal, low, critical, recovery, minimum, maximum);
  (void)full; (void)nominal;
  if (voltage < minimum || voltage > maximum) {
    invalidSamples++;
    if (invalidSamples >= BATTERY_SENSOR_ERROR_CONFIRM_SAMPLES) setSensorError();
    return;
  }
  invalidSamples = 0;

  if (voltage <= critical) {
    criticalSamples++;
    lowSamples = recoverySamples = 0;
    if (criticalSamples >= BATTERY_CRITICAL_CONFIRM_SAMPLES && status != BATTERY_CRITICAL) {
      status = BATTERY_CRITICAL;
      diagnostics.criticalEventCount++;
      criticalEvent = true;
    }
    return;
  }
  criticalSamples = 0;

  if (status == BATTERY_CRITICAL || status == BATTERY_RECOVERING || status == BATTERY_SENSOR_ERROR) {
    if (voltage >= recovery) {
      status = BATTERY_RECOVERING;
      recoverySamples++;
      if (recoverySamples >= BATTERY_RECOVERY_CONFIRM_SAMPLES) {
        status = BATTERY_NORMAL;
        recoverySamples = 0;
      }
    } else {
      recoverySamples = 0;
    }
    return;
  }

  if (voltage <= low) {
    lowSamples++;
    recoverySamples = 0;
    if (lowSamples >= BATTERY_LOW_CONFIRM_SAMPLES && status != BATTERY_LOW) {
      status = BATTERY_LOW;
      diagnostics.lowWarningCount++;
      lowEvent = true;
    }
  } else if (status == BATTERY_LOW) {
    if (voltage >= recovery && ++recoverySamples >= BATTERY_RECOVERY_CONFIRM_SAMPLES) {
      status = BATTERY_NORMAL;
      lowSamples = recoverySamples = 0;
    }
  } else {
    lowSamples = 0;
    status = BATTERY_NORMAL;
  }
}

void updateBatteryMonitor(bool motorRunning) {
  if (!monitorEnabled) { status = BATTERY_DISABLED; return; }
  unsigned long now = millis();
  if (now - lastSampleAt < BATTERY_SAMPLE_INTERVAL_MS) return;
  lastSampleAt = now;

  int raw = -1;
  float pinMillivolts = NAN;
  if (!readSample(raw, pinMillivolts)) {
    lastSampleValid = false;
    diagnostics.latestRawAdc = raw;
    if (++invalidSamples >= BATTERY_SENSOR_ERROR_CONFIRM_SAMPLES) setSensorError();
    return;
  }

  lastSampleValid = true;
  diagnostics.latestRawAdc = raw;
  if (!isfinite(diagnostics.filteredAdc)) {
    diagnostics.filteredAdc = (float)raw;
    filteredPinMillivolts = pinMillivolts;
  } else {
    diagnostics.filteredAdc += BATTERY_FILTER_ALPHA * ((float)raw - diagnostics.filteredAdc);
    filteredPinMillivolts += BATTERY_FILTER_ALPHA * (pinMillivolts - filteredPinMillivolts);
  }
  if (validSampleCount < 255) validSampleCount++;
  diagnostics.voltage = (filteredPinMillivolts / 1000.0F * dividerRatio()) * calibrationMultiplier + calibrationOffsetV;

  if (!isfinite(diagnostics.minimumVoltage) || diagnostics.voltage < diagnostics.minimumVoltage) diagnostics.minimumVoltage = diagnostics.voltage;
  if (!isfinite(diagnostics.maximumVoltage) || diagnostics.voltage > diagnostics.maximumVoltage) diagnostics.maximumVoltage = diagnostics.voltage;
  if (motorRunning && (!isfinite(diagnostics.lowestMotorRunningVoltage) || diagnostics.voltage < diagnostics.lowestMotorRunningVoltage)) {
    diagnostics.lowestMotorRunningVoltage = diagnostics.voltage;
  }

  if (now - initializedAt < BATTERY_STARTUP_SETTLE_MS || validSampleCount < BATTERY_FILTER_MIN_SAMPLES) {
    status = BATTERY_UNKNOWN;
    return;
  }
  updateStatus(diagnostics.voltage);
}

int batteryRawAdc() { return diagnostics.latestRawAdc; }
float batteryFilteredAdc() { return diagnostics.filteredAdc; }
float batteryVoltage() { return diagnostics.voltage; }
BatteryStatus batteryStatus() { return status; }
static bool voltageWithinProfileRange() {
  float full, nominal, low, critical, recovery, minimum, maximum;
  profileThresholds(full, nominal, low, critical, recovery, minimum, maximum);
  (void)full; (void)nominal; (void)low; (void)critical; (void)recovery;
  return isfinite(diagnostics.voltage) && diagnostics.voltage >= minimum && diagnostics.voltage <= maximum;
}

bool batterySensorValid() { return monitorEnabled && lastSampleValid && voltageWithinProfileRange() && status != BATTERY_UNKNOWN && status != BATTERY_SENSOR_ERROR; }
bool batteryMonitoringEnabled() { return monitorEnabled; }
bool batteryAllowsOperation() { return !monitorEnabled || (lastSampleValid && voltageWithinProfileRange() && (status == BATTERY_NORMAL || status == BATTERY_LOW)); }
BatteryProfile selectedBatteryProfile() { return profile; }
float batteryCalibrationMultiplier() { return calibrationMultiplier; }
float batteryCalibrationOffsetV() { return calibrationOffsetV; }
const BatteryDiagnostics& batteryDiagnostics() { return diagnostics; }

bool consumeBatteryLowEvent() { bool event = lowEvent; lowEvent = false; return event; }
bool consumeBatteryCriticalEvent() { bool event = criticalEvent; criticalEvent = false; return event; }
bool consumeBatterySensorErrorEvent() { bool event = sensorErrorEvent; sensorErrorEvent = false; return event; }

bool calibrateBatteryToKnownVoltage(float knownVoltage) {
  if (!monitorEnabled || !isfinite(diagnostics.voltage) || knownVoltage < BATTERY_CALIBRATION_KNOWN_MIN_V || knownVoltage > BATTERY_CALIBRATION_KNOWN_MAX_V || diagnostics.voltage <= 0.0F) return false;
  float candidate = calibrationMultiplier * knownVoltage / diagnostics.voltage;
  if (candidate < BATTERY_CALIBRATION_MULTIPLIER_MIN || candidate > BATTERY_CALIBRATION_MULTIPLIER_MAX) return false;
  calibrationMultiplier = candidate;
  resetBatteryFilter();
  return true;
}

void resetBatteryCalibration() {
  calibrationMultiplier = BATTERY_DEFAULT_CALIBRATION_MULTIPLIER;
  calibrationOffsetV = BATTERY_DEFAULT_CALIBRATION_OFFSET_V;
  resetBatteryFilter();
}

void setBatteryMonitoringEnabled(bool enabled) { monitorEnabled = enabled; resetBatteryFilter(); }
bool setBatteryProfile(BatteryProfile selectedProfile) { if (!validProfile(selectedProfile)) return false; profile = selectedProfile; resetBatteryFilter(); return true; }

int batteryApproximatePercent() {
  if (!batteryPercentageValid()) return -1;
  const float pointsV[] = { 15.0F, 16.0F, 17.0F, 18.0F, 19.0F, 20.0F, 21.0F };
  const int pointsP[] = { 0, 10, 25, 50, 75, 90, 100 };
  float voltage = diagnostics.voltage;
  if (voltage <= pointsV[0]) return 0;
  if (voltage >= pointsV[6]) return 100;
  for (int i = 1; i < 7; i++) {
    if (voltage <= pointsV[i]) {
      float fraction = (voltage - pointsV[i - 1]) / (pointsV[i] - pointsV[i - 1]);
      return constrain((int)roundf(pointsP[i - 1] + fraction * (pointsP[i] - pointsP[i - 1])), 0, 100);
    }
  }
  return -1;
}

bool batteryPercentageValid() { return BATTERY_PERCENTAGE_ENABLED && profile == BATTERY_PROFILE_TOOL_20V && batterySensorValid(); }

const char* batteryStatusToString(BatteryStatus value) {
  switch (value) {
    case BATTERY_DISABLED: return "DISABLED";
    case BATTERY_UNKNOWN: return "UNKNOWN";
    case BATTERY_NORMAL: return "NORMAL";
    case BATTERY_LOW: return "LOW";
    case BATTERY_CRITICAL: return "CRITICAL";
    case BATTERY_RECOVERING: return "RECOVERING";
    case BATTERY_SENSOR_ERROR: return "SENSOR_ERROR";
    default: return "UNKNOWN";
  }
}

const char* batteryProfileToString(BatteryProfile value) {
  switch (value) {
    case BATTERY_PROFILE_TOOL_20V: return "TOOL_20V";
    case BATTERY_PROFILE_12V_BUS: return "BUS_12V";
    case BATTERY_PROFILE_CUSTOM: return "CUSTOM";
    default: return "UNKNOWN";
  }
}

const char* batteryMeasurementPointToString() {
  switch (BATTERY_MEASUREMENT_POINT) {
    case BATTERY_POINT_TOOL_INPUT: return "TOOL_INPUT";
    case BATTERY_POINT_MOTOR_12V_BUS: return "MOTOR_12V";
    case BATTERY_POINT_CUSTOM: return "CUSTOM";
    default: return "UNKNOWN";
  }
}

bool batterySimulationEnabled() { return CROSSWIND_BATTERY_SIMULATION != 0; }
void setSimulatedBatteryVoltage(float voltage) {
#if CROSSWIND_BATTERY_SIMULATION
  simulatedVoltage = voltage;
  simulatedInvalid = false;
#else
  (void)voltage;
#endif
}
void setSimulatedBatteryInvalid(bool invalid) {
#if CROSSWIND_BATTERY_SIMULATION
  simulatedInvalid = invalid;
#else
  (void)invalid;
#endif
}
