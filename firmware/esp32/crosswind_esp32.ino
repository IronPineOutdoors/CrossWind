#include <Arduino.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_task_wdt.h>
#include <esp_arduino_version.h>

/*
  Iron Pine Outdoors Crosswind - ESP32 Phase 1 Prototype Firmware

  Phase 1 hardware:
    ESP32 dev board, BTS7960 / IBT-2 motor driver, 12V wiper motor,
    left and right roller limit switches, start/stop button, mode button,
    speed potentiometer or future rotary input, 12V battery, and 12V-to-5V buck.

  Wiring reference:
    ESP32 GPIO25 -> BTS7960 RPWM
    ESP32 GPIO26 -> BTS7960 LPWM
    ESP32 GPIO27 -> BTS7960 R_EN
    ESP32 GPIO14 -> BTS7960 L_EN
    ESP32 GND    -> BTS7960 GND and battery/buck ground

    Left limit switch  -> GPIO32 and GND, using INPUT_PULLUP
    Right limit switch -> GPIO33 and GND, using INPUT_PULLUP
      Preferred Phase 1 wiring is normally closed. With INPUT_PULLUP this means
      the input is LOW during normal travel and HIGH when the switch opens at
      the limit. Set LIMIT_ACTIVE_STATE below if your wiring is different.

    Start/stop button -> GPIO18 and GND, using INPUT_PULLUP
    Mode button       -> GPIO19 and GND, using INPUT_PULLUP
    Speed input       -> GPIO39 analog input from a 0-3.3V potentiometer wiper
    Status LED        -> GPIO2 onboard LED on many ESP32 dev boards

  Future expansion hooks reserved in this file:
    pitch actuator, battery voltage sensor, OLED display, rotary encoder,
    and additional Crosswind product modes.

  BLE commands:
    AUTH=<token>
    START
    STOP
    MODE=SWEEP
    MODE=RANDOM
    MODE=FLUSH
    MODE=CENTERING
    SPEED=0-255
    STATUS
    CLEAR_FAULT
*/

const char* FIRMWARE_VERSION = "Crosswind ESP32 Phase 1 v1.2";

// Pin assignment for the ESP32, BTS7960 motor driver, and user interface.
const int RPWM_PIN = 25;
const int LPWM_PIN = 26;
const int R_EN_PIN = 27;
const int L_EN_PIN = 14;
const int LEFT_LIMIT_PIN = 32;
const int RIGHT_LIMIT_PIN = 33;
const int START_STOP_BUTTON_PIN = 18;
const int MODE_BUTTON_PIN = 19;
const int SPEED_POT_PIN = 39;
const int STATUS_LED_PIN = 2;

// Future hardware hook placeholders.
const int PITCH_ACTUATOR_PIN = -1;
const int BATTERY_VOLTAGE_PIN = -1;
const int OLED_SDA_PIN = -1;
const int OLED_SCL_PIN = -1;
const int ROTARY_ENCODER_A_PIN = -1;
const int ROTARY_ENCODER_B_PIN = -1;

// PWM channel configuration.
const int PWM_CHANNEL_LEFT = 0;
const int PWM_CHANNEL_RIGHT = 1;
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;

// Phase 1 motion and safety constants.
const uint8_t MIN_PWM = 45;
const uint8_t MAX_PWM = 255;
const uint8_t DEFAULT_PWM = 120;
const uint8_t RAMP_STEP = 5;
const uint16_t RAMP_INTERVAL_MS = 20;
const uint16_t LIMIT_DWELL_MS = 1000;
const uint32_t MAX_TRAVEL_TIME_MS = 30000;
const int LIMIT_ACTIVE_STATE = HIGH;

// General timing constants.
const uint16_t DEBOUNCE_DELAY_MS = 50;
const uint16_t DIRECTION_CHANGE_DEADTIME_MS = 50;
const uint16_t RANDOM_ACTION_MIN_MS = 1200;
const uint16_t RANDOM_ACTION_MAX_MS = 3000;
const uint16_t FLUSH_PAUSE_MIN_MS = 1000;
const uint16_t FLUSH_PAUSE_MAX_MS = 5000;
const uint16_t FLUSH_RUN_MS = 1000;
const uint8_t WATCHDOG_TIMEOUT_SECONDS = 5;
const uint8_t FLUSH_SPEED = 180;
const uint8_t CENTERING_SPEED = 90;
const unsigned long BLE_COMMAND_RATE_MS = 200;
const unsigned long SPEED_OVERRIDE_MS = 30000;
const size_t MAX_BLE_COMMAND_LENGTH = 64;
const uint8_t MAX_AUTH_FAILURES = 3;
const unsigned long AUTH_LOCKOUT_MS = 60000;

