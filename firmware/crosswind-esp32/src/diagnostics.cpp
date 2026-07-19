#include "diagnostics.h"

#include <esp_system.h>

#include "battery_monitor.h"
#include "environment.h"
#include "inputs.h"
#include "limits.h"
#include "menu.h"
#include "modes.h"
#include "motor.h"
#include "motion.h"
#include "trigger.h"

const char* modeToString(Mode mode) {
  switch (mode) {
    case SWEEP: return "SWEEP";
    case RANDOM: return "RANDOM";
    case FLUSH: return "FLUSH";
    case CENTERING: return "CENTERING";
    default: return "UNKNOWN";
  }
}

const char* directionToString(Direction dir) {
  return dir == DIR_RIGHT ? "RIGHT" : "LEFT";
}

const char* faultToString(FaultCode fault) {
  switch (fault) {
    case FAULT_NONE: return "NONE";
    case FAULT_TRAVEL_TIMEOUT: return "TRAVEL_TIMEOUT";
    case FAULT_BOTH_LIMITS: return "BOTH_LIMITS";
    case FAULT_BUTTON_STUCK: return "BUTTON_STUCK";
    case FAULT_STARTUP_BOTH_LIMITS: return "STARTUP_BOTH_LIMITS";
    case FAULT_TEMP: return "TEMP_FAULT";
    case FAULT_LIMIT: return "LIMIT";
    case FAULT_ESTOP: return "ESTOP";
    case FAULT_RUN_TIMEOUT: return "RUN_TIMEOUT";
    case FAULT_OVERCURRENT: return "OVERCURRENT";
    case FAULT_LIMIT_STUCK: return "LIMIT_STUCK";
    case FAULT_UNEXPECTED_LIMIT: return "UNEXPECTED_LIMIT";
    case FAULT_CALIBRATION_FAILED: return "CALIBRATION_FAILED";
    case FAULT_BATTERY_CRITICAL: return "BATTERY_CRITICAL";
    case FAULT_BATTERY_SENSOR: return "BATTERY_SENSOR";
    default: return "UNKNOWN";
  }
}

static const char* resetReasonToString(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON: return "POWERON";
    case ESP_RST_EXT: return "EXTERNAL";
    case ESP_RST_SW: return "SOFTWARE";
    case ESP_RST_PANIC: return "PANIC";
    case ESP_RST_INT_WDT: return "INT_WDT";
    case ESP_RST_TASK_WDT: return "TASK_WDT";
    case ESP_RST_WDT: return "WDT";
    case ESP_RST_DEEPSLEEP: return "DEEPSLEEP";
    case ESP_RST_BROWNOUT: return "BROWNOUT";
    case ESP_RST_SDIO: return "SDIO";
    default: return "UNKNOWN";
  }
}

static const char* speedInputToString() {
  switch (SPEED_INPUT_TYPE) {
    case SPEED_INPUT_ROTARY_ENCODER: return "ROTARY_ENCODER";
    case SPEED_INPUT_POTENTIOMETER: return "POTENTIOMETER";
    default: return "UNKNOWN";
  }
}

static const char* environmentSensorToString() {
  return "BME280";
}

