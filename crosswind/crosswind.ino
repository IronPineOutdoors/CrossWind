#include <Arduino.h>

// CrossWind
// Controls a bidirectional pump/motor with manual, random, flush, and centering modes.
// This version adds non-blocking flush sequencing, safety checks, and optional serial debug.
//#define DEBUG_SERIAL

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

const uint16_t DEBOUNCE_DELAY_MS = 50;
const uint8_t MIN_PWM = 50;
const uint8_t MAX_PWM = 255;
const uint8_t RAMP_STEP = 10;
const uint16_t RAMP_DELAY_MS = 15;
const uint16_t PAUSE_MIN_MS = 1000;
const uint16_t PAUSE_MAX_MS = 5000;
const uint16_t RANDOM_ACTION_MIN_MS = 1200;
const uint16_t RANDOM_ACTION_MAX_MS = 3000;
const uint16_t FLUSH_WAIT_MIN_MS = 1000;
const uint16_t FLUSH_WAIT_MAX_MS = 5000;
const uint16_t FLUSH_RUN_MS = 1000;
const uint16_t FLUSH_DONE_MS = 500;
const uint8_t FLUSH_SPEED = 255;
const uint8_t CENTERING_SPEED = 100;

enum Direction { FORWARD, REVERSE };
enum Mode { MANUAL, RANDOM, FLUSH, CENTERING };
enum FlushState { FLUSH_IDLE, FLUSH_WAIT, FLUSH_RUNNING, FLUSH_DONE };

Mode currentMode = MANUAL;
Direction currentDirection = FORWARD;

bool leftLimitHit = false;
bool rightLimitHit = false;

uint8_t currentPwm = MIN_PWM;

bool startStopActive = false;
bool modeButtonEvent = false;

bool lastStartStopState = HIGH;
bool lastModeButtonState = HIGH;
unsigned long lastStartStopChange = 0;
unsigned long lastModeButtonChange = 0;

FlushState flushState = FLUSH_IDLE;
unsigned long flushStartTime = 0;
unsigned long flushWaitDuration = 0;
Direction flushDirection = FORWARD;

#ifdef DEBUG_SERIAL
void debugLog(const char* message) {
  Serial.println(message);
}
#else
void debugLog(const char* message) {
  (void)message;
}
#endif

void setMotor(Direction dir, uint8_t pwm) {
  if (dir == FORWARD) {
    digitalWrite(L_EN, HIGH);
    analogWrite(LPWM, pwm);
    digitalWrite(R_EN, LOW);
    analogWrite(RPWM, 0);
  } else {
    digitalWrite(L_EN, LOW);
    analogWrite(LPWM, 0);
    digitalWrite(R_EN, HIGH);
    analogWrite(RPWM, pwm);
  }
}

void stopMotor() {
  digitalWrite(L_EN, LOW);
  analogWrite(LPWM, 0);
  digitalWrite(R_EN, LOW);
  analogWrite(RPWM, 0);
}

void rampMotor(Direction dir, uint8_t targetPwm) {
  if (targetPwm < MIN_PWM) {
    targetPwm = MIN_PWM;
  }

  for (uint8_t pwm = MIN_PWM; pwm < targetPwm; pwm += RAMP_STEP) {
    setMotor(dir, pwm);
    delay(RAMP_DELAY_MS);
  }

  setMotor(dir, targetPwm);
}

bool bothLimitsHit() {
  return leftLimitHit && rightLimitHit;
}

void emergencyStop(const char* reason) {
  stopMotor();
  startStopActive = false;
  debugLog(reason);
}

void readButtons() {
  bool rawStart = digitalRead(START_STOP_BUTTON);
  bool rawMode = digitalRead(MODE_BUTTON);
  unsigned long now = millis();

  if (rawStart != lastStartStopState) {
    lastStartStopChange = now;
  }
  if (now - lastStartStopChange >= DEBOUNCE_DELAY_MS) {
    startStopActive = (rawStart == LOW);
  }

  if (rawMode != lastModeButtonState) {
    lastModeButtonChange = now;
  }
  if (now - lastModeButtonChange >= DEBOUNCE_DELAY_MS) {
    modeButtonEvent = (rawMode == LOW && lastModeButtonState == HIGH);
  }

  lastStartStopState = rawStart;
  lastModeButtonState = rawMode;
}

void readLimits() {
  leftLimitHit = digitalRead(LEFT_LIMIT) == LOW;
  rightLimitHit = digitalRead(RIGHT_LIMIT) == LOW;

  if (bothLimitsHit() && startStopActive) {
    emergencyStop("BOTH_LIMITS");
  }
}

void readSpeedPot() {
  currentPwm = map(analogRead(SPEED_POT), 0, 1023, MIN_PWM, MAX_PWM);
}

