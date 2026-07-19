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
#if CROSSWIND_MOTION_SIMULATION
  (void)pwm;
#elif ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(RPWM_PIN, pwm);
#else
  ledcWrite(PWM_CHANNEL_RIGHT, pwm);
#endif
}

static void writeLeftPwm(uint8_t pwm) {
#if CROSSWIND_MOTION_SIMULATION
  (void)pwm;
#elif ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(LPWM_PIN, pwm);
#else
  ledcWrite(PWM_CHANNEL_LEFT, pwm);
#endif
}

static void writeOptionalPin(int pin, uint8_t value) {
#if CROSSWIND_MOTION_SIMULATION
  (void)pin;
  (void)value;
#else
  if (pin >= 0) {
    digitalWrite(pin, value);
  }
#endif
}

static void writeOutputs(Direction dir, uint8_t pwm) {
  if (dir == DIR_RIGHT) {
    writeOptionalPin(L_EN_PIN, LOW);
    writeLeftPwm(0);
    writeOptionalPin(R_EN_PIN, HIGH);
    writeRightPwm(pwm);
  } else {
    writeOptionalPin(R_EN_PIN, LOW);
    writeRightPwm(0);
    writeOptionalPin(L_EN_PIN, HIGH);
    writeLeftPwm(pwm);
  }
}

void beginMotor() {
#if !CROSSWIND_MOTION_SIMULATION
  pinMode(RPWM_PIN, OUTPUT);
  pinMode(LPWM_PIN, OUTPUT);
  if (R_EN_PIN >= 0) {
    pinMode(R_EN_PIN, OUTPUT);
  }
  if (L_EN_PIN >= 0) {
    pinMode(L_EN_PIN, OUTPUT);
  }

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(RPWM_PIN, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(LPWM_PIN, PWM_FREQ, PWM_RESOLUTION);
#else
  ledcSetup(PWM_CHANNEL_RIGHT, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_LEFT, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(RPWM_PIN, PWM_CHANNEL_RIGHT);
  ledcAttachPin(LPWM_PIN, PWM_CHANNEL_LEFT);
#endif
#endif

  stopMotor();
#if CROSSWIND_MOTION_SIMULATION
  Serial.println("Motion simulation: motor GPIO outputs disabled");
#endif
}

void stopMotor() {
  requestedPwm = 0;
  appliedPwm = 0;
  motorRunning = false;
  directionChangePending = false;
  writeOptionalPin(R_EN_PIN, LOW);
  writeOptionalPin(L_EN_PIN, LOW);
  writeRightPwm(0);
  writeLeftPwm(0);
}

void driveMotor(Direction dir, uint8_t pwm) {
  uint8_t normalizedPwm = normalizeRunPwm(pwm);
  if (normalizedPwm == 0) {
    stopMotor();
    return;
  }

  requestedDirection = dir;
  requestedPwm = normalizedPwm;

  if (motorRunning && dir != activeDirection && !directionChangePending) {
    writeRightPwm(0);
    writeLeftPwm(0);
    writeOptionalPin(R_EN_PIN, LOW);
    writeOptionalPin(L_EN_PIN, LOW);
    appliedPwm = 0;
    motorRunning = false;
    directionChangePending = true;
    directionDeadtimeUntil = millis() + DIRECTION_CHANGE_DEADTIME_MS;
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
}

uint8_t motorAppliedPwm() {
  return appliedPwm;
}

Direction motorDirection() {
  return activeDirection;
}
