#include "menu.h"

#include <string.h>

#include "motion.h"

enum EditTarget { EDIT_NONE, EDIT_SPEED, EDIT_CONTRAST, EDIT_STATUS_TIMEOUT, EDIT_BATTERY_KNOWN_DECIVOLTS };
enum ConfirmAction { CONFIRM_NONE, CONFIRM_CENTER, CONFIRM_MOTION_CALIBRATION, CONFIRM_BATTERY_CALIBRATION, CONFIRM_BATTERY_RESET, CONFIRM_RESTORE_DEFAULTS };

static UiState state = UI_BOOT;
static UiState previousState = UI_STATUS;
static UiPage page = UI_PAGE_STATUS;
static uint8_t selectedItem = 0;
static int editValue = 0;
static int editOriginal = 0;
static EditTarget editTarget = EDIT_NONE;
static ConfirmAction confirmAction = CONFIRM_NONE;
static UiIntent pendingIntent = { UI_INTENT_NONE, 0, 0 };
static unsigned long stateStartedAt = 0;
static unsigned long lastInteractionAt = 0;
static char notification[22] = "";
static uint8_t contrastSetting = UI_DEFAULT_CONTRAST;
static uint16_t timeoutSettingSeconds = UI_DEFAULT_STATUS_TIMEOUT_SECONDS;
static bool settingsDirty = false;
static int pendingBatteryKnownDecivolts = 0;

static const uint8_t TOP_ITEM_COUNT = 10;

static void changeState(UiState next) {
  previousState = state;
  state = next;
  stateStartedAt = millis();
}

static void queueIntent(UiIntentType type, int value = 0, int secondary = 0) {
  if (pendingIntent.type == UI_INTENT_NONE) pendingIntent = { type, value, secondary };
}

static void returnToStatus() {
  page = UI_PAGE_STATUS;
  selectedItem = 0;
  editTarget = EDIT_NONE;
  confirmAction = CONFIRM_NONE;
  changeState(UI_STATUS);
}

static void openPage(UiPage nextPage) {
  page = nextPage;
  selectedItem = 0;
  changeState(UI_SCREEN);
}

static int wrapValue(int value, int count) {
  while (value < 0) value += count;
  while (value >= count) value -= count;
  return value;
}

static void beginEdit(EditTarget target, int value) {
  editTarget = target;
  editOriginal = editValue = value;
  changeState(UI_EDIT_VALUE);
}

static void beginConfirm(ConfirmAction action) {
  confirmAction = action;
  changeState(UI_CONFIRM);
}

void initMenu(uint8_t contrast, uint16_t statusTimeoutSeconds) {
  contrastSetting = constrain(contrast, UI_MIN_CONTRAST, UI_MAX_CONTRAST);
  timeoutSettingSeconds = constrain(statusTimeoutSeconds, UI_MIN_STATUS_TIMEOUT_SECONDS, UI_MAX_STATUS_TIMEOUT_SECONDS);
  state = UI_BOOT;
  page = UI_PAGE_STATUS;
  stateStartedAt = lastInteractionAt = millis();
  settingsDirty = false;
}

void notifyUiActivity() {
  lastInteractionAt = millis();
}

void updateMenu(const ControllerState& controller) {
  unsigned long now = millis();
  if (controller.faultActive) {
    if (state != UI_FAULT) changeState(UI_FAULT);
    lastInteractionAt = now;
    return;
  }
  if (state == UI_FAULT) returnToStatus();
  bool calibrationActive = motionCalibrationStatus() == CALIBRATION_RUNNING || (controller.running && controller.mode == CENTERING);
  if (calibrationActive && state != UI_CALIBRATION) changeState(UI_CALIBRATION);
  if (!calibrationActive && state == UI_CALIBRATION) returnToStatus();
  if (state == UI_BOOT && now - stateStartedAt >= UI_BOOT_SCREEN_MS) returnToStatus();
  if (state == UI_NOTIFICATION && now - stateStartedAt >= UI_NOTIFICATION_MS) {
    state = previousState == UI_NOTIFICATION ? UI_STATUS : previousState;
    stateStartedAt = now;
  }
  if ((state == UI_MENU || state == UI_SCREEN) && now - lastInteractionAt >= (uint32_t)timeoutSettingSeconds * 1000UL) returnToStatus();
  if (UI_STAY_AWAKE_WHILE_RUNNING && controller.running) lastInteractionAt = now;
}

