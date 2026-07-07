#include "limits.h"

#include "config.h"

struct DebouncedLimit {
  int pin;
  bool rawActive;
  bool stableActive;
  unsigned long changedAt;
};

static DebouncedLimit leftLimit = { LEFT_LIMIT_PIN, false, false, 0 };
static DebouncedLimit rightLimit = { RIGHT_LIMIT_PIN, false, false, 0 };

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
  if (!ENABLE_LIMIT_SWITCHES || LEFT_LIMIT_PIN < 0) {
    return -1;
  }
  return digitalRead(LEFT_LIMIT_PIN);
}

int rightLimitRawLevel() {
  if (!ENABLE_LIMIT_SWITCHES || RIGHT_LIMIT_PIN < 0) {
    return -1;
  }
  return digitalRead(RIGHT_LIMIT_PIN);
}

void enterCalibrationMode() {
  Serial.println("Calibration mode placeholder: Alpha limit switches are safety inputs only");
}

void setLeftLimitReference() {
  Serial.println("Left limit reference placeholder: no Alpha position tracking yet");
}

void setRightLimitReference() {
  Serial.println("Right limit reference placeholder: no Alpha position tracking yet");
}
