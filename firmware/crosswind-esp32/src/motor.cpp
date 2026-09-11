#include "motor.h"

#include <esp_arduino_version.h>

static uint8_t requestedPwm = 0;
static uint8_t appliedPwm = 0;
static Direction requestedDirection = DIR_RIGHT;
static Direction activeDirection = DIR_RIGHT;
static bool motorRunning = false;
static bool directionChangePending = false;
static unsigned long directionDeadtimeUntil = 0;
static unsigned long lastRampUpdate = 0;
static bool pwmReady = false;
static uint8_t rightPwmOutput = 0;
static uint8_t leftPwmOutput = 0;
static bool rightEnableOutput = false;
static bool leftEnableOutput = false;

static uint8_t normalizeRunPwm(uint8_t pwm) {
  if (pwm == 0) {
    return 0;
  }
  if (pwm < MIN_PWM) {
    return MIN_PWM;
  }
  return pwm > MAX_PWM ? MAX_PWM : pwm;
}

static void writeRightPwm(uint8_t pwm) {
  rightPwmOutput = pwmReady ? pwm : 0;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  if (pwmReady) ledcWrite(RPWM_PIN, pwm);
#else
  if (pwmReady) ledcWrite(PWM_CHANNEL_RIGHT, pwm);
#endif
}

static void writeLeftPwm(uint8_t pwm) {
  leftPwmOutput = pwmReady ? pwm : 0;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  if (pwmReady) ledcWrite(LPWM_PIN, pwm);
#else
  if (pwmReady) ledcWrite(PWM_CHANNEL_LEFT, pwm);
#endif
}

static void writeOptionalPin(int pin, uint8_t value) {
  if (pin >= 0) {
    digitalWrite(pin, value);
  }
}

static void writeOutputs(Direction dir, uint8_t pwm) {
  // The BTS7960 uses one enable input per half-bridge. Both halves must be
  // enabled to provide a complete current path; RPWM/LPWM select direction.
  if (!pwmReady || pwm == 0) {
    writeRightPwm(0);
    writeLeftPwm(0);
    writeOptionalPin(R_EN_PIN, LOW);
    writeOptionalPin(L_EN_PIN, LOW);
    rightEnableOutput = false;
    leftEnableOutput = false;
    return;
  }
  writeOptionalPin(R_EN_PIN, HIGH);
  writeOptionalPin(L_EN_PIN, HIGH);
  rightEnableOutput = R_EN_PIN >= 0;
  leftEnableOutput = L_EN_PIN >= 0;

  const bool useRightPwm = (dir == DIR_RIGHT) != MOTOR_DIRECTION_INVERTED;
  if (useRightPwm) {
    writeLeftPwm(0);
    writeRightPwm(pwm);
  } else {
    writeRightPwm(0);
    writeLeftPwm(pwm);
  }
}

static void writeEnables(bool enabled) {
  rightEnableOutput = enabled && R_EN_PIN >= 0;
  leftEnableOutput = enabled && L_EN_PIN >= 0;
  writeOptionalPin(R_EN_PIN, enabled ? HIGH : LOW);
  writeOptionalPin(L_EN_PIN, enabled ? HIGH : LOW);
}

static void logOutputsIfChanged() {
  static bool first = true;
  static uint8_t lastRightPwm = 0, lastLeftPwm = 0;
  static bool lastRightEnable = false, lastLeftEnable = false;
  if (!first && lastRightPwm == rightPwmOutput && lastLeftPwm == leftPwmOutput &&
      lastRightEnable == rightEnableOutput && lastLeftEnable == leftEnableOutput) return;
  first = false;
  lastRightPwm = rightPwmOutput;
  lastLeftPwm = leftPwmOutput;
  lastRightEnable = rightEnableOutput;
  lastLeftEnable = leftEnableOutput;
  Serial.printf("MOTOR OUT R_EN=%u L_EN=%u RPWM=%u LPWM=%u\n",
                rightEnableOutput, leftEnableOutput, rightPwmOutput, leftPwmOutput);
}

