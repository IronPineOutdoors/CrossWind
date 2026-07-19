#include <Arduino.h>
#include <esp_arduino_version.h>
#include <esp_task_wdt.h>

#include "ble_control.h"
#include "battery_monitor.h"
#include "diagnostics.h"
#include "display.h"
#include "environment.h"
#include "inputs.h"
#include "limits.h"
#include "menu.h"
#include "modes.h"
#include "motor.h"
#include "motion.h"
#include "storage.h"
#include "status_led.h"
#include "trigger.h"

static ControllerState state = {
  SWEEP,
  DIR_RIGHT,
  false,
  false,
  FAULT_NONE,
  DEFAULT_PWM
};

static unsigned long lastBleStatus = 0;
static EnvironmentStatus lastUiEnvironmentStatus = ENV_STATUS_ERROR;
static bool lastUiBleConnected = false;
static unsigned long runStartedAt = 0;
static bool systemArmed = false;

static void setRunning(bool running);

static void latchFault(FaultCode fault) {
  stopMotor();
  cancelThrowerTrigger();
  systemArmed = false;
  setRunning(false);
  state.speed = 0;
  if (state.faultActive) {
    return;
  }
  state.faultActive = true;
  state.lastFault = fault;
  resetModeState();
  saveSettings(state);
  Serial.print("FAULT: ");
  Serial.println(faultToString(fault));
}

static void clearFault() {
  stopMotor();
  cancelThrowerTrigger();
  setRunning(false);
  state.faultActive = false;
  state.lastFault = FAULT_NONE;
  state.speed = readSpeedPwm();
  resetModeState();
  saveSettings(state);
}

static bool limitsReleased() {
  return !leftLimitActive() && !rightLimitActive();
}

static bool movementSafeToStart() {
  return !bothLimitsActive() && !emergencyStopActive() && batteryAllowsOperation();
}

static bool motorAllowed() {
  return state.running && !state.faultActive && !emergencyStopActive() && batteryAllowsOperation();
}

static void setRunning(bool running) {
  if (state.running == running) {
    return;
  }
  state.running = running;
  runStartedAt = running ? millis() : 0;
}

static int readMotorCurrentRaw() {
  if (MOTOR_CURRENT_SENSE_PIN < 0) {
    return -1;
  }
  return analogRead(MOTOR_CURRENT_SENSE_PIN);
}

static bool clearFaultIfSafe(const char* source) {
  if (!state.faultActive) {
    return false;
  }

  if (!limitsReleased() || emergencyStopActive() || !batteryAllowsOperation()) {
    Serial.print(source);
    Serial.println(" fault clear blocked: release safety inputs first");
    sendBleResponse("ERROR", "FAULT_CLEAR_BLOCKED_SAFETY_INPUT_ACTIVE");
    return true;
  }

  clearFault();
  Serial.print(source);
  Serial.println(" fault cleared");
  sendBleResponse("OK", "FAULT_CLEARED");
  return true;
}

static bool parseByteValue(const String& value, uint8_t& parsedValue) {
  if (value.length() == 0) {
    return false;
  }

  uint16_t result = 0;
  for (uint16_t i = 0; i < value.length(); i++) {
    char c = value.charAt(i);
    if (c < '0' || c > '9') {
      return false;
    }
    result = result * 10 + (uint16_t)(c - '0');
    if (result > 255) {
      return false;
    }
  }

  parsedValue = (uint8_t)result;
  return true;
}

static void setMode(Mode mode) {
  state.mode = mode;
  setRunning(false);
  cancelThrowerTrigger();
  resetModeState();
  saveSettings(state);
}

