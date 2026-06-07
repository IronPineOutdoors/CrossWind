#include "inputs.h"

struct DebouncedButton {
  int pin;
  bool rawPressed;
  bool stablePressed;
  bool pressEvent;
  unsigned long changedAt;
};

static DebouncedButton startButton = { START_STOP_BUTTON_PIN, false, false, false, 0 };
static DebouncedButton modeButton = { MODE_BUTTON_PIN, false, false, false, 0 };
static int lastSpeedRaw = 0;
static uint8_t lastSpeedPwm = DEFAULT_PWM;

static bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

static void updateButton(DebouncedButton& button) {
  bool raw = readPressed(button.pin);
  unsigned long now = millis();

  if (raw != button.rawPressed) {
    button.rawPressed = raw;
    button.changedAt = now;
  }

  if (now - button.changedAt >= DEBOUNCE_DELAY_MS && raw != button.stablePressed) {
    button.stablePressed = raw;
    if (button.stablePressed) {
      button.pressEvent = true;
    }
  }
}

void beginInputs() {
  pinMode(START_STOP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SPEED_POT_PIN, INPUT);
  startButton.rawPressed = startButton.stablePressed = readPressed(START_STOP_BUTTON_PIN);
  modeButton.rawPressed = modeButton.stablePressed = readPressed(MODE_BUTTON_PIN);
}

void updateInputs() {
  updateButton(startButton);
  updateButton(modeButton);
  lastSpeedRaw = analogRead(SPEED_POT_PIN);
  lastSpeedPwm = map(lastSpeedRaw, 0, 4095, 0, MAX_PWM);
}

bool consumeStartPressed() {
  bool event = startButton.pressEvent;
  startButton.pressEvent = false;
  return event;
}

bool consumeModePressed() {
  bool event = modeButton.pressEvent;
  modeButton.pressEvent = false;
  return event;
}

uint8_t readSpeedPwm() {
  return lastSpeedPwm;
}

int readSpeedRaw() {
  return lastSpeedRaw;
}