void handleUiEncoder(int delta, const ControllerState& controller) {
  if (delta == 0 || state == UI_FAULT || state == UI_BOOT || state == UI_CONFIRM || state == UI_CALIBRATION) return;
  notifyUiActivity();
  if (state == UI_STATUS) {
    int next = constrain((int)controller.speed + delta * ROTARY_ENCODER_SPEED_STEP, 0, (int)MAX_PWM);
    queueIntent(UI_INTENT_SET_SPEED, next);
    return;
  }
  if (state == UI_MENU) {
    selectedItem = (uint8_t)wrapValue((int)selectedItem + delta, TOP_ITEM_COUNT);
    return;
  }
  if (state == UI_EDIT_VALUE) {
    int step = editTarget == EDIT_SPEED ? ROTARY_ENCODER_SPEED_STEP
      : editTarget == EDIT_CONTRAST ? UI_CONTRAST_STEP : editTarget == EDIT_STATUS_TIMEOUT ? UI_STATUS_TIMEOUT_STEP_SECONDS : 1;
    int minimum = editTarget == EDIT_SPEED ? 0 : editTarget == EDIT_CONTRAST ? UI_MIN_CONTRAST
      : editTarget == EDIT_STATUS_TIMEOUT ? UI_MIN_STATUS_TIMEOUT_SECONDS : (int)(BATTERY_CALIBRATION_KNOWN_MIN_V * 10.0F);
    int maximum = editTarget == EDIT_SPEED ? MAX_PWM : editTarget == EDIT_CONTRAST ? UI_MAX_CONTRAST
      : editTarget == EDIT_STATUS_TIMEOUT ? UI_MAX_STATUS_TIMEOUT_SECONDS : (int)(BATTERY_CALIBRATION_KNOWN_MAX_V * 10.0F);
    editValue = constrain(editValue + delta * step, minimum, maximum);
    return;
  }
  if (state != UI_SCREEN) return;
  if (page == UI_PAGE_MODE) selectedItem = (uint8_t)wrapValue((int)selectedItem + delta, 5);
  else if (page == UI_PAGE_MOTION) selectedItem = (uint8_t)wrapValue((int)selectedItem + delta, 3);
  else if (page == UI_PAGE_CALIBRATION) selectedItem = (uint8_t)wrapValue((int)selectedItem + delta, 5);
  else if (page == UI_PAGE_DIAGNOSTICS) selectedItem = (uint8_t)wrapValue((int)selectedItem + delta, 3);
  else if (page == UI_PAGE_SETTINGS) selectedItem = (uint8_t)wrapValue((int)selectedItem + delta, 6);
  else selectedItem = (uint8_t)wrapValue((int)selectedItem + delta, 2);
}

void handleUiShortPress(const ControllerState& controller) {
  notifyUiActivity();
  if (state == UI_BOOT) { returnToStatus(); return; }
  if (state == UI_FAULT) { queueIntent(UI_INTENT_CLEAR_FAULT); return; }
  if (state == UI_STATUS) { changeState(UI_MENU); selectedItem = 0; return; }
  if (state == UI_NOTIFICATION) { state = previousState; return; }
  if (state == UI_MENU) {
    if (selectedItem == 0 || selectedItem == 9) returnToStatus();
    else {
      uint8_t topSelection = selectedItem;
      openPage((UiPage)topSelection);
      if (page == UI_PAGE_MODE) selectedItem = (uint8_t)controller.mode;
    }
    return;
  }
  if (state == UI_EDIT_VALUE) {
    if (editTarget == EDIT_SPEED) queueIntent(UI_INTENT_SET_SPEED, editValue, 1);
    else if (editTarget == EDIT_CONTRAST) { contrastSetting = (uint8_t)editValue; settingsDirty = true; }
    else if (editTarget == EDIT_STATUS_TIMEOUT) { timeoutSettingSeconds = (uint16_t)editValue; settingsDirty = true; }
    else if (editTarget == EDIT_BATTERY_KNOWN_DECIVOLTS) {
      pendingBatteryKnownDecivolts = editValue;
      editTarget = EDIT_NONE;
      beginConfirm(CONFIRM_BATTERY_CALIBRATION);
      return;
    }
    editTarget = EDIT_NONE;
    changeState(UI_SCREEN);
    return;
  }
  if (state == UI_CONFIRM) {
    if (confirmAction == CONFIRM_CENTER) queueIntent(UI_INTENT_CENTER);
    else if (confirmAction == CONFIRM_MOTION_CALIBRATION) queueIntent(UI_INTENT_MOTION_CALIBRATE);
    else if (confirmAction == CONFIRM_BATTERY_CALIBRATION) queueIntent(UI_INTENT_BATTERY_CALIBRATE, pendingBatteryKnownDecivolts);
    else if (confirmAction == CONFIRM_BATTERY_RESET) queueIntent(UI_INTENT_RESET_BATTERY_CALIBRATION);
    else if (confirmAction == CONFIRM_RESTORE_DEFAULTS) queueIntent(UI_INTENT_RESTORE_DEFAULTS);
    confirmAction = CONFIRM_NONE;
    changeState(UI_SCREEN);
    return;
  }
  if (state != UI_SCREEN) return;

  if (page == UI_PAGE_MODE) {
    if (selectedItem == 4) { changeState(UI_MENU); selectedItem = 1; }
    else if (controller.running) showUiNotification("STOP TO CHANGE MODE");
    else queueIntent(UI_INTENT_SET_MODE, selectedItem);
  } else if (page == UI_PAGE_MOTION) {
    if (selectedItem == 0) beginEdit(EDIT_SPEED, controller.speed);
    else if (selectedItem == 1) queueIntent(controller.running ? UI_INTENT_STOP : UI_INTENT_START);
    else { changeState(UI_MENU); selectedItem = 2; }
  } else if (page == UI_PAGE_CALIBRATION) {
    if (selectedItem == 0) beginConfirm(CONFIRM_CENTER);
    else if (selectedItem == 1) beginConfirm(CONFIRM_MOTION_CALIBRATION);
    else if (selectedItem == 2) beginEdit(EDIT_BATTERY_KNOWN_DECIVOLTS, 200);
    else if (selectedItem == 3) beginConfirm(CONFIRM_BATTERY_RESET);
    else { changeState(UI_MENU); selectedItem = 6; }
  } else if (page == UI_PAGE_SETTINGS) {
    if (selectedItem == 0) beginEdit(EDIT_CONTRAST, contrastSetting);
    else if (selectedItem == 1) beginEdit(EDIT_STATUS_TIMEOUT, timeoutSettingSeconds);
    else if (selectedItem == 2) { queueIntent(UI_INTENT_SAVE_SETTINGS, contrastSetting, timeoutSettingSeconds); settingsDirty = false; }
    else if (selectedItem == 3) beginConfirm(CONFIRM_RESTORE_DEFAULTS);
    else if (selectedItem == 4) showUiNotification(FIRMWARE_VERSION);
    else if (selectedItem == 5) { changeState(UI_MENU); selectedItem = 8; }
  } else if (page != UI_PAGE_DIAGNOSTICS && selectedItem == 1) {
    changeState(UI_MENU);
    selectedItem = (uint8_t)page;
  }
}

