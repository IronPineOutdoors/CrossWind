#include <Arduino.h>
#include <esp_arduino_version.h>
#include <esp_task_wdt.h>

#include "ble_control.h"
#include "diagnostics.h"
#include "display.h"
#include "environment.h"
#include "inputs.h"
#include "limits.h"
#include "modes.h"
#include "motor.h"
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
static unsigned long runStartedAt = 0;
static bool systemArmed = false;
static bool setupDisplayMode = false;

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
  return (!ENABLE_LIMIT_FAULTS || limitsReleased()) && !emergencyStopActive();
}

static bool motorAllowed() {
  return state.running && !state.faultActive && movementSafeToStart();
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

  if (!limitsReleased() || emergencyStopActive()) {
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

static bool triggerAllowed() {
  if (state.faultActive) {
    Serial.println("Trigger ignored: fault active");
    sendBleResponse("ERROR", "TRIGGER_BLOCKED_FAULT");
    return false;
  }

  if (!movementSafeToStart()) {
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
      sendBleResponse("OK", "STARTED");
    }
    return true;
  }

  if (command == "STOP") {
    setRunning(false);
    stopMotor();
    cancelThrowerTrigger();
    resetModeState();
    sendBleResponse("OK", "STOPPED");
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
    saveSettings(state);
    sendBleResponse("OK", "SPEED_SET");
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
  setupDisplayMode = false;

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

  updateInputs();
  updateLimits();
  if (bothLimitsActive()) {
    state.faultActive = ENABLE_LIMIT_FAULTS;
    state.lastFault = FAULT_STARTUP_BOTH_LIMITS;
    setRunning(false);
    state.speed = 0;
    Serial.println("FAULT: both limits active at startup");
    saveSettings(state);
  } else if (ENABLE_LIMIT_FAULTS && !limitsReleased()) {
    state.faultActive = true;
    state.lastFault = FAULT_LIMIT;
    setRunning(false);
    state.speed = 0;
    Serial.println("FAULT: limit active at startup");
    saveSettings(state);
  }

  printStartupDiagnostics(state);
  beginBle(handleBleCommand);
  updateBleStatus(state);
}

static void selectStatusLedMode() {
  EnvironmentStatus environmentStatus = getEnvironmentStatus();
  if (state.faultActive) {
    showFaultStatus();
  } else if (environmentStatus == ENV_STATUS_HOT || environmentStatus == ENV_STATUS_TEMP_FAULT || environmentStatus == ENV_STATUS_ERROR) {
    showWarningStatus();
  } else if (systemArmed) {
    showArmedStatus();
  } else {
    showReadyStatus();
  }
}

void loop() {
  updateInputs();
  updateLimits();
  updateEnvironment();

  if (emergencyStopActive()) {
    latchFault(FAULT_ESTOP);
  }

  if (ENABLE_TEMP_FAULTS && environmentTempFaultActive()) {
    latchFault(FAULT_TEMP);
  }

  if (ENABLE_LIMIT_FAULTS && state.running && (leftLimitActive() || rightLimitActive())) {
    latchFault(bothLimitsActive() ? FAULT_BOTH_LIMITS : FAULT_LIMIT);
  }

  if (ENABLE_MOTOR_SESSION_TIMEOUT && state.running && runStartedAt > 0 && millis() - runStartedAt >= MOTOR_SESSION_TIMEOUT_MS) {
    latchFault(FAULT_RUN_TIMEOUT);
  }

  if (ENABLE_MOTOR_OVERCURRENT_FAULT && readMotorCurrentRaw() >= MOTOR_OVERCURRENT_THRESHOLD_RAW) {
    latchFault(FAULT_OVERCURRENT);
  }

  if (systemArmed && !movementSafeToStart()) {
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

  if (consumeMenuPressed()) {
    if (state.faultActive) {
      clearFaultIfSafe("ENCODER");
    } else {
      setupDisplayMode = !setupDisplayMode;
      Serial.println(setupDisplayMode ? "MENU SETUP" : "MENU MAIN");
    }
  }

  if (consumeArmPressed()) {
    if (state.faultActive) {
      clearFaultIfSafe("ARM");
    } else if (!movementSafeToStart()) {
      systemArmed = false;
      Serial.println("ARM blocked: limit active");
      sendBleResponse("ERROR", "ARM_BLOCKED_LIMIT_ACTIVE");
    } else {
      systemArmed = !systemArmed;
      Serial.println(systemArmed ? "ARM ON" : "ARM OFF");
    }
  }

  if (consumeFirePressed()) {
    requestSafeTrigger();
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

  updateMotorRamp();
  updateTrigger();
  selectStatusLedMode();
  updateStatusLed();
  updateDisplay(state, systemArmed, setupDisplayMode);
  printRuntimeStatus(state);

  unsigned long now = millis();
  if (now - lastBleStatus >= BLE_STATUS_INTERVAL_MS) {
    updateBleStatus(state);
    lastBleStatus = now;
  }

  esp_task_wdt_reset();
}