enum Direction { DIR_RIGHT, DIR_LEFT };
enum Mode { SWEEP, RANDOM, FLUSH, CENTERING };
enum SweepState { SWEEP_IDLE, SWEEP_MOVING_RIGHT, SWEEP_DWELL_RIGHT, SWEEP_MOVING_LEFT, SWEEP_DWELL_LEFT };
enum FlushState { FLUSH_IDLE, FLUSH_WAIT, FLUSH_RUNNING, FLUSH_DONE };
enum FaultCode { FAULT_NONE = 0, FAULT_TRAVEL_TIMEOUT = 1, FAULT_BOTH_LIMITS = 2, FAULT_BUTTON_STUCK = 3, FAULT_STARTUP_BOTH_LIMITS = 4, FAULT_UNKNOWN = 255 };

Mode currentMode = SWEEP;
Direction currentDirection = DIR_RIGHT;
SweepState sweepState = SWEEP_IDLE;
FlushState flushState = FLUSH_IDLE;

bool leftLimitHit = false;
bool rightLimitHit = false;
bool startStopActive = false;
bool modeButtonEvent = false;
bool faultActive = false;

bool lastStartPressed = false;
bool lastModePressed = false;
bool debouncedStartPressed = false;
bool debouncedModePressed = false;
unsigned long lastStartStopChange = 0;
unsigned long lastModeButtonChange = 0;

uint8_t currentPwm = DEFAULT_PWM;
uint8_t targetMotorPwm = 0;
uint8_t appliedMotorPwm = 0;
Direction activeMotorDirection = DIR_RIGHT;
bool motorOutputActive = false;
unsigned long lastRampStepTime = 0;
unsigned long motionStartTime = 0;
unsigned long dwellStartTime = 0;

unsigned long nextRandomChange = 0;
uint8_t randomPwm = DEFAULT_PWM;
Direction randomDirection = DIR_RIGHT;

unsigned long flushActionStart = 0;
unsigned long flushWaitDuration = 0;
Direction flushDirection = DIR_RIGHT;

bool speedOverrideActive = false;
unsigned long speedOverrideExpires = 0;

BLECharacteristic* commandCharacteristic = nullptr;
BLECharacteristic* statusCharacteristic = nullptr;
BLECharacteristic* responseCharacteristic = nullptr;

const char* BLE_DEVICE_NAME = "Crosswind-ESP32";
const char* BLE_SERVICE_UUID = "12345678-1234-1234-1234-1234567890ab";
const char* BLE_COMMAND_CHAR_UUID = "12345678-1234-1234-1234-1234567890ac";
const char* BLE_STATUS_CHAR_UUID = "12345678-1234-1234-1234-1234567890ad";
const char* BLE_RESPONSE_CHAR_UUID = "12345678-1234-1234-1234-1234567890ae";

Preferences prefs;
const char* PREFS_NAMESPACE = "crosswind";
const int PREFS_MAGIC = 0xA5A5;
const int PREFS_VERSION = 2;
const char* PREFS_MAGIC_KEY = "magic";
const char* PREFS_VERSION_KEY = "version";
const char* PREFS_CHECKSUM_KEY = "checksum";
const char* PREFS_MODE_KEY = "mode";
const char* PREFS_DIRECTION_KEY = "direction";
const char* PREFS_LAST_SPEED_KEY = "lastSpeed";
const char* PREFS_LAST_FAULT_KEY = "lastFault";
const char* PREFS_FAULT_COUNT_KEY = "faultCount";
const char* PREFS_AUTH_KEY = "auth";
const char* PREFS_RESET_CAUSE_KEY = "resetCause";
const char* PREFS_AUTH_FAILURE_KEY = "authFails";
const char* PREFS_AUTH_LOCKOUT_KEY = "authLockout";

String bleAuthToken = "CROSSWIND";
bool bleClientConnected = false;
bool bleAuthorized = false;
int lastFaultCode = FAULT_NONE;
int faultCount = 0;
int lastResetCause = ESP_RST_POWERON;
int authFailureCount = 0;
unsigned long authLockoutUntil = 0;
unsigned long lastBleCommandMillis = 0;
String lastResponseMessage = "READY";

String normalizeToken(const String& token);
String modeToString(Mode mode);
String directionToString(Direction dir);
String faultToString(int fault);
String buildStatusPayload();
String buildResponsePayload();
bool processControlCommand(const String& cmd, const String& value);
void sendCommandResponse(const String& status, const String& message);
void updateBleStatus();
void setMode(Mode mode);
void loadPersistentState();
void persistState();
int calculatePrefsChecksum(int mode, int direction, int speed, int fault);
void clearFault();
void emergencyStop(FaultCode fault, const String& reason);
void driveMotor(Direction dir, uint8_t pwm);
void stopMotor();
void writeLeftPwm(uint8_t pwm);
void writeRightPwm(uint8_t pwm);

class CommandCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) override {
    unsigned long now = millis();
    if (now - lastBleCommandMillis < BLE_COMMAND_RATE_MS) {
      sendCommandResponse("ERROR", "RATE_LIMIT");
      return;
    }
    lastBleCommandMillis = now;