static void processUiIntent(const UiIntent& intent) {
  switch (intent.type) {
    case UI_INTENT_SET_MODE:
      if (state.running) showUiNotification("STOP TO CHANGE MODE");
      else { setMode((Mode)constrain(intent.value, (int)SWEEP, (int)CENTERING)); showUiNotification("MODE SAVED"); }
      break;
    case UI_INTENT_SET_SPEED:
      if (state.faultActive) showUiNotification("SPEED BLOCKED");
      else {
        setSpeedPwm((uint8_t)constrain(intent.value, 0, (int)MAX_PWM));
        state.speed = readSpeedPwm();
        if (intent.secondaryValue != 0) { saveSettings(state); showUiNotification("SPEED SAVED"); }
      }
      break;
    case UI_INTENT_START:
      if (state.faultActive) showUiNotification("FAULT ACTIVE");
      else if (!movementSafeToStart()) showUiNotification("START BLOCKED");
      else { setRunning(true); startMotion(state); showUiNotification("MOTION STARTED"); }
      break;
    case UI_INTENT_STOP:
      setRunning(false); stopMotor(); cancelThrowerTrigger(); resetModeState(); showUiNotification("MOTION STOPPED");
      break;
    case UI_INTENT_CENTER:
      if (requestCentering(state)) { runStartedAt = millis(); showUiNotification("CENTER STARTED"); }
      else showUiNotification("CENTER BLOCKED");
      break;
    case UI_INTENT_MOTION_CALIBRATE:
      if (requestCalibration(state)) { runStartedAt = millis(); showUiNotification("CAL STARTED"); }
      else showUiNotification("CAL BLOCKED");
      break;
    case UI_INTENT_CLEAR_FAULT:
      if (!clearFaultIfSafe("MENU")) showUiNotification("NO ACTIVE FAULT");
      else if (state.faultActive) showUiNotification("CLEAR BLOCKED");
      else showUiNotification("FAULT CLEARED");
      break;
    case UI_INTENT_BATTERY_CALIBRATE:
      if (state.running) showUiNotification("STOP REQUIRED");
      else if (calibrateBatteryToKnownVoltage((float)intent.value / 10.0F)) {
        saveBatterySettings(batteryMonitoringEnabled(), selectedBatteryProfile(), batteryCalibrationMultiplier(), batteryCalibrationOffsetV());
        showUiNotification("BATTERY CAL SAVED");
      } else showUiNotification("BAT CAL REJECTED");
      break;
    case UI_INTENT_RESET_BATTERY_CALIBRATION:
      if (state.running) showUiNotification("STOP REQUIRED");
      else {
        resetBatteryCalibration();
        saveBatterySettings(batteryMonitoringEnabled(), selectedBatteryProfile(), batteryCalibrationMultiplier(), batteryCalibrationOffsetV());
        showUiNotification("BAT CAL RESET");
      }
      break;
    case UI_INTENT_SAVE_SETTINGS:
      saveUiSettings((uint8_t)intent.value, (uint16_t)intent.secondaryValue);
      showUiNotification("SETTINGS SAVED");
      break;
    case UI_INTENT_RESTORE_DEFAULTS:
      if (state.running) showUiNotification("STOP REQUIRED");
      else {
        saveUiSettings(UI_DEFAULT_CONTRAST, UI_DEFAULT_STATUS_TIMEOUT_SECONDS);
        initMenu(UI_DEFAULT_CONTRAST, UI_DEFAULT_STATUS_TIMEOUT_SECONDS);
        setSpeedPwm(DEFAULT_PWM);
        state.speed = DEFAULT_PWM;
        setMode(SWEEP);
        saveSettings(state);
        showUiNotification("DEFAULTS RESTORED");
      }
      break;
    case UI_INTENT_NONE:
    default:
      break;
  }
}

static bool triggerAllowed() {
  if (state.faultActive) {
    Serial.println("Trigger ignored: fault active");
    sendBleResponse("ERROR", "TRIGGER_BLOCKED_FAULT");
    return false;
  }

  if (!limitsReleased() || emergencyStopActive() || !batteryAllowsOperation()) {
    Serial.println("Trigger ignored: limit active");
    sendBleResponse("ERROR", "TRIGGER_BLOCKED_LIMIT_ACTIVE");
    return false;
  }

  if (!state.running && !ALLOW_TRIGGER_WHEN_STOPPED) {
    Serial.println("Trigger ignored: controller stopped");
    sendBleResponse("ERROR", "TRIGGER_BLOCKED_STOPPED");
    return false;
  }

  return true;
}

static bool requestSafeTrigger() {
  if (!systemArmed) {
    Serial.println("FIRE BLOCKED - NOT ARMED");
    sendBleResponse("ERROR", "FIRE_BLOCKED_NOT_ARMED");
    return false;
  }

  if (!triggerAllowed()) {
    return false;
  }

  bool accepted = requestThrowerTrigger();
  sendBleResponse(accepted ? "OK" : "ERROR", accepted ? "TRIGGER_REQUESTED" : "TRIGGER_BLOCKED_INTERVAL");
  return accepted;
}

