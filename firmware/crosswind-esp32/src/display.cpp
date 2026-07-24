#include "display.h"

#include <U8g2lib.h>
#include <Wire.h>
#include <esp_task_wdt.h>

#include "environment.h"
#include "limits.h"
#include "trigger.h"

static constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;
static constexpr uint8_t OLED_I2C_FALLBACK_ADDRESS = 0x3D;
static constexpr uint32_t OLED_I2C_CLOCK_HZ = 100000;
static constexpr uint16_t DISPLAY_POWER_SETTLE_MS = 500;
static constexpr uint16_t DISPLAY_UPDATE_INTERVAL_MS = 250;
static constexpr uint16_t DISPLAY_RETRY_INTERVAL_MS = 2000;
static constexpr uint16_t DISPLAY_STARTUP_TEST_MS = 1000;

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

static void showStartupTest() {
  display.clearBuffer();
  display.setFont(u8g2_font_6x12_tr);
  display.setFontMode(1);
  display.setDrawColor(1);
  display.drawStr(0, 12, "CROSSWIND");
  display.drawStr(0, 30, "Display OK");
  display.drawStr(0, 48, "SSD1309 128x64");
  display.sendBuffer();
  delay(DISPLAY_STARTUP_TEST_MS);
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
  showStartupTest();
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

void updateDisplay(const ControllerState& state, bool systemArmed, bool setupDisplayMode) {
  if (!displayReady) {
    if (millis() - lastDisplayInitAttempt < DISPLAY_RETRY_INTERVAL_MS || !tryInitDisplay()) return;
  }

  unsigned long now = millis();
  if (now - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL_MS) return;
  lastDisplayUpdate = now;

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

  if (setupDisplayMode) {
    snprintf(line, sizeof(line), "L:%d raw:%d", leftLimitActive() ? 1 : 0, leftLimitRawLevel());
    display.drawStr(0, 48, line);
    snprintf(line, sizeof(line), "R:%d raw:%d", rightLimitActive() ? 1 : 0, rightLimitRawLevel());
    display.drawStr(0, 58, line);
  } else {
    snprintf(line, sizeof(line), "Limit: %s", limitStatusText(state));
    display.drawStr(0, 48, line);
  }
  display.sendBuffer();
}
