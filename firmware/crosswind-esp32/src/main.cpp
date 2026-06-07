#include <Arduino.h>
#include <esp_task_wdt.h>

#include "ble_control.h"
#include "diagnostics.h"
#include "inputs.h"
#include "limits.h"
#include "modes.h"
#include "motor.h"
#include "storage.h"
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

static void latchFault(FaultCode fault) {
  stopMotor();
  cancelThrowerTrigger();
  state.running = false;
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
  state.running = false;
  state.faultActive = false;
  state.lastFault = FAULT_NONE;
  resetModeState();
  saveSettings(state);
}

static void setMode(Mode mode) {
  state.mode = mode;
  state.running = false;
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

  if (!state.running && !ALLOW_TRIGGER_WHEN_STOPPED) {
    Serial.println("Trigger ignored: controller stopped");
    sendBleResponse("ERROR", "TRIGGER_BLOCKED_STOPPED");
    return false;
  }

  return true;
}

static bool requestSafeTrigger() {
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
    } else {
      state.running = true;
      sendBleResponse("OK", "STARTED");
    }
    return true;
  }

  if (command == "STOP") {
    state.running = false;
    stopMotor();
    cancelThrowerTrigger();
    resetModeState();
    sendBleResponse("OK", "STOPPED");
    return true;
  }

  if (command == "CLEAR_FAULT") {
    clearFault();
    sendBleResponse("OK", "FAULT_CLEARED");
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
    int requestedSpeed = value.toInt();
    if (requestedSpeed < 0 || requestedSpeed > 255) {
      return false;
    }
    state.speed = (uint8_t)requestedSpeed;
    saveSettings(state);
    sendBleResponse("OK", "SPEED_SET");
    return true;
  }

  if (command == "MODE") {
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

  esp_task_wdt_config_t wdtConfig = {
    .timeout_ms = WATCHDOG_TIMEOUT_SECONDS * 1000,
    .idle_core_mask = UINT32_MAX,
    .trigger_panic = true,
  };
  esp_task_wdt_init(&wdtConfig);
  esp_task_wdt_add(NULL);

  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  beginMotor();
  initTrigger();
  beginLimits();
  beginInputs();
  beginStorage();

  StoredSettings settings = loadSettings();
  state.mode = settings.mode;
  state.lastFault = settings.lastFault;
  state.speed = settings.lastSpeed;

  updateInputs();
  updateLimits();
  if (bothLimitsActive()) {
    state.faultActive = true;
    state.lastFault = FAULT_STARTUP_BOTH_LIMITS;
    saveSettings(state);
  }

  printStartupDiagnostics(state);
  beginBle(handleBleCommand);
  updateBleStatus(state);
}

void loop() {
  updateInputs();
  updateLimits();

  if (consumeStartPressed()) {
    if (state.faultActive && !bothLimitsActive()) {
      clearFault();
    } else if (!state.faultActive) {
      state.running = !state.running;
      if (!state.running) {
        cancelThrowerTrigger();
        resetModeState();
      }
    }
  }

  if (consumeModePressed() && !state.running && !state.faultActive) {
    state.mode = (Mode)((state.mode + 1) % 4);
    resetModeState();
    saveSettings(state);
  }

  state.speed = readSpeedPwm();

  if (bothLimitsActive()) {
    latchFault(FAULT_BOTH_LIMITS);
  }

  bool wasFaulted = state.faultActive;
  updateMode(state);
  if (!wasFaulted && state.faultActive) {
    cancelThrowerTrigger();
    saveSettings(state);
    Serial.print("FAULT: ");
    Serial.println(faultToString(state.lastFault));
  }

  updateMotorRamp();
  updateTrigger();
  printRuntimeStatus(state);

  unsigned long now = millis();
  if (now - lastBleStatus >= BLE_STATUS_INTERVAL_MS) {
    updateBleStatus(state);
    lastBleStatus = now;
  }

  digitalWrite(STATUS_LED_PIN, state.running && !state.faultActive ? HIGH : (state.faultActive ? (millis() / 200) % 2 : LOW));
  esp_task_wdt_reset();
}