static bool handleBleCommand(const String& command, const String& value) {
  if (command == "START") {
    if (state.faultActive) {
      sendBleResponse("ERROR", "FAULT_ACTIVE_CLEAR_REQUIRED");
    } else if (!movementSafeToStart()) {
      sendBleResponse("ERROR", "START_BLOCKED_LIMIT_ACTIVE");
    } else {
      setRunning(true);
      startMotion(state);
      sendBleResponse("OK", "STARTED");
      showUiNotification("BLE STARTED");
    }
    return true;
  }

  if (command == "STOP") {
    setRunning(false);
    stopMotor();
    cancelThrowerTrigger();
    resetModeState();
    sendBleResponse("OK", "STOPPED");
    showUiNotification("BLE STOPPED");
    return true;
  }

  if (command == "CLEAR_FAULT") {
    clearFaultIfSafe("BLE");
    return true;
  }

  if (command == "STATUS") {
    sendBleResponse("OK", buildStatusPayload(state));
    updateBleStatus(state);
    return true;
  }

  if (command == "TRIGGER" || command == "FIRE" || command == "LAUNCH") {
    requestSafeTrigger();
    return true;
  }

  if (command == "MOTION_STATUS") {
    sendBleResponse("OK", buildStatusPayload(state));
    return true;
  }

  if (command == "CENTER") {
    bool accepted = requestCentering(state);
    if (accepted) runStartedAt = millis();
    sendBleResponse(accepted ? "OK" : "ERROR", accepted ? "CENTERING_STARTED" : "CENTERING_BLOCKED");
    return true;
  }

  if (command == "CALIBRATE") {
    bool accepted = requestCalibration(state);
    if (accepted) runStartedAt = millis();
    sendBleResponse(accepted ? "OK" : "ERROR", accepted ? "CALIBRATION_STARTED" : "CALIBRATION_BLOCKED");
    return true;
  }

  if (command == "BATTERY_STATUS") {
    sendBleResponse("OK", buildStatusPayload(state));
    return true;
  }

  if (command == "BATTERY_MONITOR") {
    if (state.running) {
      sendBleResponse("ERROR", "BATTERY_SETTING_BLOCKED_STOP_REQUIRED");
      return true;
    }
    String monitorValue = value;
    monitorValue.trim();
    monitorValue.toUpperCase();
    if (monitorValue == "ON" || monitorValue == "ENABLE") setBatteryMonitoringEnabled(true);
    else if (monitorValue == "OFF" || monitorValue == "DISABLE") setBatteryMonitoringEnabled(false);
    else return false;
    saveBatterySettings(batteryMonitoringEnabled(), selectedBatteryProfile(), batteryCalibrationMultiplier(), batteryCalibrationOffsetV());
    sendBleResponse("OK", batteryMonitoringEnabled() ? "BATTERY_MONITOR_ENABLED" : "BATTERY_MONITOR_DISABLED");
    return true;
  }

  if (command == "BATTERY_PROFILE") {
    if (state.running) {
      sendBleResponse("ERROR", "BATTERY_SETTING_BLOCKED_STOP_REQUIRED");
      return true;
    }
    String profileValue = value;
    profileValue.trim();
    profileValue.toUpperCase();
    BatteryProfile requestedProfile;
    if (profileValue == "TOOL_20V") requestedProfile = BATTERY_PROFILE_TOOL_20V;
    else if (profileValue == "BUS_12V") requestedProfile = BATTERY_PROFILE_12V_BUS;
    else if (profileValue == "CUSTOM") requestedProfile = BATTERY_PROFILE_CUSTOM;
    else return false;
    setBatteryProfile(requestedProfile);
    saveBatterySettings(batteryMonitoringEnabled(), selectedBatteryProfile(), batteryCalibrationMultiplier(), batteryCalibrationOffsetV());
    sendBleResponse("OK", "BATTERY_PROFILE_SET");
    return true;
  }

  if (command == "BATTERY_CALIBRATE") {
    if (state.running) {
      sendBleResponse("ERROR", "BATTERY_CALIBRATION_BLOCKED_STOP_REQUIRED");
      return true;
    }
    String calibrationValue = value;
    calibrationValue.trim();
    float knownVoltage = calibrationValue.toFloat();
    if (calibrationValue.length() == 0 || !calibrateBatteryToKnownVoltage(knownVoltage)) {
      sendBleResponse("ERROR", "BATTERY_CALIBRATION_REJECTED");
      return true;
    }
    saveBatterySettings(batteryMonitoringEnabled(), selectedBatteryProfile(), batteryCalibrationMultiplier(), batteryCalibrationOffsetV());
    sendBleResponse("OK", "BATTERY_CALIBRATED");
    return true;
  }

  if (command == "BATTERY_CALIBRATION_RESET") {
    if (state.running) {
      sendBleResponse("ERROR", "BATTERY_SETTING_BLOCKED_STOP_REQUIRED");
      return true;
    }
    resetBatteryCalibration();
    saveBatterySettings(batteryMonitoringEnabled(), selectedBatteryProfile(), batteryCalibrationMultiplier(), batteryCalibrationOffsetV());
    sendBleResponse("OK", "BATTERY_CALIBRATION_RESET");
    return true;
  }

  if (command == "SIM_BATTERY" && batterySimulationEnabled()) {
    String simulationValue = value;
    simulationValue.trim();
    simulationValue.toUpperCase();
    if (simulationValue == "INVALID") setSimulatedBatteryInvalid(true);
    else {
      float simulatedVoltage = simulationValue.toFloat();
      if (simulatedVoltage < 0.0F || simulatedVoltage > 35.0F) return false;
      setSimulatedBatteryVoltage(simulatedVoltage);
    }
    sendBleResponse("OK", "SIM_BATTERY_SET");
    return true;
  }

  if (command == "SIM_UI" && uiSimulationEnabled()) {
    String uiValue = value;
    uiValue.trim();
    uiValue.toUpperCase();
    if (uiValue == "NEXT") handleUiEncoder(1, state);
    else if (uiValue == "PREV") handleUiEncoder(-1, state);
    else if (uiValue == "PRESS") handleUiShortPress(state);
    else if (uiValue == "HOLD") handleUiLongPress(state);
    else return false;
    sendBleResponse("OK", "SIM_UI_EVENT");
    return true;
  }

  if (command == "SIM_LIMITS" && motionSimulationEnabled()) {
    String limitValue = value;
    limitValue.trim();
    limitValue.toUpperCase();
    if (limitValue == "AUTO") setSimulatedLimitMask(-1);
    else if (limitValue == "NONE") setSimulatedLimitMask(0);
    else if (limitValue == "LEFT") setSimulatedLimitMask(1);
    else if (limitValue == "RIGHT") setSimulatedLimitMask(2);
    else if (limitValue == "BOTH") setSimulatedLimitMask(3);
    else return false;
    sendBleResponse("OK", "SIM_LIMITS_SET");
    return true;
  }

  if (command == "SPEED") {
    uint8_t requestedSpeed = 0;
    if (!parseByteValue(value, requestedSpeed)) {
      return false;
    }
    if (state.faultActive) {
      sendBleResponse("ERROR", "SPEED_BLOCKED_FAULT");
      return true;
    }
    state.speed = requestedSpeed;
    setSpeedPwm(requestedSpeed);
    saveSettings(state);
    sendBleResponse("OK", "SPEED_SET");
    showUiNotification("BLE SPEED UPDATED");
    return true;
  }

  if (command == "MODE") {
    if (state.running) {
      sendBleResponse("ERROR", "MODE_BLOCKED_STOP_REQUIRED");
      return true;
    }

    String modeValue = value;
    modeValue.trim();
    modeValue.toUpperCase();

    if (modeValue == "SWEEP") {
      setMode(SWEEP);
    } else if (modeValue == "RANDOM") {
      setMode(RANDOM);
    } else if (modeValue == "FLUSH") {
      setMode(FLUSH);
    } else if (modeValue == "CENTERING") {
      setMode(CENTERING);
    } else {
      return false;
    }
    sendBleResponse("OK", "MODE_SET");
    showUiNotification("BLE MODE UPDATED");
    return true;
  }

  return false;
}

