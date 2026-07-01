#include "display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "environment.h"
#include "motor.h"

static constexpr int SCREEN_WIDTH = 128;
static constexpr int SCREEN_HEIGHT = 64;
static constexpr int OLED_RESET = -1;
static constexpr uint8_t OLED_ADDRESS = 0x3C;
static constexpr uint16_t DISPLAY_UPDATE_INTERVAL_MS = 250;

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
static bool displayReady = false;
static unsigned long lastDisplayUpdate = 0;

static int motorPercent() {
  return map(motorAppliedPwm(), 0, 255, 0, 100);
}

void initDisplay() {
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  displayReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  if (!displayReady) {
    Serial.println("WARNING: SSD1306 OLED not found at 0x3C");
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("CROSSWIND");
  display.println("Display ready");
  display.display();
}

void updateDisplay(const ControllerState& state) {
  if (!displayReady) {
    return;
  }

  unsigned long now = millis();
  if (now - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL_MS) {
    return;
  }
  lastDisplayUpdate = now;

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("CROSSWIND");

  display.print("Motor: ");
  display.print(state.running && !state.faultActive ? motorPercent() : 0);
  display.println("%");

  display.print("Temp: ");
  if (environmentDataValid()) {
    display.print((int)(getTemperatureF() + 0.5F));
    display.println("F");
  } else {
    display.println("--F");
  }

  display.print("Hum: ");
  if (environmentDataValid()) {
    display.print((int)(getHumidity() + 0.5F));
    display.println("%");
  } else {
    display.println("--%");
  }

  EnvironmentStatus envStatus = getEnvironmentStatus();
  if (state.faultActive && state.lastFault == FAULT_TEMP) {
    envStatus = ENV_STATUS_TEMP_FAULT;
  }
  display.println(environmentStatusToString(envStatus));
  display.display();
}