    String payload = pCharacteristic->getValue();
    payload.trim();
    if (payload.length() == 0) {
      return;
    }
    if (payload.length() > MAX_BLE_COMMAND_LENGTH) {
      sendCommandResponse("ERROR", "COMMAND_TOO_LONG");
      return;
    }

    int separator = payload.indexOf('=');
    if (separator < 0) {
      separator = payload.indexOf(' ');
    }

    String cmd = payload;
    String val = "";
    if (separator >= 0) {
      cmd = payload.substring(0, separator);
      val = payload.substring(separator + 1);
    }

    cmd = normalizeToken(cmd);
    val.trim();

    if (cmd.length() == 0) {
      return;
    }

    Serial.printf("BLE command received: %s\n", payload.c_str());
    String responseBefore = lastResponseMessage;
    bool handled = processControlCommand(cmd, val);
    if (!handled) {
      sendCommandResponse("ERROR", "INVALID_COMMAND_OR_VALUE");
    } else if (lastResponseMessage == responseBefore) {
      sendCommandResponse("OK", "COMMAND_ACCEPTED");
    }
    updateBleStatus();
  }
};

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    (void)pServer;
    bleClientConnected = true;
    Serial.println("BLE client connected");
  }

  void onDisconnect(BLEServer* pServer) override {
    (void)pServer;
    bleClientConnected = false;
    bleAuthorized = false;
    Serial.println("BLE client disconnected");
    BLEDevice::startAdvertising();
  }
};

String normalizeToken(const String& token) {
  String normalized = token;
  normalized.trim();
  normalized.toUpperCase();
  return normalized;
}

String modeToString(Mode mode) {
  switch (mode) {
    case SWEEP:
      return "SWEEP";
    case RANDOM:
      return "RANDOM";
    case FLUSH:
      return "FLUSH";
    case CENTERING:
      return "CENTERING";
    default:
      return "UNKNOWN";
  }
}

String directionToString(Direction dir) {
  return dir == DIR_RIGHT ? "RIGHT" : "LEFT";
}

String faultToString(int fault) {
  switch (fault) {
    case FAULT_NONE:
      return "NONE";
    case FAULT_TRAVEL_TIMEOUT:
      return "TRAVEL_TIMEOUT";
    case FAULT_BOTH_LIMITS:
      return "BOTH_LIMITS";
    case FAULT_BUTTON_STUCK:
      return "BUTTON_STUCK";
    case FAULT_STARTUP_BOTH_LIMITS:
      return "STARTUP_BOTH_LIMITS";
    default:
      return "UNKNOWN";
  }
}

String resetCauseToString(int cause) {
  switch (cause) {
    case ESP_RST_POWERON:
      return "POWERON";
    case ESP_RST_WDT:
      return "WATCHDOG";
    case ESP_RST_BROWNOUT:
      return "BROWNOUT";
    case ESP_RST_SW:
      return "SOFTWARE";
    case ESP_RST_SDIO:
      return "SDIO";
    case ESP_RST_DEEPSLEEP:
      return "DEEPSLEEP";
    case ESP_RST_EXT:
      return "EXTERNAL";
    default:
      return "UNKNOWN";
  }
}

bool isAuthLocked() {
  return authLockoutUntil > 0 && ((long)(authLockoutUntil - millis()) > 0);
}

void setMode(Mode mode) {
  currentMode = mode;
  sweepState = SWEEP_IDLE;
  flushState = FLUSH_IDLE;
  nextRandomChange = 0;
  stopMotor();
  persistState();
}

String buildStatusPayload() {
  String payload = "fw=" + String(FIRMWARE_VERSION);
  payload += ";mode=" + modeToString(currentMode);
  payload += ";dir=" + directionToString(currentDirection);
  payload += ";running=" + String(startStopActive ? "1" : "0");
  payload += ";faultActive=" + String(faultActive ? "1" : "0");
  payload += ";pwm=" + String(currentPwm);
  payload += ";motorPwm=" + String(appliedMotorPwm);
  payload += ";speedOverride=" + String(speedOverrideActive ? "1" : "0");
  payload += ";leftLimit=" + String(leftLimitHit ? "1" : "0");
  payload += ";rightLimit=" + String(rightLimitHit ? "1" : "0");
  payload += ";faultCount=" + String(faultCount);
  payload += ";lastFault=" + faultToString(lastFaultCode);
  payload += ";resetCause=" + resetCauseToString(lastResetCause);
  payload += ";authLocked=" + String(isAuthLocked() ? "1" : "0");
  payload += ";authFails=" + String(authFailureCount);
  return payload;
}

String buildResponsePayload() {
  String payload = "status=";
  payload += lastResponseMessage.startsWith("ERROR") ? "ERROR" : "OK";
  payload += ";message=" + lastResponseMessage;
  return payload;
}

void updateBleStatus() {
  if (!statusCharacteristic || !bleClientConnected) {
    return;
  }
  String payload = buildStatusPayload();
  statusCharacteristic->setValue(payload.c_str());
  statusCharacteristic->notify();
}

