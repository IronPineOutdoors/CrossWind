#include "status_led.h"

#include <esp_arduino_version.h>

#include "config.h"
#include "trigger.h"

enum StatusLedMode {
  STATUS_LED_OFF,
  STATUS_LED_BOOT,
  STATUS_LED_READY,
  STATUS_LED_ARMED,
  STATUS_LED_FAULT,
  STATUS_LED_WARNING
};

static StatusLedMode statusMode = STATUS_LED_OFF;
static unsigned long modeStartedAt = 0;

static void writeRed(uint8_t value) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(RGB_RED_PIN, value);
#else
  ledcWrite(RGB_RED_CHANNEL, value);
#endif
}

static void writeGreen(uint8_t value) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(RGB_GREEN_PIN, value);
#else
  ledcWrite(RGB_GREEN_CHANNEL, value);
#endif
}

static void writeBlue(uint8_t value) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(RGB_BLUE_PIN, value);
#else
  ledcWrite(RGB_BLUE_CHANNEL, value);
#endif
}

static void setMode(StatusLedMode mode) {
  if (statusMode == mode) {
    return;
  }
  if (statusMode == STATUS_LED_BOOT && millis() - modeStartedAt < 360) {
    return;
  }
  statusMode = mode;
  modeStartedAt = millis();
}

void initStatusLed() {
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(RGB_RED_PIN, RGB_PWM_FREQ, RGB_PWM_RESOLUTION);
  ledcAttach(RGB_GREEN_PIN, RGB_PWM_FREQ, RGB_PWM_RESOLUTION);
  ledcAttach(RGB_BLUE_PIN, RGB_PWM_FREQ, RGB_PWM_RESOLUTION);
#else
  ledcSetup(RGB_RED_CHANNEL, RGB_PWM_FREQ, RGB_PWM_RESOLUTION);
  ledcSetup(RGB_GREEN_CHANNEL, RGB_PWM_FREQ, RGB_PWM_RESOLUTION);
  ledcSetup(RGB_BLUE_CHANNEL, RGB_PWM_FREQ, RGB_PWM_RESOLUTION);
  ledcAttachPin(RGB_RED_PIN, RGB_RED_CHANNEL);
  ledcAttachPin(RGB_GREEN_PIN, RGB_GREEN_CHANNEL);
  ledcAttachPin(RGB_BLUE_PIN, RGB_BLUE_CHANNEL);
#endif

  modeStartedAt = millis();
  statusMode = STATUS_LED_BOOT;
  setStatusLed(0, 0, 255);
}

void setStatusLed(uint8_t r, uint8_t g, uint8_t b) {
  writeRed(r);
  writeGreen(g);
  writeBlue(b);
}

void setStatusOff() {
  statusMode = STATUS_LED_OFF;
  modeStartedAt = millis();
  setStatusLed(0, 0, 0);
}

void showReadyStatus() {
  setMode(STATUS_LED_READY);
}

void showArmedStatus() {
  setMode(STATUS_LED_ARMED);
}

void showFaultStatus() {
  setMode(STATUS_LED_FAULT);
}

void showWarningStatus() {
  setMode(STATUS_LED_WARNING);
}

void updateStatusLed() {
  unsigned long now = millis();

  if (isTriggerActive()) {
    bool flashOn = ((now / 120) % 2) == 0;
    setStatusLed(flashOn ? 255 : 80, flashOn ? 255 : 0, flashOn ? 255 : 120);
    return;
  }

  if (statusMode == STATUS_LED_BOOT) {
    unsigned long elapsed = now - modeStartedAt;
    if (elapsed < 180) {
      setStatusLed(0, 0, 255);
      return;
    }
    if (elapsed < 360) {
      setStatusLed(0, 255, 0);
      return;
    }
    setMode(STATUS_LED_READY);
  }

  switch (statusMode) {
    case STATUS_LED_READY:
      setStatusLed(0, 180, 0);
      break;
    case STATUS_LED_ARMED:
      setStatusLed(0, 0, 255);
      break;
    case STATUS_LED_FAULT:
      setStatusLed(((now / 200) % 2) == 0 ? 255 : 0, 0, 0);
      break;
    case STATUS_LED_WARNING:
      setStatusLed(220, 160, 0);
      break;
    case STATUS_LED_OFF:
      setStatusOff();
      break;
    case STATUS_LED_BOOT:
      break;
  }
}
