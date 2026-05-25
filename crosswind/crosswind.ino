#include <Arduino.h>

// CrossWind
// Controls a bidirectional pump/motor with manual, random, flush, and centering modes.

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
const uint8_t FLUSH_SPEED = 255;
const uint8_t CENTERING_SPEED = 100;

enum Direction { FORWARD, REVERSE };
enum Mode { MANUAL, RANDOM, FLUSH, CENTERING };

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
  } else {
    modeButtonEvent = false;
  }

  lastStartStopState = rawStart;
  lastModeButtonState = rawMode;
}

void readLimits() {
  leftLimitHit = digitalRead(LEFT_LIMIT) == LOW;
  rightLimitHit = digitalRead(RIGHT_LIMIT) == LOW;
}

void readSpeedPot() {
  currentPwm = map(analogRead(SPEED_POT), 0, 1023, MIN_PWM, MAX_PWM);
}

void manualMode() {
  if (!startStopActive) {
    stopMotor();
    return;
  }

  if (currentDirection == FORWARD) {
    if (rightLimitHit) {
      stopMotor();
      delay(500);
      currentDirection = REVERSE;
    } else {
      setMotor(FORWARD, currentPwm);
    }
  } else {
    if (leftLimitHit) {
      stopMotor();
      delay(500);
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

void flushMode() {
  if (!startStopActive) {
    stopMotor();
    return;
  }

  stopMotor();
  delay(random(PAUSE_MIN_MS, PAUSE_MAX_MS));

  if (random(0, 2) == 1) {
    setMotor(FORWARD, FLUSH_SPEED);
    if (rightLimitHit) {
      stopMotor();
    } else {
      rampMotor(FORWARD, FLUSH_SPEED);
      delay(1000);
      stopMotor();
    }
  } else {
    setMotor(REVERSE, FLUSH_SPEED);
    if (leftLimitHit) {
      stopMotor();
    } else {
      rampMotor(REVERSE, FLUSH_SPEED);
      delay(1000);
      stopMotor();
    }
  }
}

void centeringMode() {
  if (!startStopActive) {
    stopMotor();
    return;
  }

  if (currentDirection == FORWARD) {
    if (rightLimitHit) {
      stopMotor();
      delay(500);
      currentDirection = REVERSE;
    } else {
      setMotor(FORWARD, CENTERING_SPEED);
    }
  } else {
    if (leftLimitHit) {
      stopMotor();
      delay(500);
      currentDirection = FORWARD;
    } else {
      setMotor(REVERSE, CENTERING_SPEED);
    }
  }
}

void setup() {
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

  if (modeButtonEvent) {
    currentMode = (Mode)((currentMode + 1) % 4);
    delay(250);
  }

  digitalWrite(STATUS_LED, startStopActive ? HIGH : LOW);

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
}
