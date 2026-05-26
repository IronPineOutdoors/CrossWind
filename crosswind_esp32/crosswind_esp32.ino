#include <Arduino.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// CrossWind ESP32
// BLE-controlled pump/motor controller with manual, random, flush, and centering modes.
//
// BLE command format:
//   COMMAND=VALUE
// Examples:
//   MODE=RANDOM
//   SPEED=180
//   DIRECTION=REVERSE
//   START
//   STOP

// Pin assignment for the ESP32 motor driver and user interface.
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

// PWM channel configuration.
const int PWM_CHANNEL_LEFT = 0;
const int PWM_CHANNEL_RIGHT = 1;
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;

// Timing and motion constants.
const uint16_t DEBOUNCE_DELAY_MS = 50;
const uint8_t MIN_PWM = 50;
const uint8_t MAX_PWM = 255;
const uint8_t RAMP_STEP = 10;
const uint16_t RANDOM_ACTION_MIN_MS = 1200;
const uint16_t RANDOM_ACTION_MAX_MS = 3000;
const uint16_t FLUSH_PAUSE_MIN_MS = 1000;
const uint16_t FLUSH_PAUSE_MAX_MS = 5000;
const uint16_t FLUSH_RUN_MS = 1000;
const uint8_t FLUSH_SPEED = 255;
const uint8_t CENTERING_SPEED = 100;

// Operational modes for the wobbler.
enum Direction { FORWARD, REVERSE };
enum Mode { MANUAL, RANDOM, FLUSH, CENTERING };
enum FlushState { FLUSH_IDLE, FLUSH_WAIT, FLUSH_RUNNING, FLUSH_DONE };

// Current system state.
Mode currentMode = MANUAL;
Direction currentDirection = FORWARD;
FlushState flushState = FLUSH_IDLE;

// Sensor and input state.
bool leftLimitHit = false;
bool rightLimitHit = false;
bool startStopActive = false;
bool modeButtonEvent = false;

// Button debouncing state.
bool lastStartStopState = HIGH;
bool lastModeButtonState = HIGH;
unsigned long lastStartStopChange = 0;
unsigned long lastModeButtonChange = 0;

// Motor speed state.
uint8_t currentPwm = MIN_PWM;
unsigned long nextRandomChange = 0;
uint8_t randomPwm = MIN_PWM;
Direction randomDirection = FORWARD;

// Flush timing state.
unsigned long flushActionStart = 0;
unsigned long flushWaitDuration = 0;

// BLE speed override state: remote SPEED commands take priority for a short time.
bool speedOverrideActive = false;
unsigned long speedOverrideExpires = 0;

BLECharacteristic* commandCharacteristic = nullptr;
BLECharacteristic* statusCharacteristic = nullptr;
BLECharacteristic* responseCharacteristic = nullptr;

const char* BLE_DEVICE_NAME = "CrossWind-ESP32";
const char* BLE_SERVICE_UUID = "12345678-1234-1234-1234-1234567890ab";
const char* BLE_COMMAND_CHAR_UUID = "12345678-1234-1234-1234-1234567890ac";
const char* BLE_STATUS_CHAR_UUID = "12345678-1234-1234-1234-1234567890ad";
const char* BLE_RESPONSE_CHAR_UUID = "12345678-1234-1234-1234-1234567890ae";
const char* FIRMWARE_VERSION = "CrossWind ESP32 v1.1";

Preferences prefs;
const char* PREFS_NAMESPACE = "uniwobbler";
const int PREFS_MAGIC = 0xA5A5;
const char* PREFS_MAGIC_KEY = "magic";
const char* PREFS_CHECKSUM_KEY = "checksum";
const size_t MAX_BLE_COMMAND_LENGTH = 64;
bool bleClientConnected = false;

String lastResponseMessage = "READY";

// BLE command processing and status helpers.
bool processControlCommand(const String& cmd, const String& value);

// Mode helper to centralize mode switching behavior.
void setMode(Mode mode);

// Persistence helpers.
void loadPersistentState();
void persistState();
int calculatePrefsChecksum(int mode, int direction, int pwm);

// BLE response helpers.
String buildResponsePayload();
void sendCommandResponse(const String& status, const String& message);

// Safety helpers.
void emergencyStop(const String& reason);

// Normalize BLE command and parameter tokens for case-insensitive parsing.
String normalizeToken(const String& token);

// Helpers for BLE status text generation.
String modeToString(Mode mode);
String directionToString(Direction dir);
String buildStatusPayload();
String buildResponsePayload();
void updateBleStatus();
void sendCommandResponse(const String& status, const String& message);

void setSpeedOverride(uint8_t pwm);