bool beginMotor() {
  pinMode(RPWM_PIN, OUTPUT);
  pinMode(LPWM_PIN, OUTPUT);
  if (R_EN_PIN >= 0) {
    pinMode(R_EN_PIN, OUTPUT);
  }
  if (L_EN_PIN >= 0) {
    pinMode(L_EN_PIN, OUTPUT);
  }

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  bool rightAttached = ledcAttach(RPWM_PIN, PWM_FREQ, PWM_RESOLUTION);
  bool leftAttached = ledcAttach(LPWM_PIN, PWM_FREQ, PWM_RESOLUTION);
  pwmReady = rightAttached && leftAttached;
#else
  bool rightAttached = ledcSetup(PWM_CHANNEL_RIGHT, PWM_FREQ, PWM_RESOLUTION) > 0;
  bool leftAttached = ledcSetup(PWM_CHANNEL_LEFT, PWM_FREQ, PWM_RESOLUTION) > 0;
  ledcAttachPin(RPWM_PIN, PWM_CHANNEL_RIGHT);
  ledcAttachPin(LPWM_PIN, PWM_CHANNEL_LEFT);
  pwmReady = rightAttached && leftAttached;
#endif

  stopMotor();
  Serial.printf("MOTOR PWM init %s: RPWM=%d LPWM=%d freq=%dHz resolution=%dbit\n",
                pwmReady ? "OK" : "FAILED", RPWM_PIN, LPWM_PIN, PWM_FREQ, PWM_RESOLUTION);
  Serial.printf("MOTOR direction mapping: inverted=%s (logical plate LEFT/RIGHT)\n",
                MOTOR_DIRECTION_INVERTED ? "YES" : "NO");
  return pwmReady;
}

void stopMotor() {
  requestedPwm = 0;
  appliedPwm = 0;
  motorRunning = false;
  directionChangePending = false;
  writeRightPwm(0);
  writeLeftPwm(0);
  writeEnables(false);
  logOutputsIfChanged();
}

void driveMotor(Direction dir, uint8_t pwm) {
  if (!pwmReady) {
    stopMotor();
    return;
  }
  if (dir != DIR_RIGHT && dir != DIR_LEFT) {
    Serial.println("MOTOR command rejected: invalid/conflicting direction");
    stopMotor();
    return;
  }
  uint8_t normalizedPwm = normalizeRunPwm(pwm);
  if (normalizedPwm == 0) {
    stopMotor();
    return;
  }

  bool commandChanged = requestedDirection != dir || requestedPwm != normalizedPwm;
  requestedDirection = dir;
  requestedPwm = normalizedPwm;

  if (motorRunning && dir != activeDirection && !directionChangePending) {
    writeRightPwm(0);
    writeLeftPwm(0);
    writeEnables(false);
    appliedPwm = 0;
    motorRunning = false;
    directionChangePending = true;
    directionDeadtimeUntil = millis() + DIRECTION_CHANGE_DEADTIME_MS;
    logOutputsIfChanged();
  }

  if (commandChanged) {
    Serial.printf("[MOTOR] direction=%s requestedDuty=%u/255 reversal=%s\n",
                  dir == DIR_RIGHT ? "RIGHT" : "LEFT", normalizedPwm,
                  directionChangePending ? "DEAD_TIME" : "NONE");
  }
}

void updateMotorRamp() {
  if (requestedPwm == 0) {
    return;
  }

  unsigned long now = millis();
  if (directionChangePending) {
    if ((long)(now - directionDeadtimeUntil) < 0) {
      return;
    }
    directionChangePending = false;
  }

  if (appliedPwm == 0 || now - lastRampUpdate >= RAMP_INTERVAL_MS) {
    if (appliedPwm < requestedPwm) {
      uint16_t nextPwm = appliedPwm + RAMP_STEP;
      appliedPwm = nextPwm > requestedPwm ? requestedPwm : nextPwm;
    } else if (appliedPwm > requestedPwm) {
      appliedPwm = appliedPwm > RAMP_STEP ? appliedPwm - RAMP_STEP : requestedPwm;
      if (appliedPwm < requestedPwm) {
        appliedPwm = requestedPwm;
      }
    }
    lastRampUpdate = now;
  }

  activeDirection = requestedDirection;
  motorRunning = true;
  writeOutputs(activeDirection, appliedPwm);
  logOutputsIfChanged();
}

uint8_t motorAppliedPwm() {
  return appliedPwm;
}

Direction motorDirection() {
  return activeDirection;
}

bool motorPwmReady() { return pwmReady; }
uint8_t motorRightPwm() { return rightPwmOutput; }
uint8_t motorLeftPwm() { return leftPwmOutput; }
bool motorRightEnable() { return rightEnableOutput; }
bool motorLeftEnable() { return leftEnableOutput; }