void printStartupDiagnostics(const ControllerState& state) {
  Serial.println();
  Serial.println("Crosswind ESP32 startup diagnostics");
  Serial.print("  Firmware: ");
  Serial.println(FIRMWARE_VERSION);
  Serial.print("  Reset reason: ");
  Serial.println(resetReasonToString(esp_reset_reason()));
  Serial.print("  Mode: ");
  Serial.println(modeToString(state.mode));
  Serial.print("  Speed input: ");
  Serial.println(speedInputToString());
  Serial.print("  Environment sensor: ");
  Serial.println(environmentSensorToString());
  Serial.print("  BLE device: ");
  Serial.println(BLE_DEVICE_NAME);
  Serial.print("  Trigger pulse ms: ");
  Serial.println(TRIGGER_PULSE_MS);
  Serial.print("  Trigger min interval ms: ");
  Serial.println(MIN_TRIGGER_INTERVAL_MS);
  Serial.print("  BLE command min interval ms: ");
  Serial.println(BLE_COMMAND_MIN_INTERVAL_MS);
  Serial.print("  Motor session timeout enabled: ");
  Serial.println(ENABLE_MOTOR_SESSION_TIMEOUT ? "YES" : "NO");
  Serial.print("  Motor session timeout ms: ");
  Serial.println(MOTOR_SESSION_TIMEOUT_MS);
  Serial.print("  Motor current sense pin: ");
  Serial.println(MOTOR_CURRENT_SENSE_PIN);
  Serial.print("  Battery monitor: ");
  Serial.println(batteryMonitoringEnabled() ? "ENABLED" : "DISABLED");
  Serial.print("  Battery ADC pin: ");
  Serial.println(BATTERY_VOLTAGE_PIN);
  Serial.print("  Battery measurement point: ");
  Serial.println(batteryMeasurementPointToString());
  Serial.print("  Battery profile: ");
  Serial.println(batteryProfileToString(selectedBatteryProfile()));
  Serial.print("  Battery calibration multiplier: ");
  Serial.println(batteryCalibrationMultiplier(), 4);
  Serial.print("  Motor overcurrent fault enabled: ");
  Serial.println(ENABLE_MOTOR_OVERCURRENT_FAULT ? "YES" : "NO");
  Serial.print("  E-stop pin: ");
  Serial.println(ESTOP_PIN);
  Serial.print("  Limit active state: ");
  Serial.println(LIMIT_ACTIVE_STATE == LOW ? "LOW" : "HIGH");
  Serial.print("  Limit faults enabled: ");
  Serial.println(ENABLE_LIMIT_FAULTS ? "YES" : "NO");
  Serial.print("  Limit pins L/R: ");
  Serial.print(LEFT_LIMIT_PIN);
  Serial.print("/");
  Serial.println(RIGHT_LIMIT_PIN);
  Serial.print("  RGB pins R/G/B: ");
  Serial.print(RGB_RED_PIN);
  Serial.print("/");
  Serial.print(RGB_GREEN_PIN);
  Serial.print("/");
  Serial.println(RGB_BLUE_PIN);
  Serial.print("  Left limit: ");
  Serial.println(leftLimitActive() ? "ACTIVE" : "clear");
  Serial.print("  Left limit raw: ");
  Serial.println(leftLimitRawLevel());
  Serial.print("  Right limit: ");
  Serial.println(rightLimitActive() ? "ACTIVE" : "clear");
  Serial.print("  Right limit raw: ");
  Serial.println(rightLimitRawLevel());
  Serial.print("  Pot raw: ");
  Serial.println(readSpeedRaw());
  Serial.print("  Current speed PWM: ");
  Serial.println(state.speed);
  Serial.print("  Motor PWM: ");
  Serial.println(motorAppliedPwm());
  Serial.print("  Last stored fault: ");
  Serial.println(faultToString(state.lastFault));
  Serial.print("  Fault active: ");
  Serial.println(state.faultActive ? "YES" : "NO");
  Serial.print("  E-stop active: ");
  Serial.println(emergencyStopActive() ? "YES" : "NO");
  Serial.print("  Thrower trigger enabled: ");
  Serial.println(ENABLE_THROWER_TRIGGER ? "YES" : "NO");
  Serial.print("  Trigger active: ");
  Serial.println(isTriggerActive() ? "YES" : "NO");
  Serial.println();
}

void printRuntimeStatus(const ControllerState& state) {
  static unsigned long lastPrint = 0;
  unsigned long now = millis();
  if (now - lastPrint < SERIAL_STATUS_INTERVAL_MS) {
    return;
  }
  lastPrint = now;

  Serial.println(buildStatusPayload(state));
}

