#include <Arduino.h>
#include <EEPROM.h>
#include <avr/wdt.h>

/*
  Iron Pine Outdoors Crosswind - Arduino AVR Phase 1 Prototype Firmware

  Phase 1 hardware:
    Arduino Uno/Nano, BTS7960 / IBT-2 motor driver, 12V Mitsubishi Outlander
    rear wiper motor, left/right roller limit switches, start/stop button,
    mode button, speed potentiometer, 12V battery, and 12V-to-5V buck converter.

  Wiring reference:
    Arduino D5  -> BTS7960 RPWM
    Arduino D6  -> BTS7960 LPWM
    Arduino D7  -> BTS7960 R_EN
    Arduino D8  -> BTS7960 L_EN
    Arduino GND -> BTS7960 GND and battery/buck ground

    Left limit switch  -> D9 and GND, using INPUT_PULLUP
    Right limit switch -> D10 and GND, using INPUT_PULLUP
      Preferred Phase 1 wiring is normally closed. With INPUT_PULLUP this means
      the input is LOW during normal travel and HIGH when the switch opens at
      the limit. Change LIMIT_ACTIVE_STATE if your wiring is different.

    Start/stop button -> D11 and GND, using INPUT_PULLUP
    Mode button       -> D12 and GND, using INPUT_PULLUP
    Speed pot wiper   -> A0, with pot ends to 5V and GND
    Status LED        -> D13 onboard LED

  Future path:
    The mode enum and mode handlers are kept aligned with the ESP32 firmware
    path: SWEEP, RANDOM, FLUSH, CENTERING. Phase 1 uses SWEEP as the reliable
    default. Future modes can grow without changing the basic motor/safety API.
*/

const char* FIRMWARE_VERSION = "Crosswind AVR Phase 1 v1.2";

// Pin assignment for Uno/Nano and BTS7960.
const uint8_t RPWM = 5;
const uint8_t LPWM = 6;
const uint8_t R_EN = 7;
const uint8_t L_EN = 8;
const uint8_t LEFT_LIMIT = 9;
const uint8_t RIGHT_LIMIT = 10;
const uint8_t START_STOP_BUTTON = 11;
const uint8_t MODE_BUTTON = 12;
const uint8_t SPEED_POT = A0;
const uint8_t STATUS_LED = 13;

// Configurable Phase 1 motion and safety constants.
const uint8_t MIN_PWM = 45;
const uint8_t MAX_PWM = 255;
const uint8_t DEFAULT_PWM = 120;
const uint8_t RAMP_STEP = 5;
const uint16_t RAMP_INTERVAL_MS = 20;
const uint16_t LIMIT_DWELL_MS = 1000;
const uint32_t MAX_TRAVEL_TIME_MS = 30000UL;
const uint8_t LIMIT_ACTIVE_STATE = HIGH;

// General AVR controller timing.
const uint16_t DEBOUNCE_DELAY_MS = 50;
const uint16_t DIRECTION_CHANGE_DEADTIME_MS = 50;
const uint32_t BUTTON_STUCK_TIMEOUT_MS = 10000UL;
const uint16_t SERIAL_STATUS_INTERVAL_MS = 1000;
const uint8_t EEPROM_MAGIC = 0xA5;
const uint8_t EEPROM_STATE_VERSION = 2;

enum Direction { DIR_RIGHT, DIR_LEFT };
enum Mode { SWEEP, RANDOM, FLUSH, CENTERING };
enum SweepState { SWEEP_IDLE, SWEEP_MOVING_RIGHT, SWEEP_DWELL_RIGHT, SWEEP_MOVING_LEFT, SWEEP_DWELL_LEFT };
enum FaultCode { FAULT_NONE = 0, FAULT_TRAVEL_TIMEOUT = 1, FAULT_BOTH_LIMITS = 2, FAULT_BUTTON_STUCK = 3, FAULT_STARTUP_BOTH_LIMITS = 4, FAULT_UNKNOWN = 255 };

struct __attribute__((packed)) PersistedState {
  uint8_t magic;
  uint8_t version;
  uint8_t mode;
  uint8_t lastFault;
  uint8_t lastSpeed;
  uint8_t checksum;
};

Mode currentMode = SWEEP;
Direction currentDirection = DIR_RIGHT;
SweepState sweepState = SWEEP_IDLE;
FaultCode lastFaultCode = FAULT_NONE;

