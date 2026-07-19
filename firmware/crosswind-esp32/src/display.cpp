#include "display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "battery_monitor.h"
#include "ble_control.h"
#include "diagnostics.h"
#include "environment.h"
#include "inputs.h"
#include "limits.h"
#include "menu.h"
#include "motion.h"
#include "motor.h"
#include "trigger.h"

static constexpr int SCREEN_WIDTH = 128;
static constexpr int SCREEN_HEIGHT = 64;
static constexpr int OLED_RESET = -1;
static constexpr uint8_t OLED_ADDRESS = 0x3C;

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
static bool displayReady = false;
static bool displayPowered = true;
static unsigned long lastDisplayUpdate = 0;
static UiState lastRenderedState = UI_BOOT;
static uint8_t lastContrast = 0;

static void title(const char* text) {
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println(text);
  display.drawFastHLine(0, 9, 128, SSD1306_WHITE);
  display.setCursor(0, 12);
}

static void footer(const char* text) {
  display.setCursor(0, 56);
  display.print(text);
}

static void menuRow(const char* text, bool selected, bool enabled = true) {
  int16_t y = display.getCursorY();
  if (selected) {
    display.fillRect(0, y - 1, 128, 9, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  } else {
    display.setTextColor(enabled ? SSD1306_WHITE : SSD1306_BLACK, enabled ? SSD1306_BLACK : SSD1306_WHITE);
  }
  display.print(selected ? ">" : " ");
  display.println(text);
  display.setTextColor(SSD1306_WHITE);
}

static void renderBoot() {
  display.setTextSize(1);
  display.setCursor(9, 7);
  display.println("IRON PINE OUTDOORS");
  display.setTextSize(2);
  display.setCursor(14, 23);
  display.println("CROSSWIND");
  display.setTextSize(1);
  display.setCursor(5, 45);
  display.println(FIRMWARE_VERSION);
  display.setCursor(34, 56);
  display.println("INITIALIZING");
}

static const char* warningText() {
  if (batteryStatus() == BATTERY_CRITICAL) return "BAT CRITICAL";
  if (batteryStatus() == BATTERY_LOW || batteryStatus() == BATTERY_RECOVERING) return "BAT LOW";
  if (batteryStatus() == BATTERY_SENSOR_ERROR) return "BAT ERROR";
  EnvironmentStatus environment = getEnvironmentStatus();
  if (environment == ENV_STATUS_TEMP_FAULT) return "TEMP FAULT";
  if (environment == ENV_STATUS_HOT) return "TEMP HIGH";
  if (environment == ENV_STATUS_ERROR) return "ENV ERROR";
  return "OK";
}

static void renderStatus(const ControllerState& state, bool armed) {
  title("CROSSWIND STATUS");
  display.print(armed ? "ARMED " : "SAFE  ");
  display.print(state.running ? "RUN " : "STOP ");
  display.println(modeToString(state.mode));
  display.print(motionPhaseToString());
  display.print(" ");
  display.println(directionToString(motionCommandedDirection()));
  display.print("SPD ");
  display.print(state.speed);
  display.print(" PWM ");
  display.println(motorAppliedPwm());
  display.print("BAT ");
  if (!batteryMonitoringEnabled()) display.print("OFF");
  else if (batterySensorValid()) { display.print(batteryVoltage(), 1); display.print("V"); }
  else display.print(batteryStatusToString(batteryStatus()));
  display.print(" BLE ");
  display.println(bleClientConnected() ? "ON" : "--");
  display.print("WARN ");
  display.println(warningText());
  footer("Press:Menu  Turn:Speed");
}

static const char* const topItems[] = { "Status", "Mode", "Motion", "Trigger", "Battery", "Environment", "Calibration", "Diagnostics", "Settings", "Back" };

static void renderTopMenu() {
  title("MENU");
  uint8_t selected = uiSelectedItem();
  uint8_t first = selected > 3 ? selected - 3 : 0;
  if (first > 5) first = 5;
  for (uint8_t i = first; i < first + 5; i++) menuRow(topItems[i], i == selected);
  footer("Hold:Back");
}

static void renderMode(const ControllerState& state) {
  title("SELECT MODE");
  const char* modes[] = { "SWEEP", "RANDOM", "FLUSH", "CENTERING", "Back" };
  for (uint8_t i = 0; i < 5; i++) menuRow(modes[i], i == uiSelectedItem(), !state.running || i == 4);
  if (state.running) footer("STOP required");
}

static void renderMotion(const ControllerState& state) {
  title("MOTION");
  char speed[20];
  snprintf(speed, sizeof(speed), "Speed: %u", state.speed);
  menuRow(speed, uiSelectedItem() == 0);
  menuRow(state.running ? "Stop motion" : "Start motion", uiSelectedItem() == 1);
  menuRow("Back", uiSelectedItem() == 2);
  display.println("Limits/timing bounded");
  footer("Press:Select");
}

static void renderTrigger(bool armed) {
  title("TRIGGER");
  display.print("Automatic: ");
  display.println(ENABLE_AUTOMATIC_TRIGGER ? "AVAILABLE" : "UNAVAILABLE");
  display.print("State: ");
  display.println(armed ? "ARMED" : "NOT ARMED");
  display.print("Pulse: "); display.print(TRIGGER_PULSE_MS); display.println("ms");
  display.print("Interval: "); display.print(MIN_TRIGGER_INTERVAL_MS); display.println("ms");
  menuRow("Info", uiSelectedItem() == 0, false);
  menuRow("Back", uiSelectedItem() == 1);
}

static void renderBattery() {
  title("BATTERY");
  display.print("Status: "); display.println(batteryStatusToString(batteryStatus()));
  display.print("Voltage: ");
  if (batterySensorValid()) { display.print(batteryVoltage(), 2); display.println("V"); } else display.println("--");
  display.print("Profile: "); display.println(batteryProfileToString(selectedBatteryProfile()));
  display.print("Minimum: "); display.println(batteryDiagnostics().minimumVoltage, 2);
  display.print("Cal: ");
  display.println(fabsf(batteryCalibrationMultiplier() - BATTERY_DEFAULT_CALIBRATION_MULTIPLIER) > 0.0001F ? "YES" : "DEFAULT");
  menuRow("Info", uiSelectedItem() == 0, false);
  menuRow("Back", uiSelectedItem() == 1);
}

static void renderEnvironment() {
  title("ENVIRONMENT");
  display.print("Sensor: BME280 "); display.println(environmentStatusToString(getEnvironmentStatus()));
  display.print("Temp: "); display.print(getTemperatureF(), 1); display.println(" F");
  display.print("Humidity: "); display.print(getHumidity(), 1); display.println(" %");
  display.print("Pressure: "); display.print(getPressureHpa(), 1); display.println(" hPa");
  menuRow("Info", uiSelectedItem() == 0, false);
  menuRow("Back", uiSelectedItem() == 1);
}

static void renderCalibration() {
  title("CALIBRATION");
  const char* items[] = { "Center platform", "Motion calibrate", "Battery via BLE", "Reset battery cal", "Back" };
  for (uint8_t i = 0; i < 5; i++) menuRow(items[i], i == uiSelectedItem(), i != 2);
  footer("Thrower removed!");
}

static void renderDiagnostics(const ControllerState& state) {
  title("DIAGNOSTICS");
  uint8_t diagPage = uiSelectedItem();
  const MotionDiagnostics& motion = motionDiagnostics();
  if (diagPage == 0) {
    display.print("FW "); display.println(FIRMWARE_VERSION);
    display.print("Up "); display.print(millis() / 1000UL); display.println(" sec");
    display.print("BLE "); display.println(bleClientConnected() ? "connected" : "offline");
    display.print("Fault "); display.println(faultToString(state.lastFault));
  } else if (diagPage == 1) {
    display.print("Sweeps "); display.println(motion.completedSweeps);
    display.print("Endpoints L/R "); display.print(motion.leftEndpointActivations); display.print("/"); display.println(motion.rightEndpointActivations);
    display.print("Reversals "); display.println(motion.directionReversals);
    display.print("Travel ms "); display.println(calibratedFullTravelTimeMs());
  } else {
    display.print("Bat min "); display.println(batteryDiagnostics().minimumVoltage, 2);
    display.print("Bat motor "); display.println(batteryDiagnostics().lowestMotorRunningVoltage, 2);
    display.print("Trig last "); display.println(getLastTriggerTime());
    display.print("Limits "); display.print(leftLimitActive()); display.print("/"); display.println(rightLimitActive());
  }
  footer("Turn:Page Hold:Back");
}

static void renderSettings() {
  title("SETTINGS");
  char contrast[20]; snprintf(contrast, sizeof(contrast), "Contrast: %u", uiDisplayContrast());
  char timeout[20]; snprintf(timeout, sizeof(timeout), "Status: %us", uiStatusTimeoutSeconds());
  const char* items[] = { contrast, timeout, "Save settings", "Restore defaults", "Firmware info", "Back" };
  uint8_t selected = uiSelectedItem();
  uint8_t first = selected > 3 ? 1 : 0;
  for (uint8_t i = first; i < first + 5; i++) menuRow(items[i], i == selected, i != 4);
  footer(uiSettingsDirty() ? "* Unsaved" : "Hold:Back");
}

static void renderEdit() {
  title("EDIT VALUE");
  display.setTextSize(2);
  display.setCursor(25, 22);
  if (uiPage() == UI_PAGE_CALIBRATION && uiSelectedItem() == 2) {
    display.print(uiEditValue() / 10.0F, 1);
    display.println(" V");
  } else display.println(uiEditValue());
  display.setTextSize(1);
  display.setCursor(8, 43);
  display.println("Turn to adjust");
  footer("Press:OK Hold:Cancel");
}

static void renderConfirm() {
  title("CONFIRM ACTION");
  if (uiPage() == UI_PAGE_CALIBRATION && uiSelectedItem() == 2) {
    display.println("Confirm multimeter");
    display.print("value: "); display.print(uiEditValue() / 10.0F, 1); display.println(" V");
    display.println("Motor must be off");
  } else if (uiPage() == UI_PAGE_CALIBRATION) {
    display.println("REMOVE THROWER");
    display.println("CLEAR MOVING AREA");
    display.println("KEEP HANDS AWAY");
  } else {
    display.println("Restore defaults?");
    display.println("Saved UI settings");
    display.println("will be replaced.");
  }
  footer("Press:Confirm Hold:Cancel");
}

static void renderCalibrationProgress() {
  title("CALIBRATION ACTIVE");
  display.println("KEEP AREA CLEAR");
  display.print("Stage: "); display.println(motionPhaseToString());
  display.print("Direction: "); display.println(directionToString(motionCommandedDirection()));
  display.print("Endpoint: "); display.println((int)motionActiveEndpoint());
  display.print("Travel ms: "); display.println(calibratedFullTravelTimeMs());
  footer("STOP available via BLE");
}

static void renderFault(const ControllerState& state) {
  title("!!! FAULT !!!");
  display.println(faultToString(state.lastFault));
  display.println("MOTION: DISABLED");
  display.println("TRIGGER: DISABLED");
  if (leftLimitActive() || rightLimitActive()) display.println("Release limit first");
  else if (emergencyStopActive()) display.println("Release E-stop first");
  else if (!batteryAllowsOperation()) display.println("Battery not recovered");
  else display.println("Inspect before clear");
  footer("Press:Attempt clear");
}

void initDisplay() {
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  displayReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  if (!displayReady) { Serial.println("WARNING: SSD1306 OLED not found at 0x3C"); return; }
  display.clearDisplay();
  display.display();
}

void updateDisplay(const ControllerState& state, bool systemArmed) {
  if (!displayReady) return;
  bool shouldOff = uiDisplayOff() && !state.faultActive && !(UI_STAY_AWAKE_WHILE_RUNNING && state.running);
  if (shouldOff != !displayPowered) {
    display.ssd1306_command(shouldOff ? SSD1306_DISPLAYOFF : SSD1306_DISPLAYON);
    displayPowered = !shouldOff;
  }
  if (!displayPowered) return;

  uint8_t contrast = uiDisplayDimmed() && !state.faultActive ? max((uint8_t)8, (uint8_t)(uiDisplayContrast() / 4)) : uiDisplayContrast();
  if (contrast != lastContrast) {
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(contrast);
    lastContrast = contrast;
  }

  unsigned long now = millis();
  UiState current = uiState();
  if (current == lastRenderedState && now - lastDisplayUpdate < UI_DISPLAY_REFRESH_MS) return;
  lastRenderedState = current;
  lastDisplayUpdate = now;
  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (current == UI_BOOT) renderBoot();
  else if (current == UI_STATUS) renderStatus(state, systemArmed);
  else if (current == UI_MENU) renderTopMenu();
  else if (current == UI_EDIT_VALUE) renderEdit();
  else if (current == UI_CONFIRM) renderConfirm();
  else if (current == UI_NOTIFICATION) { title("NOTICE"); display.println(uiNotificationText()); footer("Press:Dismiss"); }
  else if (current == UI_CALIBRATION) renderCalibrationProgress();
  else if (current == UI_FAULT) renderFault(state);
  else {
    switch (uiPage()) {
      case UI_PAGE_MODE: renderMode(state); break;
      case UI_PAGE_MOTION: renderMotion(state); break;
      case UI_PAGE_TRIGGER: renderTrigger(systemArmed); break;
      case UI_PAGE_BATTERY: renderBattery(); break;
      case UI_PAGE_ENVIRONMENT: renderEnvironment(); break;
      case UI_PAGE_CALIBRATION: renderCalibration(); break;
      case UI_PAGE_DIAGNOSTICS: renderDiagnostics(state); break;
      case UI_PAGE_SETTINGS: renderSettings(); break;
      default: renderStatus(state, systemArmed); break;
    }
  }
  display.display();
}