String buildStatusPayload(const ControllerState& state) {
  String payload = "fw=" + String(FIRMWARE_VERSION);
  payload += ";mode=" + String(modeToString(state.mode));
  payload += ";running=" + String(state.running ? "1" : "0");
  payload += ";fault=" + String(state.faultActive ? faultToString(state.lastFault) : "NONE");
  payload += ";dir=" + String(directionToString(state.direction));
  payload += ";sweepState=" + String(sweepStateToString());
  payload += ";motionState=" + String(motionPhaseToString());
  payload += ";centered=" + String(motionIsApproximatelyCentered() ? "1" : "0");
  payload += ";travelMs=" + String(calibratedFullTravelTimeMs());
  payload += ";speed=" + String(state.speed);
  payload += ";motorPwm=" + String(motorAppliedPwm());
  payload += ";leftLimit=" + String(leftLimitActive() ? "1" : "0");
  payload += ";rightLimit=" + String(rightLimitActive() ? "1" : "0");
  payload += ";leftLimitRaw=" + String(leftLimitRawLevel());
  payload += ";rightLimitRaw=" + String(rightLimitRawLevel());
  payload += ";potRaw=" + String(readSpeedRaw());
  payload += ";estop=" + String(emergencyStopActive() ? "1" : "0");
  payload += ";triggerEnabled=" + String(ENABLE_THROWER_TRIGGER ? "1" : "0");
  payload += ";triggerActive=" + String(isTriggerActive() ? "1" : "0");
  payload += ";lastTriggerMs=" + String(getLastTriggerTime());
  payload += ";envStatus=" + String(environmentStatusToString(getEnvironmentStatus()));
  payload += ";tempF=" + String(environmentDataValid() ? getTemperatureF() : NAN, 1);
  payload += ";humidity=" + String(environmentDataValid() ? getHumidity() : NAN, 1);
  payload += ";pressureHpa=" + String(environmentDataValid() ? getPressureHpa() : NAN, 1);
  const BatteryDiagnostics& battery = batteryDiagnostics();
  payload += ";batteryEnabled=" + String(batteryMonitoringEnabled() ? "1" : "0");
  payload += ";batteryPoint=" + String(batteryMeasurementPointToString());
  payload += ";batteryProfile=" + String(batteryProfileToString(selectedBatteryProfile()));
  payload += ";batteryStatus=" + String(batteryStatusToString(batteryStatus()));
  payload += ";batteryValid=" + String(batterySensorValid() ? "1" : "0");
  payload += ";batteryV=" + String(batterySensorValid() ? batteryVoltage() : NAN, 2);
  payload += ";batteryPct=" + String(batteryPercentageValid() ? batteryApproximatePercent() : -1);
  payload += ";batteryRaw=" + String(battery.latestRawAdc);
  payload += ";batteryFiltered=" + String(battery.filteredAdc, 1);
  payload += ";batteryMinV=" + String(battery.minimumVoltage, 2);
  payload += ";batteryMaxV=" + String(battery.maximumVoltage, 2);
  payload += ";batteryMotorMinV=" + String(battery.lowestMotorRunningVoltage, 2);
  payload += ";batteryLowCount=" + String(battery.lowWarningCount);
  payload += ";batteryCriticalCount=" + String(battery.criticalEventCount);
  payload += ";batteryErrorCount=" + String(battery.sensorErrorCount);
  payload += ";batteryCal=" + String(batteryCalibrationMultiplier(), 4);
  bool batteryCalibrated = fabsf(batteryCalibrationMultiplier() - BATTERY_DEFAULT_CALIBRATION_MULTIPLIER) > 0.0001F
    || fabsf(batteryCalibrationOffsetV() - BATTERY_DEFAULT_CALIBRATION_OFFSET_V) > 0.0001F;
  payload += ";batteryCalibrated=" + String(batteryCalibrated ? "1" : "0");
  payload += ";uiState=" + String(uiStateToString());
  payload += ";uiPage=" + String(uiPageToString());
  payload += ";uiContrast=" + String(uiDisplayContrast());
  payload += ";uiStatusTimeout=" + String(uiStatusTimeoutSeconds());
  payload += ";uiDirty=" + String(uiSettingsDirty() ? "1" : "0");
  const MotionDiagnostics& motion = motionDiagnostics();
  payload += ";sweeps=" + String(motion.completedSweeps);
  payload += ";reversals=" + String(motion.directionReversals);
  payload += ";randomMoves=" + String(motion.randomMovements);
  payload += ";centerAttempts=" + String(motion.centeringAttempts);
  payload += ";centerSuccess=" + String(motion.centeringSuccesses);
  payload += ";calAttempts=" + String(motion.calibrationAttempts);
  payload += ";calSuccess=" + String(motion.calibrationSuccesses);
  payload += ";calFailures=" + String(motion.calibrationFailures);
  payload += ";motionTimeouts=" + String(motion.motionTimeouts);
  payload += ";stuckLimits=" + String(motion.stuckLimitFaults);
  return payload;
}
