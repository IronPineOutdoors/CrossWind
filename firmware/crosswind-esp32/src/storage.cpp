#include "storage.h"

#include <Preferences.h>

static Preferences prefs;
static const char* NAMESPACE = "crosswind";
static const uint16_t STORAGE_VERSION = 2;

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
    case FAULT_BATTERY_CRITICAL:
    case FAULT_BATTERY_SENSOR:
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
  settings.storageVersion = prefs.getUShort("version", 1);
  settings.batteryMonitorEnabled = prefs.getBool("batEnabled", BATTERY_MONITOR_DEFAULT_ENABLED);
  settings.batteryProfile = (BatteryProfile)prefs.getUChar("batProfile", (uint8_t)BATTERY_DEFAULT_PROFILE);
  settings.batteryCalibrationMultiplier = prefs.getFloat("batCalMul", BATTERY_DEFAULT_CALIBRATION_MULTIPLIER);
  settings.batteryCalibrationOffsetV = prefs.getFloat("batCalOff", BATTERY_DEFAULT_CALIBRATION_OFFSET_V);

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
  if (settings.batteryProfile > BATTERY_PROFILE_CUSTOM) settings.batteryProfile = BATTERY_DEFAULT_PROFILE;
  if (!isfinite(settings.batteryCalibrationMultiplier) || settings.batteryCalibrationMultiplier < BATTERY_CALIBRATION_MULTIPLIER_MIN || settings.batteryCalibrationMultiplier > BATTERY_CALIBRATION_MULTIPLIER_MAX) {
    settings.batteryCalibrationMultiplier = BATTERY_DEFAULT_CALIBRATION_MULTIPLIER;
  }
  if (!isfinite(settings.batteryCalibrationOffsetV) || settings.batteryCalibrationOffsetV < -3.0F || settings.batteryCalibrationOffsetV > 3.0F) {
    settings.batteryCalibrationOffsetV = BATTERY_DEFAULT_CALIBRATION_OFFSET_V;
  }

  return settings;
}

void saveBatterySettings(bool enabled, BatteryProfile profile, float calibrationMultiplier, float calibrationOffsetV) {
  prefs.putUShort("version", STORAGE_VERSION);
  prefs.putBool("batEnabled", enabled);
  prefs.putUChar("batProfile", (uint8_t)profile);
  prefs.putFloat("batCalMul", calibrationMultiplier);
  prefs.putFloat("batCalOff", calibrationOffsetV);
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