void sendCommandResponse(const String& status, const String& message) {
  lastResponseMessage = status == "ERROR" ? "ERROR:" + message : message;
  if (responseCharacteristic) {
    String payload = buildResponsePayload();
    responseCharacteristic->setValue(payload.c_str());
    responseCharacteristic->notify();
  }
  Serial.printf("BLE CMD %s: %s\n", status.c_str(), message.c_str());
}

int calculatePrefsChecksum(int mode, int direction, int speed, int fault) {
  return (mode * 31) ^ (direction * 17) ^ (speed * 13) ^ (fault * 7) ^ PREFS_VERSION;
}

void loadPersistentState() {
  prefs.begin(PREFS_NAMESPACE, true);
  int storedMagic = prefs.getInt(PREFS_MAGIC_KEY, 0);
  int storedVersion = prefs.getInt(PREFS_VERSION_KEY, 0);
  int storedMode = prefs.getInt(PREFS_MODE_KEY, SWEEP);
  int storedDirection = prefs.getInt(PREFS_DIRECTION_KEY, DIR_RIGHT);
  int storedSpeed = prefs.getInt(PREFS_LAST_SPEED_KEY, DEFAULT_PWM);
  int storedFault = prefs.getInt(PREFS_LAST_FAULT_KEY, FAULT_NONE);
  int storedChecksum = prefs.getInt(PREFS_CHECKSUM_KEY, 0);
  faultCount = prefs.getInt(PREFS_FAULT_COUNT_KEY, 0);
  lastResetCause = prefs.getInt(PREFS_RESET_CAUSE_KEY, ESP_RST_POWERON);
  authFailureCount = prefs.getInt(PREFS_AUTH_FAILURE_KEY, 0);
  authLockoutUntil = prefs.getULong(PREFS_AUTH_LOCKOUT_KEY, 0);
  bleAuthToken = prefs.getString(PREFS_AUTH_KEY, bleAuthToken.c_str());
  prefs.end();

  if (authLockoutUntil > 0 && ((long)(authLockoutUntil - millis()) <= 0)) {
    authLockoutUntil = 0;
    authFailureCount = 0;
  }

  bool valid = storedMagic == PREFS_MAGIC &&
               storedVersion == PREFS_VERSION &&
               storedChecksum == calculatePrefsChecksum(storedMode, storedDirection, storedSpeed, storedFault);

  if (!valid) {
    currentMode = SWEEP;
    currentDirection = DIR_RIGHT;
    currentPwm = DEFAULT_PWM;
    lastFaultCode = FAULT_NONE;
    faultActive = false;
    persistState();
    return;
  }

  if (storedMode >= SWEEP && storedMode <= CENTERING) {
    currentMode = (Mode)storedMode;
  }
  if (storedDirection == DIR_RIGHT || storedDirection == DIR_LEFT) {
    currentDirection = (Direction)storedDirection;
  }
  if (storedSpeed >= 0 && storedSpeed <= MAX_PWM) {
    currentPwm = storedSpeed;
  }
  lastFaultCode = storedFault;
}

void persistState() {
  prefs.begin(PREFS_NAMESPACE, false);
  prefs.putInt(PREFS_MAGIC_KEY, PREFS_MAGIC);
  prefs.putInt(PREFS_VERSION_KEY, PREFS_VERSION);
  prefs.putInt(PREFS_MODE_KEY, currentMode);
  prefs.putInt(PREFS_DIRECTION_KEY, currentDirection);
  prefs.putInt(PREFS_LAST_SPEED_KEY, currentPwm);
  prefs.putInt(PREFS_LAST_FAULT_KEY, lastFaultCode);
  prefs.putInt(PREFS_FAULT_COUNT_KEY, faultCount);
  prefs.putString(PREFS_AUTH_KEY, bleAuthToken.c_str());
  prefs.putInt(PREFS_RESET_CAUSE_KEY, lastResetCause);
  prefs.putInt(PREFS_AUTH_FAILURE_KEY, authFailureCount);
  prefs.putULong(PREFS_AUTH_LOCKOUT_KEY, authLockoutUntil);
  prefs.putInt(PREFS_CHECKSUM_KEY, calculatePrefsChecksum(currentMode, currentDirection, currentPwm, lastFaultCode));
  prefs.end();
}

void clearFault() {
  faultActive = false;
  startStopActive = false;
  lastFaultCode = FAULT_NONE;
  sweepState = SWEEP_IDLE;
  flushState = FLUSH_IDLE;
  stopMotor();
  persistState();
}

void emergencyStop(FaultCode fault, const String& reason) {
  stopMotor();
  startStopActive = false;
  if (faultActive) {
    return;
  }
  faultActive = true;
  lastFaultCode = fault;
  faultCount++;
  persistState();
  digitalWrite(STATUS_LED_PIN, LOW);
  Serial.printf("FAULT: %s\n", reason.c_str());
  sendCommandResponse("ERROR", reason);
}

