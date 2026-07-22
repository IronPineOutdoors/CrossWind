#include "display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "environment.h"
#include "limits.h"
#include "trigger.h"

static constexpr int SCREEN_WIDTH = 128;
static constexpr int SCREEN_HEIGHT = 64;
static constexpr int OLED_RESET = -1;
static constexpr uint8_t OLED_ADDRESS_PRIMARY = 0x3C;
static constexpr uint8_t OLED_ADDRESS_SECONDARY = 0x3D;
static constexpr uint16_t DISPLAY_UPDATE_INTERVAL_MS = 250;
static constexpr uint16_t DISPLAY_RETRY_INTERVAL_MS = 2000;

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
static bool displayReady = false;
static bool displayWarningPrinted = false;
static unsigned long lastDisplayUpdate = 0;
static unsigned long lastDisplayInitAttempt = 0;

static bool displayResponding(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

static int motorPercent(const ControllerState& state) {
  return map(state.speed, 0, 255, 0, 100);
}

static const char* displayStatusText(const ControllerState& state, bool systemArmed) {
  EnvironmentStatus environmentStatus = getEnvironmentStatus();
  if (isTriggerActive()) {
    return "FIRING";
  }
  if (state.faultActive) {
    return "FAULT";
  }
  if (environmentStatus == ENV_STATUS_HOT || environmentStatus == ENV_STATUS_TEMP_FAULT || environmentStatus == ENV_STATUS_ERROR) {
    return "WARNING";
  }
  return systemArmed ? "ARMED" : "SAFE";
}

static const char* limitStatusText(const ControllerState& state) {
  if (state.faultActive && state.lastFault == FAULT_BOTH_LIMITS) {
    return "FAULT: BOTH";
  }
  if (state.faultActive && state.lastFault == FAULT_LIMIT) {
    return "FAULT: LIMIT";
  }
  if (leftLimitActive() || rightLimitActive()) {
    return "ACTIVE";
  }
  return "OK";
}

static bool tryInitDisplay() {
  lastDisplayInitAttempt = millis();
  uint8_t displayAddress = OLED_ADDRESS_PRIMARY;
  if (!displayResponding(displayAddress)) {
    displayAddress = OLED_ADDRESS_SECONDARY;
    if (!displayResponding(displayAddress)) {
      if (!displayWarningPrinted) {
        Serial.println("WARNING: SSD1306 OLED not found at 0x3C or 0x3D; retrying");
        displayWarningPrinted = true;
      }
      return false;
    }
  }

  // The environment module owns initialization of the shared I2C bus. Passing
  // false here prevents Adafruit_SSD1306 from restarting Wire after the BME280
  // probe.
  displayReady = display.begin(SSD1306_SWITCHCAPVCC, displayAddress, true, false);
  if (!displayReady) {
    if (!displayWarningPrinted) {
      Serial.println("WARNING: SSD1306 OLED initialization failed; retrying");
      displayWarningPrinted = true;
    }
    return false;
  }

  displayWarningPrinted = false;
  Serial.print("Display ready at 0x");
  Serial.println(displayAddress, HEX);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("CROSSWIND");
  display.println("Display ready");
  display.display();
  return true;
}

void initDisplay() {
  tryInitDisplay();
}

void updateDisplay(const ControllerState& state, bool systemArmed, bool setupDisplayMode) {
  if (!displayReady) {
    if (millis() - lastDisplayInitAttempt < DISPLAY_RETRY_INTERVAL_MS || !tryInitDisplay()) {
      return;
    }
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

  display.print("Status: ");
  display.println(displayStatusText(state, systemArmed));

  display.print("Relay: ");
  display.println(isTriggerActive() ? "ON" : "OFF");

  if (setupDisplayMode) {
    display.print("L:");
    display.print(leftLimitActive() ? "1" : "0");
    display.print(" raw:");
    display.println(leftLimitRawLevel());

    display.print("R:");
    display.print(rightLimitActive() ? "1" : "0");
    display.print(" raw:");
    display.println(rightLimitRawLevel());
  } else {
    display.print("Limit: ");
    display.println(limitStatusText(state));
  }
  display.display();
}