void handleUiLongPress(const ControllerState& controller) {
  (void)controller;
  notifyUiActivity();
  if (state == UI_EDIT_VALUE) {
    editValue = editOriginal;
    editTarget = EDIT_NONE;
    changeState(UI_SCREEN);
    showUiNotification("EDIT CANCELED");
  } else if (state == UI_MENU || state == UI_SCREEN || state == UI_CONFIRM || state == UI_NOTIFICATION) {
    returnToStatus();
  }
}

void showUiNotification(const char* text) {
  strncpy(notification, text ? text : "", sizeof(notification) - 1);
  notification[sizeof(notification) - 1] = '\0';
  changeState(UI_NOTIFICATION);
  lastInteractionAt = millis();
}

UiIntent consumeUiIntent() { UiIntent result = pendingIntent; pendingIntent = { UI_INTENT_NONE, 0, 0 }; return result; }
UiState uiState() { return state; }
UiPage uiPage() { return page; }
uint8_t uiSelectedItem() { return selectedItem; }
int uiEditValue() { return editValue; }
bool uiSettingsDirty() { return settingsDirty; }
const char* uiNotificationText() { return notification; }
uint8_t uiDisplayContrast() { return state == UI_EDIT_VALUE && editTarget == EDIT_CONTRAST ? (uint8_t)editValue : contrastSetting; }
uint16_t uiStatusTimeoutSeconds() { return timeoutSettingSeconds; }
bool uiDisplayDimmed() { unsigned long idle = millis() - lastInteractionAt; return idle >= UI_DIM_TIMEOUT_MS && idle < UI_OFF_TIMEOUT_MS; }
bool uiDisplayOff() { return millis() - lastInteractionAt >= UI_OFF_TIMEOUT_MS; }
bool uiSimulationEnabled() { return CROSSWIND_UI_SIMULATION != 0; }
const char* uiStateToString() {
  switch (state) {
    case UI_BOOT: return "BOOT"; case UI_STATUS: return "STATUS"; case UI_MENU: return "MENU";
    case UI_SCREEN: return "SCREEN"; case UI_EDIT_VALUE: return "EDIT"; case UI_CONFIRM: return "CONFIRM";
    case UI_NOTIFICATION: return "NOTICE"; case UI_CALIBRATION: return "CALIBRATION"; case UI_FAULT: return "FAULT";
    default: return "UNKNOWN";
  }
}
const char* uiPageToString() {
  switch (page) {
    case UI_PAGE_STATUS: return "STATUS"; case UI_PAGE_MODE: return "MODE"; case UI_PAGE_MOTION: return "MOTION";
    case UI_PAGE_TRIGGER: return "TRIGGER"; case UI_PAGE_BATTERY: return "BATTERY"; case UI_PAGE_ENVIRONMENT: return "ENVIRONMENT";
    case UI_PAGE_CALIBRATION: return "CALIBRATION"; case UI_PAGE_DIAGNOSTICS: return "DIAGNOSTICS"; case UI_PAGE_SETTINGS: return "SETTINGS";
    default: return "UNKNOWN";
  }
}
void simulateUiEncoder(int delta) { if (uiSimulationEnabled()) handleUiEncoder(delta, ControllerState{}); }
void simulateUiShortPress() { if (uiSimulationEnabled()) handleUiShortPress(ControllerState{}); }
void simulateUiLongPress() { if (uiSimulationEnabled()) handleUiLongPress(ControllerState{}); }
