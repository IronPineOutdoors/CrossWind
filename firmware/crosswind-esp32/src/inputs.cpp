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
static DebouncedButton estopButton = { ESTOP_PIN, false, false, false, 0 };
static DebouncedButton encoderButton = { ROTARY_ENCODER_SW_PIN, false, false, false, 0 };
static DebouncedButton armButton = { ARM_BUTTON_PIN, false, false, false, 0 };
static DebouncedButton fireButton = { FIRE_BUTTON_PIN, false, false, false, 0 };
static int lastSpeedRaw = 0;
static uint8_t lastSpeedPwm = DEFAULT_PWM;
static bool lastEncoderClk = HIGH;

static bool readPressed(int pin) {
  if (pin < 0) {
    return false;
  }
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

static void updateRotaryEncoder() {
  bool clk = digitalRead(ROTARY_ENCODER_CLK_PIN);
  if (clk == lastEncoderClk) {
    return;
  }
  lastEncoderClk = clk;

  if (clk == LOW) {
    bool dt = digitalRead(ROTARY_ENCODER_DT_PIN);
    if (dt == HIGH) {
      lastSpeedPwm = lastSpeedPwm > ROTARY_ENCODER_SPEED_STEP ? lastSpeedPwm - ROTARY_ENCODER_SPEED_STEP : 0;
      Serial.print("Encoder speed: ");
      Serial.println(lastSpeedPwm);
      return;
    }

    uint16_t nextSpeed = lastSpeedPwm + ROTARY_ENCODER_SPEED_STEP;
    lastSpeedPwm = nextSpeed > MAX_PWM ? MAX_PWM : nextSpeed;
    Serial.print("Encoder speed: ");
    Serial.println(lastSpeedPwm);
  }
}

void beginInputs() {
  if (START_STOP_BUTTON_PIN >= 0) {
    pinMode(START_STOP_BUTTON_PIN, INPUT_PULLUP);
  }
  if (MODE_BUTTON_PIN >= 0) {
    pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  }
  if (ESTOP_PIN >= 0) {
    pinMode(ESTOP_PIN, INPUT_PULLUP);
  }
  pinMode(ARM_BUTTON_PIN, INPUT_PULLUP);
  pinMode(FIRE_BUTTON_PIN, INPUT_PULLUP);
  if (SPEED_INPUT_TYPE == SPEED_INPUT_ROTARY_ENCODER) {
    pinMode(ROTARY_ENCODER_CLK_PIN, INPUT_PULLUP);
    pinMode(ROTARY_ENCODER_DT_PIN, INPUT_PULLUP);
    pinMode(ROTARY_ENCODER_SW_PIN, INPUT_PULLUP);
    lastEncoderClk = digitalRead(ROTARY_ENCODER_CLK_PIN);
  } else {
    pinMode(SPEED_POT_PIN, INPUT);
#if defined(ARDUINO_ARCH_ESP32)
    analogReadResolution(12);
    analogSetPinAttenuation(SPEED_POT_PIN, ADC_11db);
#endif
  }
  startButton.rawPressed = startButton.stablePressed = readPressed(START_STOP_BUTTON_PIN);
  modeButton.rawPressed = modeButton.stablePressed = readPressed(MODE_BUTTON_PIN);
  estopButton.rawPressed = estopButton.stablePressed = readPressed(ESTOP_PIN);
  encoderButton.rawPressed = encoderButton.stablePressed = readPressed(ROTARY_ENCODER_SW_PIN);
  armButton.rawPressed = armButton.stablePressed = readPressed(ARM_BUTTON_PIN);
  fireButton.rawPressed = fireButton.stablePressed = readPressed(FIRE_BUTTON_PIN);
}

void updateInputs() {
  updateButton(startButton);
  updateButton(modeButton);
  updateButton(estopButton);
  updateButton(armButton);
  updateButton(fireButton);
  if (SPEED_INPUT_TYPE == SPEED_INPUT_ROTARY_ENCODER) {
    updateButton(encoderButton);
    updateRotaryEncoder();
    lastSpeedRaw = map(lastSpeedPwm, 0, MAX_PWM, 0, 4095);
  } else {
    lastSpeedRaw = analogRead(SPEED_POT_PIN);
    lastSpeedRaw = constrain(lastSpeedRaw, 0, 4095);
    lastSpeedPwm = (uint8_t)constrain(map(lastSpeedRaw, 0, 4095, 0, MAX_PWM), 0, MAX_PWM);
  }
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

bool consumeFirePressed() {
  bool event = fireButton.pressEvent;
  fireButton.pressEvent = false;
  if (event) {
    Serial.println("Fire button pressed");
  }
  return event;
}

bool consumeArmPressed() {
  bool event = armButton.pressEvent;
  armButton.pressEvent = false;
  return event;
}

bool consumeMenuPressed() {
  bool event = encoderButton.pressEvent;
  encoderButton.pressEvent = false;
  return event;
}

bool emergencyStopActive() {
  return estopButton.stablePressed;
}

uint8_t readSpeedPwm() {
  return lastSpeedPwm;
}

int readSpeedRaw() {
  return lastSpeedRaw;
}