uint8_t normalizeRunPwm(uint8_t pwm) {
  if (pwm == 0) {
    return 0;
  }
  if (pwm < MIN_PWM) {
    return MIN_PWM;
  }
  if (pwm > MAX_PWM) {
    return MAX_PWM;
  }
  return pwm;
}

void writeMotorOutputs(Direction dir, uint8_t pwm) {
  if (dir == DIR_RIGHT) {
    digitalWrite(L_EN_PIN, LOW);
    writeLeftPwm(0);
    digitalWrite(R_EN_PIN, HIGH);
    writeRightPwm(pwm);
  } else {
    digitalWrite(R_EN_PIN, LOW);
    writeRightPwm(0);
    digitalWrite(L_EN_PIN, HIGH);
    writeLeftPwm(pwm);
  }
}

void stopMotor() {
  targetMotorPwm = 0;
  appliedMotorPwm = 0;
  motorOutputActive = false;
  digitalWrite(R_EN_PIN, LOW);
  digitalWrite(L_EN_PIN, LOW);
  writeRightPwm(0);
  writeLeftPwm(0);
}

void driveMotor(Direction dir, uint8_t pwm) {
  if (faultActive) {
    stopMotor();
    return;
  }

  targetMotorPwm = normalizeRunPwm(pwm);
  if (targetMotorPwm == 0) {
    stopMotor();
    return;
  }

  if (motorOutputActive && dir != activeMotorDirection) {
    digitalWrite(R_EN_PIN, LOW);
    digitalWrite(L_EN_PIN, LOW);
    writeRightPwm(0);
    writeLeftPwm(0);
    appliedMotorPwm = 0;
    delay(DIRECTION_CHANGE_DEADTIME_MS);
  }

  activeMotorDirection = dir;
  motorOutputActive = true;
  currentDirection = dir;

  unsigned long now = millis();
  if (appliedMotorPwm == 0 || now - lastRampStepTime >= RAMP_INTERVAL_MS) {
    if (appliedMotorPwm < targetMotorPwm) {
      uint16_t nextPwm = appliedMotorPwm + RAMP_STEP;
      appliedMotorPwm = nextPwm > targetMotorPwm ? targetMotorPwm : nextPwm;
    } else if (appliedMotorPwm > targetMotorPwm) {
      appliedMotorPwm = appliedMotorPwm > RAMP_STEP ? appliedMotorPwm - RAMP_STEP : targetMotorPwm;
      if (appliedMotorPwm < targetMotorPwm) {
        appliedMotorPwm = targetMotorPwm;
      }
    }
    lastRampStepTime = now;
  }

  writeMotorOutputs(dir, appliedMotorPwm);
}

void writeLeftPwm(uint8_t pwm) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(LPWM_PIN, pwm);
#else
  ledcWrite(PWM_CHANNEL_LEFT, pwm);
#endif
}

void writeRightPwm(uint8_t pwm) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(RPWM_PIN, pwm);
#else
  ledcWrite(PWM_CHANNEL_RIGHT, pwm);
#endif
}

void setSpeedOverride(uint8_t pwm) {
  currentPwm = constrain(pwm, 0, MAX_PWM);
  speedOverrideActive = true;
  speedOverrideExpires = millis() + SPEED_OVERRIDE_MS;
  persistState();
}