// BLE callback that receives command writes from a connected client.
// The expected payload format is COMMAND=VALUE (or just COMMAND).
class CommandCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) override {
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
    bool handled = processControlCommand(cmd, val);
    if (!handled) {
      sendCommandResponse("ERROR", "Invalid command or value");
    } else {
      sendCommandResponse("OK", "Command accepted");
    }
    updateBleStatus();
  }
};

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    bleClientConnected = true;
    Serial.println("BLE client connected");
  }

  void onDisconnect(BLEServer* pServer) override {
    bleClientConnected = false;
    Serial.println("BLE client disconnected");
  }
};

// Convert a command token to uppercase and trim whitespace.
String normalizeToken(const String& token) {
  String normalized = token;
  normalized.trim();
  normalized.toUpperCase();
  return normalized;
}

// Convert a mode enum into a human-readable string.
String modeToString(Mode mode) {
  switch (mode) {
    case MANUAL:
      return "MANUAL";
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
  return dir == FORWARD ? "FORWARD" : "REVERSE";
}

// Set the current mode and reset mode-specific timers/state.
void setMode(Mode mode) {
  currentMode = mode;
  flushState = FLUSH_IDLE;
  nextRandomChange = 0;
  persistState();
}

// Build the current state payload that is sent over BLE.
String buildStatusPayload() {
  String payload = "fw=" + String(FIRMWARE_VERSION);
  payload += ";mode=" + modeToString(currentMode);
  payload += ";dir=" + directionToString(currentDirection);
  payload += ";running=" + String(startStopActive ? "1" : "0");
  payload += ";pwm=" + String(currentPwm);
  payload += ";speedOverride=" + String(speedOverrideActive ? "1" : "0");
  payload += ";leftLimit=" + String(leftLimitHit ? "1" : "0");
  payload += ";rightLimit=" + String(rightLimitHit ? "1" : "0");
  return payload;
}

// Send a status notification to connected BLE clients.
void updateBleStatus() {
  if (!statusCharacteristic || !bleClientConnected) {
    return;
  }
  String payload = buildStatusPayload();
  statusCharacteristic->setValue(payload.c_str());
  statusCharacteristic->notify();
}

String buildResponsePayload() {
  String payload = "status=";
  payload += lastResponseMessage.startsWith("ERROR") ? "ERROR" : "OK";
  payload += ";message=" + lastResponseMessage;
  return payload;
}

void sendCommandResponse(const String& status, const String& message) {
  lastResponseMessage = message;
  if (!responseCharacteristic) {
    return;
  }

  String payload = buildResponsePayload();
  responseCharacteristic->setValue(payload.c_str());
  responseCharacteristic->notify();
  Serial.printf("BLE CMD %s: %s\n", status.c_str(), message.c_str());
}

// Drive the motor in a given direction at the requested PWM level.
void setMotor(Direction dir, uint8_t pwm) {
  if (pwm < MIN_PWM) {
    pwm = 0;
  }

  if (dir == FORWARD) {
    digitalWrite(L_EN_PIN, HIGH);
    digitalWrite(R_EN_PIN, LOW);
    ledcWrite(PWM_CHANNEL_LEFT, pwm);
    ledcWrite(PWM_CHANNEL_RIGHT, 0);
  } else {
    digitalWrite(L_EN_PIN, LOW);
    digitalWrite(R_EN_PIN, HIGH);
    ledcWrite(PWM_CHANNEL_LEFT, 0);
    ledcWrite(PWM_CHANNEL_RIGHT, pwm);
  }
}

// Stop the motor immediately and disable both driver outputs.
void stopMotor() {
  digitalWrite(L_EN_PIN, LOW);
  digitalWrite(R_EN_PIN, LOW);
  ledcWrite(PWM_CHANNEL_LEFT, 0);
  ledcWrite(PWM_CHANNEL_RIGHT, 0);
}

// Apply a remote speed command and keep it active for 30 seconds.
// During that period, the physical speed pot is ignored.
void setSpeedOverride(uint8_t pwm) {
  currentPwm = pwm;
  speedOverrideActive = true;
  speedOverrideExpires = millis() + 30000;
  persistState();
}

// Force the system into a safe stopped state and report the reason.
void emergencyStop(const String& reason) {
  if (!startStopActive) {
    return;
  }

  startStopActive = false;
  stopMotor();
  digitalWrite(STATUS_LED_PIN, LOW);
  sendCommandResponse("ERROR", "EMERGENCY_STOP:" + reason);
}

int calculatePrefsChecksum(int mode, int direction, int pwm) {
  return (mode * 31) ^ (direction * 17) ^ pwm;
}

void loadPersistentState() {
  prefs.begin(PREFS_NAMESPACE, true);
  int storedMagic = prefs.getInt(PREFS_MAGIC_KEY, 0);
  int storedMode = prefs.getInt("mode", MANUAL);
  int storedDirection = prefs.getInt("direction", FORWARD);
  int storedPwm = prefs.getInt("pwm", MIN_PWM);
  int storedChecksum = prefs.getInt(PREFS_CHECKSUM_KEY, 0);
  prefs.end();

  if (storedMagic != PREFS_MAGIC || storedChecksum != calculatePrefsChecksum(storedMode, storedDirection, storedPwm)) {
    currentMode = MANUAL;
    currentDirection = FORWARD;
    currentPwm = MIN_PWM;
    persistState();
    return;
  }

  if (storedMode >= MANUAL && storedMode <= CENTERING) {
    currentMode = (Mode)storedMode;
  }
  if (storedDirection == FORWARD || storedDirection == REVERSE) {
    currentDirection = (Direction)storedDirection;
  }
  if (storedPwm >= MIN_PWM && storedPwm <= MAX_PWM) {
    currentPwm = storedPwm;
  }
}

void persistState() {
  prefs.begin(PREFS_NAMESPACE, false);
  prefs.putInt(PREFS_MAGIC_KEY, PREFS_MAGIC);
  prefs.putInt("mode", currentMode);
  prefs.putInt("direction", currentDirection);
  prefs.putInt("pwm", currentPwm);
  prefs.putInt(PREFS_CHECKSUM_KEY, calculatePrefsChecksum(currentMode, currentDirection, currentPwm));
  prefs.end();
}

// Handle a parsed BLE command and update the system state accordingly.
bool processControlCommand(const String& cmd, const String& value) {
  if (cmd == "START" || cmd == "RUN") {
    startStopActive = true;
    return true;
  }

  if (cmd == "STOP" || cmd == "PAUSE") {
    startStopActive = false;
    stopMotor();
    return true;
  }

  if (cmd == "STATUS") {
    updateBleStatus();
    return true;
  }

  if (cmd == "PING") {
    sendCommandResponse("OK", "PONG");
    return true;
  }

  if (cmd == "RESET") {
    currentMode = MANUAL;
    currentDirection = FORWARD;
    startStopActive = false;
    stopMotor();
    persistState();
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
    sendCommandResponse("OK", "HELP: START,STOP,STATUS,PING,RESET,MODE,DIRECTION,SPEED,SAVE,INFO");
    return true;
  }

  if (cmd == "MODE") {
    String modeValue = normalizeToken(value);
    if (modeValue == "MANUAL") {
      setMode(MANUAL);
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

  if (cmd == "DIRECTION") {
    String directionValue = normalizeToken(value);
    if (directionValue == "FORWARD") {
      currentDirection = FORWARD;
      persistState();
      return true;
    } else if (directionValue == "REVERSE") {
      currentDirection = REVERSE;
      persistState();
      return true;
    }
    return false;
  }

  if (cmd == "SPEED") {
    int speed = value.toInt();
    if (speed < MIN_PWM) {
      speed = MIN_PWM;
    }
    if (speed > MAX_PWM) {
      speed = MAX_PWM;
    }
    setSpeedOverride(speed);
    return true;
  }

  return false;
}

// Read digital inputs and update debounced button state, limit switches, and pot speed.
void readInputs() {
  bool startPressed = digitalRead(START_STOP_BUTTON_PIN) == LOW;
  bool modePressed = digitalRead(MODE_BUTTON_PIN) == LOW;
  unsigned long now = millis();

  if (startPressed != lastStartStopState) {
    lastStartStopChange = now;
  }
  if (now - lastStartStopChange >= DEBOUNCE_DELAY_MS) {
    startStopActive = startPressed;
  }

  if (modePressed != lastModeButtonState) {
    lastModeButtonChange = now;
  }
  if (now - lastModeButtonChange >= DEBOUNCE_DELAY_MS) {
    modeButtonEvent = (modePressed && !lastModeButtonState);
  }

  lastStartStopState = startPressed;
  lastModeButtonState = modePressed;

  leftLimitHit = digitalRead(LEFT_LIMIT_PIN) == LOW;
  rightLimitHit = digitalRead(RIGHT_LIMIT_PIN) == LOW;

  if (leftLimitHit && rightLimitHit && startStopActive) {
    emergencyStop("BOTH_LIMITS");
  }

  if (speedOverrideActive && (long)(millis() - speedOverrideExpires) >= 0) {
    speedOverrideActive = false;
  }

  if (!speedOverrideActive) {
    currentPwm = map(analogRead(SPEED_POT_PIN), 0, 4095, MIN_PWM, MAX_PWM);
  }
}

// Cycle through available modes when the mode button is pressed.
void handleModeChange() {
  if (!modeButtonEvent) {
    return;
  }

  currentMode = (Mode)((currentMode + 1) % 4);
  flushState = FLUSH_IDLE;
  nextRandomChange = 0;
  modeButtonEvent = false;
}

// Manual mode drives the motor continuously in the selected direction
// until a limit switch is reached or the user stops the system.
void handleManualMode() {
  if (!startStopActive) {
    stopMotor();
    return;
  }

  if (currentDirection == FORWARD) {
    if (rightLimitHit) {
      stopMotor();
      currentDirection = REVERSE;
      return;
    }
    setMotor(FORWARD, currentPwm);
  } else {
    if (leftLimitHit) {
      stopMotor();
      currentDirection = FORWARD;
      return;
    }
    setMotor(REVERSE, currentPwm);
  }
}

// Random mode picks a random direction and speed for short intervals.
void handleRandomMode() {
  if (!startStopActive) {
    stopMotor();
    return;
  }

  unsigned long now = millis();
  if (now >= nextRandomChange) {
    randomDirection = random(0, 2) == 0 ? FORWARD : REVERSE;
    randomPwm = random(MIN_PWM, currentPwm + 1);
    nextRandomChange = now + random(RANDOM_ACTION_MIN_MS, RANDOM_ACTION_MAX_MS);
  }

  if (randomDirection == FORWARD && rightLimitHit) {
    randomDirection = REVERSE;
  } else if (randomDirection == REVERSE && leftLimitHit) {
    randomDirection = FORWARD;
  }

  setMotor(randomDirection, randomPwm);
}

// Begin a flush sequence with a randomized wait before the flush action.
void startFlushSequence() {
  flushState = FLUSH_WAIT;
  flushWaitDuration = random(FLUSH_PAUSE_MIN_MS, FLUSH_PAUSE_MAX_MS);
  flushActionStart = millis();
  stopMotor();
}

void handleFlushMode() {
  if (!startStopActive) {
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
        randomDirection = random(0, 2) == 0 ? FORWARD : REVERSE;
        setMotor(randomDirection, FLUSH_SPEED);
      }
      break;
    case FLUSH_RUNNING:
      if (now - flushActionStart >= FLUSH_RUN_MS || (randomDirection == FORWARD && rightLimitHit) || (randomDirection == REVERSE && leftLimitHit)) {
        stopMotor();
        flushState = FLUSH_DONE;
        flushActionStart = now;
      }
      break;
    case FLUSH_DONE:
      if (now - flushActionStart >= 500) {
        flushState = FLUSH_IDLE;
      }
      break;
  }
}

// Centering mode moves toward the opposite limit and reverses when hit.
void handleCenteringMode() {
  if (!startStopActive) {
    stopMotor();
    return;
  }

  if (currentDirection == FORWARD) {
    if (rightLimitHit) {
      stopMotor();
      currentDirection = REVERSE;
      return;
    }
    setMotor(FORWARD, CENTERING_SPEED);
  } else {
    if (leftLimitHit) {
      stopMotor();
      currentDirection = FORWARD;
      return;
    }
    setMotor(REVERSE, CENTERING_SPEED);
  }
}

// Initialize the BLE service, characteristics, and advertising.
void startBle() {
  BLEDevice::init(BLE_DEVICE_NAME);
  BLEServer* bleServer = BLEDevice::createServer();
  BLEService* bleService = bleServer->createService(BLE_SERVICE_UUID);

  commandCharacteristic = bleService->createCharacteristic(
    BLE_COMMAND_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  commandCharacteristic->setCallbacks(new CommandCallback());

  bleServer->setCallbacks(new ServerCallbacks());

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

// Configure GPIO and PWM channels used by the motor controller.
void initPins() {
  pinMode(RPWM_PIN, OUTPUT);
  pinMode(LPWM_PIN, OUTPUT);
  pinMode(R_EN_PIN, OUTPUT);
  pinMode(L_EN_PIN, OUTPUT);
  pinMode(LEFT_LIMIT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_LIMIT_PIN, INPUT_PULLUP);
  pinMode(START_STOP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);

  ledcSetup(PWM_CHANNEL_LEFT, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_RIGHT, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LPWM_PIN, PWM_CHANNEL_LEFT);
  ledcAttachPin(RPWM_PIN, PWM_CHANNEL_RIGHT);

  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);
}

// Standard Arduino setup.
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Starting CrossWind ESP32...");

  initPins();
  loadPersistentState();
  randomSeed(esp_random());
  stopMotor();

  startBle();
}

// Main control loop: poll inputs, update mode logic, drive the motor, and refresh BLE status.
void loop() {
  readInputs();
  handleModeChange();

  switch (currentMode) {
    case MANUAL:
      handleManualMode();
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

  static unsigned long lastBleUpdate = 0;
  unsigned long now = millis();
  if (now - lastBleUpdate >= 1000) {
    updateBleStatus();
    lastBleUpdate = now;
  }

  digitalWrite(STATUS_LED_PIN, startStopActive ? HIGH : LOW);
}