void setup() {
  Serial.begin(115200);
  delay(300);

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  esp_task_wdt_config_t wdtConfig = {
    .timeout_ms = WATCHDOG_TIMEOUT_SECONDS * 1000,
    .idle_core_mask = UINT32_MAX,
    .trigger_panic = true,
  };
  esp_task_wdt_init(&wdtConfig);
#else
  esp_task_wdt_init(WATCHDOG_TIMEOUT_SECONDS, true);
#endif
  esp_task_wdt_add(NULL);

  initTrigger();
  initStatusLed();
  systemArmed = false;

  beginMotor();
  beginLimits();
  beginInputs();
  if (MOTOR_CURRENT_SENSE_PIN >= 0) {
    pinMode(MOTOR_CURRENT_SENSE_PIN, INPUT);
  }
  initEnvironment();
  initDisplay();
  beginStorage();

  StoredSettings settings = loadSettings();
  state.mode = settings.mode;
  state.lastFault = settings.lastFault;
  state.speed = settings.lastSpeed;
  setSpeedPwm(settings.lastSpeed);
  initMotion(settings.fullTravelTimeMs);
  initBatteryMonitor(settings.batteryMonitorEnabled, settings.batteryProfile, settings.batteryCalibrationMultiplier, settings.batteryCalibrationOffsetV);
  initMenu(settings.displayContrast, settings.displayStatusTimeoutSeconds);

  updateInputs();
  updateLimits();
  if (bothLimitsActive()) {
    state.faultActive = ENABLE_LIMIT_FAULTS;
    state.lastFault = FAULT_STARTUP_BOTH_LIMITS;
    setRunning(false);
    state.speed = 0;
    Serial.println("FAULT: both limits active at startup");
    saveSettings(state);
  }

  printStartupDiagnostics(state);
  beginBle(handleBleCommand);
  updateBleStatus(state);
}