bool processControlCommand(const String& cmd, const String& value) {
  if (cmd == "SETAUTH" || cmd == "SET_AUTH") {
    String newToken = normalizeToken(value);
    if (bleAuthorized || bleAuthToken == "CROSSWIND") {
      if (newToken.length() >= 6 && newToken.length() < 64) {
        bleAuthToken = newToken;
        persistState();
        sendCommandResponse("OK", "SETAUTH_OK");
        return true;
      }
      sendCommandResponse("ERROR", "SETAUTH_INVALID");
      return true;
    }
    sendCommandResponse("ERROR", "SETAUTH_NOT_AUTHORIZED");
    return true;
  }

  if (cmd == "AUTH") {
    if (isAuthLocked()) {
      sendCommandResponse("ERROR", "AUTH_LOCKED");
      return true;
    }
    if (normalizeToken(value) == String(bleAuthToken)) {
      bleAuthorized = true;
      authFailureCount = 0;
      authLockoutUntil = 0;
      persistState();
      sendCommandResponse("OK", "AUTHENTICATED");
      return true;
    }
    authFailureCount++;
    if (authFailureCount >= MAX_AUTH_FAILURES) {
      authLockoutUntil = millis() + AUTH_LOCKOUT_MS;
      persistState();
      sendCommandResponse("ERROR", "AUTH_LOCKED");
      return true;
    }
    persistState();
    sendCommandResponse("ERROR", "AUTH_FAILED");
    return true;
  }

  if (!bleAuthorized && cmd != "HELP" && cmd != "PING" && cmd != "STATUS" && cmd != "AUTH") {
    sendCommandResponse("ERROR", "NOT_AUTHENTICATED");
    return true;
  }

  if (cmd == "START" || cmd == "RUN") {
    if (faultActive) {
      sendCommandResponse("ERROR", "FAULT_ACTIVE_CLEAR_REQUIRED");
      return true;
    }
    startStopActive = true;
    return true;
  }

  if (cmd == "STOP" || cmd == "PAUSE") {
    startStopActive = false;
    stopMotor();
    return true;
  }

  if (cmd == "CLEAR_FAULT" || cmd == "RESET") {
    clearFault();
    return true;
  }

  if (cmd == "STATUS") {
    sendCommandResponse("OK", "STATUS:" + buildStatusPayload());
    updateBleStatus();
    return true;
  }

  if (cmd == "PING") {
    sendCommandResponse("OK", "PONG");
    return true;
  }

  if (cmd == "SAVE") {
    persistState();
    return true;
  }

  if (cmd == "INFO") {
    sendCommandResponse("OK", "INFO:" + buildStatusPayload());
    return true;
  }

  if (cmd == "HELP") {
    sendCommandResponse("OK", "HELP: AUTH,START,STOP,STATUS,PING,CLEAR_FAULT,MODE=SWEEP|RANDOM|FLUSH|CENTERING,SPEED=0-255,SAVE,INFO");
    return true;
  }

  if (cmd == "MODE") {
    String modeValue = normalizeToken(value);
    if (modeValue == "SWEEP") {
      setMode(SWEEP);
      return true;
    } else if (modeValue == "RANDOM" || modeValue == "AUTO") {
      setMode(RANDOM);
      return true;
    } else if (modeValue == "FLUSH") {
      setMode(FLUSH);
      return true;
    } else if (modeValue == "CENTERING" || modeValue == "CENTER") {
      setMode(CENTERING);
      return true;
    }
    return false;
  }

  if (cmd == "SPEED") {
    int speed = value.toInt();
    if (speed < 0 || speed > 255) {
      return false;
    }
    setSpeedOverride((uint8_t)speed);
    return true;
  }

  return false;
}

void readInputs() {
  unsigned long now = millis();
  bool rawStartPressed = digitalRead(START_STOP_BUTTON_PIN) == LOW;
  bool rawModePressed = digitalRead(MODE_BUTTON_PIN) == LOW;

  if (rawStartPressed != lastStartPressed) {
    lastStartStopChange = now;
    lastStartPressed = rawStartPressed;
  }
  if (now - lastStartStopChange >= DEBOUNCE_DELAY_MS && rawStartPressed != debouncedStartPressed) {
    debouncedStartPressed = rawStartPressed;
    if (debouncedStartPressed) {
      if (faultActive) {
        clearFault();
      } else {
        startStopActive = !startStopActive;
        if (!startStopActive) {
          stopMotor();
        }
      }
    }
  }

  if (rawModePressed != lastModePressed) {
    lastModeButtonChange = now;
    lastModePressed = rawModePressed;
  }
  if (now - lastModeButtonChange >= DEBOUNCE_DELAY_MS && rawModePressed != debouncedModePressed) {
    debouncedModePressed = rawModePressed;
    modeButtonEvent = debouncedModePressed;
  }

  leftLimitHit = digitalRead(LEFT_LIMIT_PIN) == LIMIT_ACTIVE_STATE;
  rightLimitHit = digitalRead(RIGHT_LIMIT_PIN) == LIMIT_ACTIVE_STATE;

  if (leftLimitHit && rightLimitHit) {
    emergencyStop(FAULT_BOTH_LIMITS, "BOTH_LIMITS");
  }

  if (speedOverrideActive && (long)(now - speedOverrideExpires) >= 0) {
    speedOverrideActive = false;
  }

  if (!speedOverrideActive) {
    currentPwm = map(analogRead(SPEED_POT_PIN), 0, 4095, 0, MAX_PWM);
  }
}

void handleModeChange() {
  if (!modeButtonEvent || startStopActive) {
    modeButtonEvent = false;
    return;
  }

  currentMode = (Mode)((currentMode + 1) % 4);
  sweepState = SWEEP_IDLE;
  flushState = FLUSH_IDLE;
  nextRandomChange = 0;
  stopMotor();
  persistState();
  modeButtonEvent = false;
}

void beginMotion(Direction dir) {
  currentDirection = dir;
  motionStartTime = millis();
  driveMotor(dir, currentPwm);
}

bool travelTimedOut() {
  return motionStartTime > 0 && millis() - motionStartTime >= MAX_TRAVEL_TIME_MS;
}

