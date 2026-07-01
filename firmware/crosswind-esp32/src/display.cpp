#include "display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "trigger.h"

static constexpr int SCREEN_WIDTH = 128;
static constexpr int SCREEN_HEIGHT = 64;
static constexpr int OLED_RESET = -1;
static constexpr uint8_t OLED_ADDRESS = 0x3C;
static constexpr uint16_t DISPLAY_UPDATE_INTERVAL_MS = 250;

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
static bool displayReady = false;
static unsigned long lastDisplayUpdate = 0;

static int motorPercent(const ControllerState& state) {
  return map(state.speed, 0, 255, 0, 100);
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

void updateDisplay(const ControllerState& state, bool systemArmed, bool setupDisplayMode) {
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
  display.print(motorPercent(state));
  display.println("%");

  display.print("State: ");
  display.println(systemArmed ? "ARMED" : "SAFE");

  display.print("Relay: ");
  display.println(isTriggerActive() ? "ON" : "OFF");

  display.print("Menu: ");
  display.println(setupDisplayMode ? "SETUP" : "MAIN");
  display.display();
}