bool leftLimitHit = false;
bool rightLimitHit = false;
bool startStopActive = false;
bool faultActive = false;
bool modeButtonEvent = false;

bool lastRawStartPressed = false;
bool lastRawModePressed = false;
bool debouncedStartPressed = false;
bool debouncedModePressed = false;
unsigned long lastStartChange = 0;
unsigned long lastModeChange = 0;
unsigned long startPressBegan = 0;

uint8_t currentPwm = DEFAULT_PWM;
uint8_t requestedMotorPwm = 0;
uint8_t appliedMotorPwm = 0;
Direction requestedDirection = DIR_RIGHT;
Direction activeDirection = DIR_RIGHT;
bool motorRunning = false;
bool directionChangePending = false;
unsigned long directionDeadtimeUntil = 0;
unsigned long lastRampUpdate = 0;

unsigned long motionStartTime = 0;
unsigned long dwellStartTime = 0;
unsigned long lastSerialStatus = 0;

uint8_t calculateEepromChecksum(const PersistedState& state);
void loadPersistentState();
void persistState();
void printStartupDiagnostics();
void printRuntimeStatus();
void readInputs();
void handleModeButton();
void handleSweepMode();
void handleRandomMode();
void handleFlushMode();
void handleCenteringMode();
void beginMotion(Direction dir);
void driveMotor(Direction dir, uint8_t pwm);
void stopMotor();
void updateMotorRamp();
void emergencyStop(FaultCode fault);
void clearFault();
bool bothLimitsActive();
bool travelTimedOut();
const char* modeToString(Mode mode);
const char* directionToString(Direction dir);
const char* faultToString(FaultCode fault);

uint8_t calculateEepromChecksum(const PersistedState& state) {
  return state.magic ^ state.version ^ state.mode ^ state.lastFault ^ state.lastSpeed ^ 0x5A;
}