void handleSweepMode() {
  if (!startStopActive || faultActive) {
    stopMotor();
    sweepState = SWEEP_IDLE;
    return;
  }

  unsigned long now = millis();
  switch (sweepState) {
    case SWEEP_IDLE:
      if (rightLimitHit && !leftLimitHit) {
        sweepState = SWEEP_DWELL_RIGHT;
        dwellStartTime = now;
        stopMotor();
      } else if (leftLimitHit && !rightLimitHit) {
        sweepState = SWEEP_DWELL_LEFT;
        dwellStartTime = now;
        stopMotor();
      } else {
        sweepState = currentDirection == DIR_LEFT ? SWEEP_MOVING_LEFT : SWEEP_MOVING_RIGHT;
        beginMotion(currentDirection);
      }
      break;

    case SWEEP_MOVING_RIGHT:
      if (rightLimitHit) {
        stopMotor();
        sweepState = SWEEP_DWELL_RIGHT;
        dwellStartTime = now;
        motionStartTime = 0;
        persistState();
      } else if (travelTimedOut()) {
        emergencyStop(FAULT_TRAVEL_TIMEOUT, "RIGHT_TRAVEL_TIMEOUT");
      } else {
        driveMotor(DIR_RIGHT, currentPwm);
      }
      break;

    case SWEEP_DWELL_RIGHT:
      stopMotor();
      if (now - dwellStartTime >= LIMIT_DWELL_MS) {
        sweepState = SWEEP_MOVING_LEFT;
        beginMotion(DIR_LEFT);
      }
      break;

    case SWEEP_MOVING_LEFT:
      if (leftLimitHit) {
        stopMotor();
        sweepState = SWEEP_DWELL_LEFT;
        dwellStartTime = now;
        motionStartTime = 0;
        persistState();
      } else if (travelTimedOut()) {
        emergencyStop(FAULT_TRAVEL_TIMEOUT, "LEFT_TRAVEL_TIMEOUT");
      } else {
        driveMotor(DIR_LEFT, currentPwm);
      }
      break;

    case SWEEP_DWELL_LEFT:
      stopMotor();
      if (now - dwellStartTime >= LIMIT_DWELL_MS) {
        sweepState = SWEEP_MOVING_RIGHT;
        beginMotion(DIR_RIGHT);
      }
      break;
  }
}

void handleRandomMode() {
  if (!startStopActive || faultActive) {
    stopMotor();
    return;
  }

  unsigned long now = millis();
  if (now >= nextRandomChange) {
    randomDirection = random(0, 2) == 0 ? DIR_RIGHT : DIR_LEFT;
    uint8_t ceiling = currentPwm < MIN_PWM ? MIN_PWM : currentPwm;
    randomPwm = random(MIN_PWM, ceiling + 1);
    nextRandomChange = now + random(RANDOM_ACTION_MIN_MS, RANDOM_ACTION_MAX_MS);
    motionStartTime = now;
  }

  if (randomDirection == DIR_RIGHT && rightLimitHit) {
    randomDirection = DIR_LEFT;
    motionStartTime = now;
  } else if (randomDirection == DIR_LEFT && leftLimitHit) {
    randomDirection = DIR_RIGHT;
    motionStartTime = now;
  }

  if (travelTimedOut()) {
    emergencyStop(FAULT_TRAVEL_TIMEOUT, "RANDOM_TRAVEL_TIMEOUT");
    return;
  }

  driveMotor(randomDirection, randomPwm);
}

void startFlushSequence() {
  flushState = FLUSH_WAIT;
  flushWaitDuration = random(FLUSH_PAUSE_MIN_MS, FLUSH_PAUSE_MAX_MS);
  flushActionStart = millis();
  stopMotor();
}

void handleFlushMode() {
  if (!startStopActive || faultActive) {
    stopMotor();
    flushState = FLUSH_IDLE;
    return;
  }

  unsigned long now = millis();
  switch (flushState) {
    case FLUSH_IDLE:
      startFlushSequence();
      break;
    case FLUSH_WAIT:
      if (now - flushActionStart >= flushWaitDuration) {
        flushActionStart = now;
        flushState = FLUSH_RUNNING;
        flushDirection = random(0, 2) == 0 ? DIR_RIGHT : DIR_LEFT;
        motionStartTime = now;
      }
      break;
    case FLUSH_RUNNING:
      if (now - flushActionStart >= FLUSH_RUN_MS ||
          (flushDirection == DIR_RIGHT && rightLimitHit) ||
          (flushDirection == DIR_LEFT && leftLimitHit)) {
        stopMotor();
        flushState = FLUSH_DONE;
        flushActionStart = now;
        motionStartTime = 0;
      } else if (travelTimedOut()) {
        emergencyStop(FAULT_TRAVEL_TIMEOUT, "FLUSH_TRAVEL_TIMEOUT");
      } else {
        driveMotor(flushDirection, FLUSH_SPEED);
      }
      break;
    case FLUSH_DONE:
      if (now - flushActionStart >= LIMIT_DWELL_MS) {
        flushState = FLUSH_IDLE;
      }
      break;
  }
}

