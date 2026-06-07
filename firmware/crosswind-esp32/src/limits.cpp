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
  pinMode(LEFT_LIMIT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_LIMIT_PIN, INPUT_PULLUP);
  leftLimit.rawActive = leftLimit.stableActive = readActive(LEFT_LIMIT_PIN);
  rightLimit.rawActive = rightLimit.stableActive = readActive(RIGHT_LIMIT_PIN);
}

void updateLimits() {
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
