#include "display.h"

#include <U8g2lib.h>
#include <Wire.h>
#include <esp_task_wdt.h>

#include "diagnostics.h"
#include "environment.h"
#include "limits.h"
#include "trigger.h"

static constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;
static constexpr uint8_t OLED_I2C_FALLBACK_ADDRESS = 0x3D;
static constexpr uint32_t OLED_I2C_CLOCK_HZ = 100000;
static constexpr uint16_t DISPLAY_POWER_SETTLE_MS = 500;
static constexpr uint16_t DISPLAY_UPDATE_INTERVAL_MS = 250;
static constexpr uint16_t DISPLAY_RETRY_INTERVAL_MS = 2000;
static constexpr uint16_t DISPLAY_STARTUP_ANIMATION_MS = 1000;
static constexpr uint8_t DISPLAY_STARTUP_FRAME_MS = 40;

// Confirmed in the installed U8g2 library. Full-buffer mode uses 1024 bytes
// for the 128 x 64 monochrome frame buffer.
static U8G2_SSD1309_128X64_NONAME0_F_HW_I2C display(
  U8G2_R0, OLED_RESET_PIN, OLED_SCL_PIN, OLED_SDA_PIN
);
static bool displayReady = false;
static bool displayWarningPrinted = false;
static unsigned long lastDisplayUpdate = 0;
static unsigned long lastDisplayInitAttempt = 0;