static void selectStatusLedMode() {
  EnvironmentStatus environmentStatus = getEnvironmentStatus();
  BatteryStatus battery = batteryStatus();
  if (state.faultActive) {
    showFaultStatus();
  } else if (battery == BATTERY_CRITICAL || battery == BATTERY_SENSOR_ERROR || battery == BATTERY_RECOVERING) {
    showWarningStatus();
  } else if (environmentStatus == ENV_STATUS_HOT || environmentStatus == ENV_STATUS_TEMP_FAULT || environmentStatus == ENV_STATUS_ERROR) {
    showWarningStatus();
  } else if (battery == BATTERY_LOW) {
    showWarningStatus();
  } else if (systemArmed) {
    showArmedStatus();
  } else {
    showReadyStatus();
  }
}

void loop() {
  updateInputs();
  int encoderDelta = consumeEncoderDelta();
  if (encoderDelta != 0) handleUiEncoder(encoderDelta, state);
  if (consumeMenuPressed()) handleUiShortPress(state);
  if (consumeMenuLongPressed()) handleUiLongPress(state);
  processUiIntent(consumeUiIntent());
  updateLimits();
  updateEnvironment();
  updateBatteryMonitor(state.running);

  if (emergencyStopActive()) {
    latchFault(FAULT_ESTOP);
  }

  if (ENABLE_TEMP_FAULTS && environmentTempFaultActive()) {
    latchFault(FAULT_TEMP);
  }

  if (consumeBatteryCriticalEvent() && ENABLE_BATTERY_CRITICAL_FAULT) {
    latchFault(FAULT_BATTERY_CRITICAL);
  }

  if (consumeBatterySensorErrorEvent()) {
    if (ENABLE_BATTERY_SENSOR_FAULT) {
      latchFault(FAULT_BATTERY_SENSOR);
    } else {
      setRunning(false);
      stopMotor();
      cancelThrowerTrigger();
      systemArmed = false;
      Serial.println("Battery sensor error: motion stopped");
    }
  }

  if (consumeBatteryLowEvent()) {
    Serial.println("WARNING: battery voltage low");
    showUiNotification("BATTERY LOW");
  }

  if (ENABLE_LIMIT_FAULTS && bothLimitsActive()) {
    latchFault(FAULT_BOTH_LIMITS);
  }

  if (ENABLE_MOTOR_SESSION_TIMEOUT && state.running && runStartedAt > 0 && millis() - runStartedAt >= MOTOR_SESSION_TIMEOUT_MS) {
    latchFault(FAULT_RUN_TIMEOUT);
  }

  if (ENABLE_MOTOR_OVERCURRENT_FAULT && readMotorCurrentRaw() >= MOTOR_OVERCURRENT_THRESHOLD_RAW) {
    latchFault(FAULT_OVERCURRENT);
  }

  if (systemArmed && (leftLimitActive() || rightLimitActive() || emergencyStopActive())) {
    systemArmed = false;
    cancelThrowerTrigger();
    Serial.println("ARM OFF: limit active");
  }

  if (consumeStartPressed()) {
    if (state.faultActive) {
      clearFaultIfSafe("START");
    } else if (!movementSafeToStart()) {
      Serial.println("START blocked: limit active");
      sendBleResponse("ERROR", "START_BLOCKED_LIMIT_ACTIVE");
    } else if (!state.faultActive) {
      setRunning(!state.running);
      if (state.running) {
        startMotion(state);
      }
      if (!state.running) {
        cancelThrowerTrigger();
        resetModeState();
      }
    }
  }

  if (consumeModePressed()) {
    if (state.faultActive) {
      clearFaultIfSafe("MODE");
    } else if (!state.running) {
      state.mode = (Mode)((state.mode + 1) % 4);
      resetModeState();
      saveSettings(state);
    }
  }

  if (consumeArmPressed()) {
    notifyUiActivity();
    if (state.faultActive) {
      clearFaultIfSafe("ARM");
    } else if (!movementSafeToStart()) {
      systemArmed = false;
      Serial.println("ARM blocked: limit active");
      sendBleResponse("ERROR", "ARM_BLOCKED_LIMIT_ACTIVE");
      showUiNotification("ARM BLOCKED");
    } else {
      systemArmed = !systemArmed;
      Serial.println(systemArmed ? "ARM ON" : "ARM OFF");
      showUiNotification(systemArmed ? "SYSTEM ARMED" : "SYSTEM SAFE");
    }
  }

  if (consumeFirePressed()) {
    notifyUiActivity();
    showUiNotification(requestSafeTrigger() ? "TRIGGER REQUESTED" : "TRIGGER BLOCKED");
  }

  if (!state.faultActive) {
    state.speed = readSpeedPwm();
  }

  bool wasFaulted = state.faultActive;
  if (motorAllowed()) {
    updateMode(state);
  } else {
    resetModeState();
  }
  if (!wasFaulted && state.faultActive) {
    cancelThrowerTrigger();
    systemArmed = false;
    state.speed = 0;
    saveSettings(state);
    Serial.print("FAULT: ");
    Serial.println(faultToString(state.lastFault));
  }

  FaultCode motionFault = consumeMotionFault();
  if (motionFault != FAULT_NONE) {
    latchFault(motionFault);
  }

  MotionEvent motionEvent = consumeMotionEvent();
  if (motionEvent == MOTION_EVENT_TRIGGER_REQUESTED) {
    if (ENABLE_AUTOMATIC_TRIGGER) requestSafeTrigger();
  } else if (motionEvent == MOTION_EVENT_CENTERED || motionEvent == MOTION_EVENT_CALIBRATED) {
    setRunning(false);
    stopMotor();
    showUiNotification(motionEvent == MOTION_EVENT_CENTERED ? "CENTER COMPLETE" : "CAL COMPLETE");
  }

  if (motionCalibrationNeedsSave()) {
    saveCalibration(calibratedFullTravelTimeMs());
    markMotionCalibrationSaved();
  }

  updateMotorRamp();
  updateTrigger();
  EnvironmentStatus currentEnvironmentStatus = getEnvironmentStatus();
  if (currentEnvironmentStatus != lastUiEnvironmentStatus) {
    if (currentEnvironmentStatus == ENV_STATUS_HOT) showUiNotification("TEMP WARNING");
    else if (currentEnvironmentStatus == ENV_STATUS_ERROR) showUiNotification("ENV SENSOR ERROR");
    lastUiEnvironmentStatus = currentEnvironmentStatus;
  }
  bool currentBleConnected = bleClientConnected();
  if (currentBleConnected != lastUiBleConnected) {
    showUiNotification(currentBleConnected ? "BLE CONNECTED" : "BLE DISCONNECTED");
    lastUiBleConnected = currentBleConnected;
  }
  updateMenu(state);
  selectStatusLedMode();
  updateStatusLed();
  updateDisplay(state, systemArmed);
  printRuntimeStatus(state);

  unsigned long now = millis();
  if (now - lastBleStatus >= BLE_STATUS_INTERVAL_MS) {
    updateBleStatus(state);
    lastBleStatus = now;
  }

  esp_task_wdt_reset();
}