const char* modeToString(Mode mode) {
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

const char* directionToString(Direction dir) {
  return dir == DIR_RIGHT ? "RIGHT" : "LEFT";
}

const char* faultToString(FaultCode fault) {
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

void loadPersistentState() {
  PersistedState state;
  EEPROM.get(0, state);

  bool valid = state.magic == EEPROM_MAGIC &&
               state.version == EEPROM_STATE_VERSION &&
               state.checksum == calculateEepromChecksum(state) &&
               state.mode <= CENTERING &&
               state.lastSpeed <= MAX_PWM;

  if (!valid) {
    currentMode = SWEEP;
    currentPwm = DEFAULT_PWM;
    lastFaultCode = FAULT_NONE;
    persistState();
    return;
  }

  currentMode = (Mode)state.mode;
  currentPwm = state.lastSpeed;
  lastFaultCode = (FaultCode)state.lastFault;
}

void persistState() {
  PersistedState state;
  state.magic = EEPROM_MAGIC;
  state.version = EEPROM_STATE_VERSION;
  state.mode = currentMode;
  state.lastFault = lastFaultCode;
  state.lastSpeed = currentPwm;
  state.checksum = calculateEepromChecksum(state);
  EEPROM.put(0, state);
}

bool bothLimitsActive() {
  return leftLimitHit && rightLimitHit;
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
    digitalWrite(L_EN, LOW);
    analogWrite(LPWM, 0);
    digitalWrite(R_EN, HIGH);
    analogWrite(RPWM, pwm);
  } else {
    digitalWrite(R_EN, LOW);
    analogWrite(RPWM, 0);
    digitalWrite(L_EN, HIGH);
    analogWrite(LPWM, pwm);
  }
}

void stopMotor() {
  requestedMotorPwm = 0;
  appliedMotorPwm = 0;
  motorRunning = false;
  directionChangePending = false;
  digitalWrite(R_EN, LOW);
  digitalWrite(L_EN, LOW);
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
}

void driveMotor(Direction dir, uint8_t pwm) {
  if (faultActive) {
    stopMotor();
    return;
  }

  uint8_t normalizedPwm = normalizeRunPwm(pwm);
  if (normalizedPwm == 0) {
    stopMotor();
    return;
  }

  requestedDirection = dir;
  requestedMotorPwm = normalizedPwm;
  currentDirection = dir;

  if (motorRunning && dir != activeDirection && !directionChangePending) {
    analogWrite(RPWM, 0);
    analogWrite(LPWM, 0);
    digitalWrite(R_EN, LOW);
    digitalWrite(L_EN, LOW);
    appliedMotorPwm = 0;
    motorRunning = false;
    directionChangePending = true;
    directionDeadtimeUntil = millis() + DIRECTION_CHANGE_DEADTIME_MS;
  }
}

void updateMotorRamp() {
  if (requestedMotorPwm == 0 || faultActive) {
    return;
  }

  unsigned long now = millis();
  if (directionChangePending) {
    if ((long)(now - directionDeadtimeUntil) < 0) {
      return;
    }
    directionChangePending = false;
  }

  if (now - lastRampUpdate >= RAMP_INTERVAL_MS || appliedMotorPwm == 0) {
    if (appliedMotorPwm < requestedMotorPwm) {
      uint16_t nextPwm = appliedMotorPwm + RAMP_STEP;
      appliedMotorPwm = nextPwm > requestedMotorPwm ? requestedMotorPwm : nextPwm;
    } else if (appliedMotorPwm > requestedMotorPwm) {
      appliedMotorPwm = appliedMotorPwm > RAMP_STEP ? appliedMotorPwm - RAMP_STEP : requestedMotorPwm;
      if (appliedMotorPwm < requestedMotorPwm) {
        appliedMotorPwm = requestedMotorPwm;
      }
    }
    lastRampUpdate = now;
  }

  activeDirection = requestedDirection;
  motorRunning = true;
  writeMotorOutputs(activeDirection, appliedMotorPwm);
}

void emergencyStop(FaultCode fault) {
  stopMotor();
  startStopActive = false;
  if (!faultActive) {
    faultActive = true;
    lastFaultCode = fault;
    persistState();
    Serial.print(F("FAULT: "));
    Serial.println(faultToString(lastFaultCode));
  }
}

void clearFault() {
  stopMotor();
  startStopActive = false;
  faultActive = false;
  lastFaultCode = FAULT_NONE;
  sweepState = SWEEP_IDLE;
  motionStartTime = 0;
  persistState();
  Serial.println(F("Fault cleared"));
}

void readInputs() {
  unsigned long now = millis();

  bool rawStartPressed = digitalRead(START_STOP_BUTTON) == LOW;
  bool rawModePressed = digitalRead(MODE_BUTTON) == LOW;

  if (rawStartPressed != lastRawStartPressed) {
    lastStartChange = now;
    lastRawStartPressed = rawStartPressed;
  }
  if (now - lastStartChange >= DEBOUNCE_DELAY_MS && rawStartPressed != debouncedStartPressed) {
    debouncedStartPressed = rawStartPressed;
    if (debouncedStartPressed) {
      startPressBegan = now;
      if (faultActive && !bothLimitsActive()) {
        clearFault();
      } else if (!faultActive) {
        startStopActive = !startStopActive;
        if (!startStopActive) {
          stopMotor();
          sweepState = SWEEP_IDLE;
        }
      }
    }
  }

  if (debouncedStartPressed && now - startPressBegan >= BUTTON_STUCK_TIMEOUT_MS) {
    emergencyStop(FAULT_BUTTON_STUCK);
  }

  if (rawModePressed != lastRawModePressed) {
    lastModeChange = now;
    lastRawModePressed = rawModePressed;
  }
  if (now - lastModeChange >= DEBOUNCE_DELAY_MS && rawModePressed != debouncedModePressed) {
    debouncedModePressed = rawModePressed;
    if (debouncedModePressed) {
      modeButtonEvent = true;
    }
  }

  leftLimitHit = digitalRead(LEFT_LIMIT) == LIMIT_ACTIVE_STATE;
  rightLimitHit = digitalRead(RIGHT_LIMIT) == LIMIT_ACTIVE_STATE;

  if (bothLimitsActive()) {
    emergencyStop(FAULT_BOTH_LIMITS);
  }

  currentPwm = map(analogRead(SPEED_POT), 0, 1023, 0, MAX_PWM);
}

void handleModeButton() {
  if (!modeButtonEvent) {
    return;
  }

  modeButtonEvent = false;
  if (startStopActive || faultActive) {
    return;
  }

  currentMode = (Mode)((currentMode + 1) % 4);
  sweepState = SWEEP_IDLE;
  stopMotor();
  persistState();

  Serial.print(F("Mode changed: "));
  Serial.println(modeToString(currentMode));
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
    motionStartTime = 0;
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
        currentDirection = DIR_LEFT;
        sweepState = SWEEP_DWELL_RIGHT;
        dwellStartTime = now;
        motionStartTime = 0;
        persistState();
      } else if (travelTimedOut()) {
        emergencyStop(FAULT_TRAVEL_TIMEOUT);
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
        currentDirection = DIR_RIGHT;
        sweepState = SWEEP_DWELL_LEFT;
        dwellStartTime = now;
        motionStartTime = 0;
        persistState();
      } else if (travelTimedOut()) {
        emergencyStop(FAULT_TRAVEL_TIMEOUT);
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
  // Future Crosswind product mode. Phase 1 keeps the same safe sweep behavior.
  handleSweepMode();
}

void handleFlushMode() {
  // Future Crosswind product mode. Phase 1 keeps the same safe sweep behavior.
  handleSweepMode();
}

void handleCenteringMode() {
  // Future Crosswind product mode. Phase 1 keeps the same safe sweep behavior.
  handleSweepMode();
}

void printStartupDiagnostics() {
  Serial.println();
  Serial.println(F("Crosswind AVR startup diagnostics"));
  Serial.print(F("  Firmware: "));
  Serial.println(FIRMWARE_VERSION);
  Serial.print(F("  Mode: "));
  Serial.println(modeToString(currentMode));
  Serial.print(F("  Left limit active: "));
  Serial.println(leftLimitHit ? F("YES") : F("NO"));
  Serial.print(F("  Right limit active: "));
  Serial.println(rightLimitHit ? F("YES") : F("NO"));
  Serial.print(F("  Pot raw: "));
  Serial.println(analogRead(SPEED_POT));
  Serial.print(F("  Current speed PWM: "));
  Serial.println(currentPwm);
  Serial.print(F("  Last EEPROM fault: "));
  Serial.println(faultToString(lastFaultCode));
  Serial.println(F("  Commands: start button toggles run/stop; mode button changes mode only while stopped."));
  Serial.println();
}

void printRuntimeStatus() {
  unsigned long now = millis();
  if (now - lastSerialStatus < SERIAL_STATUS_INTERVAL_MS) {
    return;
  }
  lastSerialStatus = now;

  Serial.print(F("mode="));
  Serial.print(modeToString(currentMode));
  Serial.print(F(" run="));
  Serial.print(startStopActive ? F("1") : F("0"));
  Serial.print(F(" fault="));
  if (faultActive) {
    Serial.print(faultToString(lastFaultCode));
  } else {
    Serial.print(F("NONE"));
  }
  Serial.print(F(" left="));
  Serial.print(leftLimitHit ? F("1") : F("0"));
  Serial.print(F(" right="));
  Serial.print(rightLimitHit ? F("1") : F("0"));
  Serial.print(F(" pot="));
  Serial.print(analogRead(SPEED_POT));
  Serial.print(F(" pwm="));
  Serial.print(currentPwm);
  Serial.print(F(" motorPwm="));
  Serial.print(appliedMotorPwm);
  Serial.print(F(" dir="));
  Serial.println(directionToString(currentDirection));
}

void setup() {
  Serial.begin(9600);
  delay(200);

  uint8_t resetFlags = MCUSR;
  MCUSR = 0;

  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);
  pinMode(LEFT_LIMIT, INPUT_PULLUP);
  pinMode(RIGHT_LIMIT, INPUT_PULLUP);
  pinMode(START_STOP_BUTTON, INPUT_PULLUP);
  pinMode(MODE_BUTTON, INPUT_PULLUP);
  pinMode(SPEED_POT, INPUT);
  pinMode(STATUS_LED, OUTPUT);

  stopMotor();
  loadPersistentState();

  leftLimitHit = digitalRead(LEFT_LIMIT) == LIMIT_ACTIVE_STATE;
  rightLimitHit = digitalRead(RIGHT_LIMIT) == LIMIT_ACTIVE_STATE;
  if (bothLimitsActive()) {
    faultActive = true;
    lastFaultCode = FAULT_STARTUP_BOTH_LIMITS;
    persistState();
  }

  if (resetFlags & _BV(WDRF)) {
    lastFaultCode = FAULT_UNKNOWN;
    persistState();
  }

  randomSeed(analogRead(A1) ^ analogRead(A2) ^ micros());
  printStartupDiagnostics();
  wdt_enable(WDTO_2S);
}

void loop() {
  readInputs();
  handleModeButton();

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

  updateMotorRamp();
  digitalWrite(STATUS_LED, startStopActive && !faultActive ? HIGH : (faultActive ? (millis() / 200) % 2 : LOW));
  printRuntimeStatus();
  wdt_reset();
}