void handleModeChange() {
  if (!modeButtonEvent) {
    return;
  }

  currentMode = (Mode)((currentMode + 1) % 4);
  flushState = FLUSH_IDLE;
  flushStartTime = 0;
  modeButtonEvent = false;
}

void manualMode() {
  if (!startStopActive) {
    stopMotor();
    return;
  }

  if (bothLimitsHit()) {
    emergencyStop("BOTH_LIMITS");
    return;
  }

  if (currentDirection == FORWARD) {
    if (rightLimitHit) {
      stopMotor();
      currentDirection = REVERSE;
    } else {
      setMotor(FORWARD, currentPwm);
    }
  } else {
    if (leftLimitHit) {
      stopMotor();
      currentDirection = FORWARD;
    } else {
      setMotor(REVERSE, currentPwm);
    }
  }
}

void randomMode() {
  static unsigned long nextChangeTime = 0;
  static Direction randomDirection = FORWARD;
  static uint8_t randomPwm = MIN_PWM;

  if (!startStopActive) {
    stopMotor();
    return;
  }

  if (bothLimitsHit()) {
    emergencyStop("BOTH_LIMITS");
    return;
  }

  unsigned long now = millis();
  if (now >= nextChangeTime) {
    randomDirection = random(0, 2) == 0 ? FORWARD : REVERSE;
    randomPwm = random(MIN_PWM, currentPwm + 1);
    nextChangeTime = now + random(RANDOM_ACTION_MIN_MS, RANDOM_ACTION_MAX_MS);
  }

  if (randomDirection == FORWARD && rightLimitHit) {
    randomDirection = REVERSE;
  } else if (randomDirection == REVERSE && leftLimitHit) {
    randomDirection = FORWARD;
  }

  setMotor(randomDirection, randomPwm);
}

void startFlushSequence() {
  flushDirection = random(0, 2) == 0 ? FORWARD : REVERSE;
  if (flushDirection == FORWARD && rightLimitHit) {
    flushDirection = REVERSE;
  } else if (flushDirection == REVERSE && leftLimitHit) {
    flushDirection = FORWARD;
  }

  flushWaitDuration = random(FLUSH_WAIT_MIN_MS, FLUSH_WAIT_MAX_MS);
  flushStartTime = millis();
  flushState = FLUSH_WAIT;
  stopMotor();
}

void flushMode() {
  if (!startStopActive) {
    stopMotor();
    flushState = FLUSH_IDLE;
    return;
  }

  if (bothLimitsHit()) {
    emergencyStop("BOTH_LIMITS");
    return;
  }

  unsigned long now = millis();
  switch (flushState) {
    case FLUSH_IDLE:
      startFlushSequence();
      break;
    case FLUSH_WAIT:
      if (now - flushStartTime >= flushWaitDuration) {
        setMotor(flushDirection, FLUSH_SPEED);
        flushStartTime = now;
        flushState = FLUSH_RUNNING;
      }
      break;
    case FLUSH_RUNNING:
      if (now - flushStartTime >= FLUSH_RUN_MS ||
          (flushDirection == FORWARD && rightLimitHit) ||
          (flushDirection == REVERSE && leftLimitHit)) {
        stopMotor();
        flushStartTime = now;
        flushState = FLUSH_DONE;
      }
      break;
    case FLUSH_DONE:
      if (now - flushStartTime >= FLUSH_DONE_MS) {
        flushState = FLUSH_IDLE;
      }
      break;
  }
}

void centeringMode() {
  if (!startStopActive) {
    stopMotor();
    return;
  }

  if (bothLimitsHit()) {
    emergencyStop("BOTH_LIMITS");
    return;
  }

  if (currentDirection == FORWARD) {
    if (rightLimitHit) {
      stopMotor();
      currentDirection = REVERSE;
    } else {
      setMotor(FORWARD, CENTERING_SPEED);
    }
  } else {
    if (leftLimitHit) {
      stopMotor();
      currentDirection = FORWARD;
    } else {
      setMotor(REVERSE, CENTERING_SPEED);
    }
  }
}

void setup() {
#ifdef DEBUG_SERIAL
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  debugLog("CrossWind setup starting");
#endif

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
  digitalWrite(STATUS_LED, LOW);
  randomSeed(analogRead(A1) ^ analogRead(A2) ^ micros());
}

void loop() {
  readButtons();
  readLimits();
  readSpeedPot();
  handleModeChange();

  switch (currentMode) {
    case MANUAL:
      manualMode();
      break;
    case RANDOM:
      randomMode();
      break;
    case FLUSH:
      flushMode();
      break;
    case CENTERING:
      centeringMode();
      break;
  }

  digitalWrite(STATUS_LED, startStopActive ? HIGH : LOW);
}
