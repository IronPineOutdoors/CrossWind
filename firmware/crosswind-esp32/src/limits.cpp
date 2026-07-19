#include "limits.h"

#include "config.h"

#if CROSSWIND_MOTION_SIMULATION
#include "motor.h"
#endif

struct DebouncedLimit {
  int pin;
  bool rawActive;
  bool stableActive;
  unsigned long changedAt;
};

static DebouncedLimit leftLimit = { LEFT_LIMIT_PIN, false, false, 0 };
static DebouncedLimit rightLimit = { RIGHT_LIMIT_PIN, false, false, 0 };

#if CROSSWIND_MOTION_SIMULATION
static float virtualPosition = 0.5F;
static unsigned long lastSimulationUpdate = 0;
static int simulatedLimitMask = -1;
#endif

static bool readActive(int pin) {
  if (!ENABLE_LIMIT_SWITCHES) {
    return false;
  }
  if (pin < 0) {
    return false;
  }
  return digitalRead(pin) == LIMIT_ACTIVE_STATE;
}

static void updateOne(DebouncedLimit& input) {
  bool raw = readActive(input.pin);
  unsigned long now = millis();

  if (raw != input.rawActive) {
    input.rawActive = raw;
    input.changedAt = now;
  }

  if (now - input.changedAt >= LIMIT_DEBOUNCE_MS) {
    input.stableActive = input.rawActive;
  }
}

void beginLimits() {
#if CROSSWIND_MOTION_SIMULATION
  virtualPosition = 0.5F;
  lastSimulationUpdate = millis();
  leftLimit.rawActive = leftLimit.stableActive = false;
  rightLimit.rawActive = rightLimit.stableActive = false;
  Serial.println("Motion simulation: virtual limits enabled");
  return;
#endif
  if (!ENABLE_LIMIT_SWITCHES) {
    leftLimit.rawActive = leftLimit.stableActive = false;
    rightLimit.rawActive = rightLimit.stableActive = false;
    return;
  }

  uint8_t limitPinMode = LIMIT_SWITCHES_USE_INTERNAL_PULLUPS ? INPUT_PULLUP : INPUT;
  if (LEFT_LIMIT_PIN >= 0) {
    pinMode(LEFT_LIMIT_PIN, limitPinMode);
  }
  if (RIGHT_LIMIT_PIN >= 0) {
    pinMode(RIGHT_LIMIT_PIN, limitPinMode);
  }
  leftLimit.rawActive = leftLimit.stableActive = readActive(LEFT_LIMIT_PIN);
  rightLimit.rawActive = rightLimit.stableActive = readActive(RIGHT_LIMIT_PIN);
}

void updateLimits() {
#if CROSSWIND_MOTION_SIMULATION
  unsigned long now = millis();
  unsigned long elapsed = now - lastSimulationUpdate;
  lastSimulationUpdate = now;
  uint8_t pwm = motorAppliedPwm();
  if (pwm > 0) {
    float delta = ((float)elapsed / (float)MOTION_SIMULATED_FULL_TRAVEL_MS) * ((float)pwm / (float)MAX_PWM);
    virtualPosition += motorDirection() == DIR_RIGHT ? delta : -delta;
    virtualPosition = constrain(virtualPosition, 0.0F, 1.0F);
  }
  leftLimit.rawActive = leftLimit.stableActive = virtualPosition <= 0.0F;
  rightLimit.rawActive = rightLimit.stableActive = virtualPosition >= 1.0F;
  if (simulatedLimitMask >= 0) {
    leftLimit.rawActive = leftLimit.stableActive = (simulatedLimitMask & 1) != 0;
    rightLimit.rawActive = rightLimit.stableActive = (simulatedLimitMask & 2) != 0;
  }
  return;
#endif
  if (!ENABLE_LIMIT_SWITCHES) {
    leftLimit.rawActive = leftLimit.stableActive = false;
    rightLimit.rawActive = rightLimit.stableActive = false;
    return;
  }

  updateOne(leftLimit);
  updateOne(rightLimit);
}

bool leftLimitActive() {
  return leftLimit.stableActive;
}

bool rightLimitActive() {
  return rightLimit.stableActive;
}

bool bothLimitsActive() {
  return leftLimitActive() && rightLimitActive();
}

int leftLimitRawLevel() {
#if CROSSWIND_MOTION_SIMULATION
  return leftLimitActive() ? LIMIT_ACTIVE_STATE : (LIMIT_ACTIVE_STATE == LOW ? HIGH : LOW);
#endif
  if (!ENABLE_LIMIT_SWITCHES || LEFT_LIMIT_PIN < 0) {
    return -1;
  }
  return digitalRead(LEFT_LIMIT_PIN);
}

int rightLimitRawLevel() {
#if CROSSWIND_MOTION_SIMULATION
  return rightLimitActive() ? LIMIT_ACTIVE_STATE : (LIMIT_ACTIVE_STATE == LOW ? HIGH : LOW);
#endif
  if (!ENABLE_LIMIT_SWITCHES || RIGHT_LIMIT_PIN < 0) {
    return -1;
  }
  return digitalRead(RIGHT_LIMIT_PIN);
}

bool motionSimulationEnabled() {
  return CROSSWIND_MOTION_SIMULATION != 0;
}

void setSimulatedLimitMask(int mask) {
#if CROSSWIND_MOTION_SIMULATION
  simulatedLimitMask = constrain(mask, -1, 3);
#else
  (void)mask;
#endif
}