static bool deviceResponding(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

static void resetDisplay() {
  pinMode(OLED_RESET_PIN, OUTPUT);
  digitalWrite(OLED_RESET_PIN, HIGH);
  delay(1);
  digitalWrite(OLED_RESET_PIN, LOW);
  delay(10);
  digitalWrite(OLED_RESET_PIN, HIGH);
  delay(10);
}

static void printI2cDiagnostics() {
  Serial.println("I2C scan:");
  uint8_t deviceCount = 0;
  for (uint8_t address = 1; address < 127; ++address) {
    if ((address & 0x07) == 0) {
      esp_task_wdt_reset();
    }
    if (!deviceResponding(address)) {
      continue;
    }
    Serial.print("  Device at 0x");
    if (address < 0x10) {
      Serial.print('0');
    }
    Serial.println(address, HEX);
    ++deviceCount;
  }
  if (deviceCount == 0) {
    Serial.println("  No I2C devices detected");
  }
}

static uint8_t detectedDisplayAddress() {
  if (deviceResponding(OLED_I2C_ADDRESS)) {
    return OLED_I2C_ADDRESS;
  }
  if (deviceResponding(OLED_I2C_FALLBACK_ADDRESS)) {
    return OLED_I2C_FALLBACK_ADDRESS;
  }
  return 0;
}

static int motorPercent(const ControllerState& state) {
  return map(state.speed, 0, 255, 0, 100);
}

static const char* displayStatusText(const ControllerState& state, bool systemArmed) {
  EnvironmentStatus environmentStatus = getEnvironmentStatus();
  if (isTriggerActive()) return "FIRING";
  if (state.faultActive) return "FAULT";
  if (environmentStatus == ENV_STATUS_HOT || environmentStatus == ENV_STATUS_TEMP_FAULT || environmentStatus == ENV_STATUS_ERROR) return "WARNING";
  return systemArmed ? "ARMED" : "SAFE";
}

static const char* limitStatusText(const ControllerState& state) {
  if (state.faultActive && state.lastFault == FAULT_BOTH_LIMITS) return "FAULT: BOTH";
  if (state.faultActive && state.lastFault == FAULT_LIMIT) return "FAULT: LIMIT";
  if (leftLimitActive() || rightLimitActive()) return "ACTIVE";
  return "OK";
}

static void drawMenuRow(uint8_t y, bool selected, const char* text) {
  if (selected) {
    display.drawBox(0, y - 8, 128, 10);
    display.setDrawColor(0);
  }
  display.drawStr(3, y, text);
  display.setDrawColor(1);
}

static void drawMenuList(const char* title, const char* const* items, uint8_t itemCount,
                         uint8_t selection) {
  display.setFont(u8g2_font_5x8_tr);
  display.drawStr(0, 8, title);
  display.drawHLine(0, 10, 128);

  uint8_t first = selection > 3 ? selection - 3 : 0;
  if (first + 5 > itemCount && itemCount > 5) first = itemCount - 5;
  for (uint8_t row = 0; row < 5 && first + row < itemCount; ++row) {
    uint8_t item = first + row;
    drawMenuRow(20 + row * 10, item == selection, items[item]);
  }
}

static void drawMenu(const ControllerState& state, MenuPage page, uint8_t selection,
                     bool editing) {
  char line[24];
  display.clearBuffer();
  display.setFontMode(1);
  display.setDrawColor(1);

  if (page == MENU_ROOT) {
    static const char* const items[] = {
      "Motor", "Mode", "Environment", "Diagnostics", "About", "Exit"
    };
    drawMenuList("CROSSWIND MENU", items, 6, selection);
  } else if (page == MENU_MOTOR) {
    const char* items[4];
    char runLine[20];
    char speedLine[20];
    char directionLine[20];
    snprintf(runLine, sizeof(runLine), "%s Motor", state.running ? "Stop" : "Start");
    snprintf(speedLine, sizeof(speedLine), "Speed: %d%%%s", motorPercent(state), editing ? " *" : "");
    snprintf(directionLine, sizeof(directionLine), "Direction: %s", directionToString(state.direction));
    items[0] = runLine;
    items[1] = speedLine;
    items[2] = directionLine;
    items[3] = "< Back";
    drawMenuList("MOTOR", items, 4, selection);
  } else if (page == MENU_MODE) {
    const char* items[5];
    char modeLines[4][18];
    static const char* const names[] = {"Sweep", "Random", "Flush", "Centering"};
    for (uint8_t i = 0; i < 4; ++i) {
      snprintf(modeLines[i], sizeof(modeLines[i]), "%c %s", state.mode == (Mode)i ? '*' : ' ', names[i]);
      items[i] = modeLines[i];
    }
    items[4] = "< Back";
    drawMenuList("MODE", items, 5, selection);
  } else {
    display.setFont(u8g2_font_5x8_tr);
    const char* title = page == MENU_ENVIRONMENT ? "ENVIRONMENT" :
                        page == MENU_DIAGNOSTICS ? "DIAGNOSTICS" : "ABOUT";
    display.drawStr(0, 8, title);
    display.drawHLine(0, 10, 128);

    if (page == MENU_ENVIRONMENT) {
      if (environmentDataValid()) {
        snprintf(line, sizeof(line), "Temp: %.1f F", getTemperatureF());
        display.drawStr(0, 22, line);
        snprintf(line, sizeof(line), "Humidity: %.1f%%", getHumidity());
        display.drawStr(0, 33, line);
        snprintf(line, sizeof(line), "Pressure: %.0f hPa", getPressureHpa());
        display.drawStr(0, 44, line);
      } else {
        display.drawStr(0, 27, "Sensor unavailable");
      }
    } else if (page == MENU_DIAGNOSTICS) {
      snprintf(line, sizeof(line), "Limits L:%s R:%s", leftLimitActive() ? "ON" : "OK", rightLimitActive() ? "ON" : "OK");
      display.drawStr(0, 22, line);
      snprintf(line, sizeof(line), "Fault: %s", state.faultActive ? faultToString(state.lastFault) : "NONE");
      display.drawStr(0, 33, line);
      snprintf(line, sizeof(line), "Relay: %s", isTriggerActive() ? "ON" : "OFF");
      display.drawStr(0, 44, line);
    } else {
      display.drawStr(0, 23, "Crosswind Controller");
      display.drawStr(0, 35, FIRMWARE_VERSION);
    }
    drawMenuRow(62, true, "< Back");
  }

  display.sendBuffer();
}

static void drawWindStreak(uint8_t x, uint8_t y, uint8_t length) {
  display.drawHLine(x, y, length);
  display.drawPixel(x + length + 2, y);
}

static void showStartupAnimation() {
  static const int8_t rotorX[8] = {0, 5, 7, 5, 0, -5, -7, -5};
  static const int8_t rotorY[8] = {-7, -5, 0, 5, 7, 5, 0, -5};
  const uint8_t frameCount = DISPLAY_STARTUP_ANIMATION_MS / DISPLAY_STARTUP_FRAME_MS;

  display.setFontMode(1);
  display.setDrawColor(1);

  for (uint8_t frame = 0; frame < frameCount; ++frame) {
    display.clearBuffer();

    // Wind streaks sweep behind a small, rotating three-blade rotor.
    uint8_t sweep = (frame * 6) % 42;
    drawWindStreak((sweep + 2) % 42, 11, 12);
    drawWindStreak((sweep + 23) % 42, 19, 8);
    drawWindStreak((sweep + 11) % 42, 51, 15);

    const uint8_t rotorPhase = frame & 0x07;
    display.drawCircle(22, 32, 11);
    display.drawDisc(22, 32, 2);
    for (uint8_t blade = 0; blade < 3; ++blade) {
      uint8_t point = (rotorPhase + blade * 3) & 0x07;
      display.drawLine(22, 32, 22 + rotorX[point], 32 + rotorY[point]);
      display.drawDisc(22 + rotorX[point], 32 + rotorY[point], 1);
    }

    display.setFont(u8g2_font_7x13B_tr);
    display.drawStr(43, 29, "CROSSWIND");
    display.setFont(u8g2_font_5x8_tr);
    display.drawStr(44, 41, frame < frameCount - 4 ? "SYSTEM START" : "READY");

    display.drawFrame(43, 48, 81, 7);
    uint8_t progress = ((frame + 1) * 77) / frameCount;
    display.drawBox(45, 50, progress, 3);
    display.sendBuffer();

    esp_task_wdt_reset();
    delay(DISPLAY_STARTUP_FRAME_MS);
  }
}

static bool tryInitDisplay() {
  lastDisplayInitAttempt = millis();
  resetDisplay();
  uint8_t displayAddress = detectedDisplayAddress();
  if (displayAddress == 0) {
    if (!displayWarningPrinted) {
      Serial.println("WARNING: SSD1309 OLED not found at 0x3C or 0x3D; retrying");
      displayWarningPrinted = true;
    }
    return false;
  }

  // U8g2 expects an 8-bit I2C address; diagnostics print conventional 7-bit addresses.
  display.setI2CAddress(displayAddress << 1);
  display.setBusClock(OLED_I2C_CLOCK_HZ);
  if (!display.begin()) {
    if (!displayWarningPrinted) {
      Serial.println("WARNING: SSD1309 OLED initialization failed; retrying");
      displayWarningPrinted = true;
    }
    return false;
  }

  // U8g2 initializes Wire internally. Restore the finite transaction timeout
  // before sending the first full frame so a bad bus cannot stall loopTask.
  Wire.setClock(OLED_I2C_CLOCK_HZ);
  Wire.setTimeOut(50);

  displayReady = true;
  displayWarningPrinted = false;
  Serial.print("SSD1309 display ready at 0x");
  if (displayAddress < 0x10) Serial.print('0');
  Serial.println(displayAddress, HEX);
  showStartupAnimation();
  return true;
}

void initDisplay() {
  // On a cold USB power-up the OLED rail can rise more slowly than the ESP32.
  // Let the panel power stabilize before issuing its first hardware reset.
  delay(DISPLAY_POWER_SETTLE_MS);
  resetDisplay();
  printI2cDiagnostics();
  tryInitDisplay();
}

void updateDisplay(const ControllerState& state, bool systemArmed, MenuPage menuPage,
                   uint8_t menuSelection, bool menuEditing) {
  if (!displayReady) {
    if (millis() - lastDisplayInitAttempt < DISPLAY_RETRY_INTERVAL_MS || !tryInitDisplay()) return;
  }

  unsigned long now = millis();
  if (now - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL_MS) return;
  lastDisplayUpdate = now;

  if (menuPage != MENU_CLOSED) {
    drawMenu(state, menuPage, menuSelection, menuEditing);
    return;
  }

  char line[24];
  display.clearBuffer();
  display.setFont(u8g2_font_5x8_tr);
  display.setFontMode(1);
  display.setDrawColor(1);
  display.drawStr(0, 8, "CROSSWIND");

  snprintf(line, sizeof(line), "Motor: %d%%", motorPercent(state));
  display.drawStr(0, 18, line);
  snprintf(line, sizeof(line), "Status: %s", displayStatusText(state, systemArmed));
  display.drawStr(0, 28, line);
  snprintf(line, sizeof(line), "Relay: %s", isTriggerActive() ? "ON" : "OFF");
  display.drawStr(0, 38, line);

  snprintf(line, sizeof(line), "Limit: %s", limitStatusText(state));
  display.drawStr(0, 48, line);
  display.sendBuffer();
}