void handleCenteringMode() {
  if (!startStopActive || faultActive) {
    stopMotor();
    motionStartTime = 0;
    return;
  }

  if (motionStartTime == 0) {
    motionStartTime = millis();
  }

  if (rightLimitHit) {
    driveMotor(DIR_LEFT, CENTERING_SPEED);
  } else if (leftLimitHit) {
    driveMotor(DIR_RIGHT, CENTERING_SPEED);
  } else {
    driveMotor(currentDirection, CENTERING_SPEED);
  }

  if (travelTimedOut()) {
    emergencyStop(FAULT_TRAVEL_TIMEOUT, "CENTERING_TRAVEL_TIMEOUT");
  }
}

void startBle() {
  BLEDevice::init(BLE_DEVICE_NAME);
  BLEServer* bleServer = BLEDevice::createServer();
  bleServer->setCallbacks(new ServerCallbacks());

  BLEService* bleService = bleServer->createService(BLE_SERVICE_UUID);

  commandCharacteristic = bleService->createCharacteristic(
    BLE_COMMAND_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  commandCharacteristic->setCallbacks(new CommandCallback());

  statusCharacteristic = bleService->createCharacteristic(
    BLE_STATUS_CHAR_UUID,
    BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
  );
  statusCharacteristic->addDescriptor(new BLE2902());
  statusCharacteristic->setValue(buildStatusPayload().c_str());

  responseCharacteristic = bleService->createCharacteristic(
    BLE_RESPONSE_CHAR_UUID,
    BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
  );
  responseCharacteristic->addDescriptor(new BLE2902());
  responseCharacteristic->setValue(buildResponsePayload().c_str());

  bleService->start();
  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(BLE_SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->start();
  Serial.println("BLE advertising started");
}

void initPins() {
  pinMode(RPWM_PIN, OUTPUT);
  pinMode(LPWM_PIN, OUTPUT);
  pinMode(R_EN_PIN, OUTPUT);
  pinMode(L_EN_PIN, OUTPUT);
  pinMode(LEFT_LIMIT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_LIMIT_PIN, INPUT_PULLUP);
  pinMode(START_STOP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED_PIN, OUTPUT);

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(LPWM_PIN, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(RPWM_PIN, PWM_FREQ, PWM_RESOLUTION);
#else
  ledcSetup(PWM_CHANNEL_LEFT, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_RIGHT, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LPWM_PIN, PWM_CHANNEL_LEFT);
  ledcAttachPin(RPWM_PIN, PWM_CHANNEL_RIGHT);
#endif

  digitalWrite(STATUS_LED_PIN, LOW);
  stopMotor();
}

void printStartupDiagnostics() {
  int speedRaw = analogRead(SPEED_POT_PIN);
  Serial.println("Crosswind startup diagnostics");
  Serial.printf("  Firmware: %s\n", FIRMWARE_VERSION);
  Serial.printf("  Current mode: %s\n", modeToString(currentMode).c_str());
  Serial.printf("  Left limit active: %s\n", leftLimitHit ? "YES" : "NO");
  Serial.printf("  Right limit active: %s\n", rightLimitHit ? "YES" : "NO");
  Serial.printf("  Speed input raw: %d\n", speedRaw);
  Serial.printf("  Speed PWM: %u\n", currentPwm);
  Serial.printf("  Last stored fault: %s\n", faultToString(lastFaultCode).c_str());
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Starting Crosswind ESP32 Phase 1...");

  esp_task_wdt_config_t wdtConfig = {
    .timeout_ms = WATCHDOG_TIMEOUT_SECONDS * 1000,
    .idle_core_mask = UINT32_MAX,
    .trigger_panic = true,
  };
  esp_task_wdt_init(&wdtConfig);
  esp_task_wdt_add(NULL);

  initPins();
  loadPersistentState();

  lastResetCause = (int)esp_reset_reason();
  if (lastResetCause == ESP_RST_WDT) {
    lastFaultCode = FAULT_UNKNOWN;
    faultCount++;
    persistState();
    Serial.println("Watchdog reset detected.");
  }

  leftLimitHit = digitalRead(LEFT_LIMIT_PIN) == LIMIT_ACTIVE_STATE;
  rightLimitHit = digitalRead(RIGHT_LIMIT_PIN) == LIMIT_ACTIVE_STATE;
  if (leftLimitHit && rightLimitHit) {
    faultActive = true;
    lastFaultCode = FAULT_STARTUP_BOTH_LIMITS;
    faultCount++;
    persistState();
  }

  randomSeed(esp_random());
  stopMotor();
  printStartupDiagnostics();
  startBle();
}

void loop() {
  readInputs();
  handleModeChange();

  switch (currentMode) {
    case SWEEP:
      handleSweepMode();
      break;
    case RANDOM:
      handleRandomMode();
      break;
    case FLUSH:
      handleFlushMode();
      break;
    case CENTERING:
      handleCenteringMode();
      break;
  }

  esp_task_wdt_reset();

  static unsigned long lastBleUpdate = 0;
  unsigned long now = millis();
  if (now - lastBleUpdate >= 1000) {
    updateBleStatus();
    lastBleUpdate = now;
  }

  digitalWrite(STATUS_LED_PIN, startStopActive && !faultActive ? HIGH : LOW);
}
