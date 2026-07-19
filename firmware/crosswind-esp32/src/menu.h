#pragma once

#include "config.h"

enum UiState { UI_BOOT, UI_STATUS, UI_MENU, UI_SCREEN, UI_EDIT_VALUE, UI_CONFIRM, UI_NOTIFICATION, UI_CALIBRATION, UI_FAULT };
enum UiPage { UI_PAGE_STATUS, UI_PAGE_MODE, UI_PAGE_MOTION, UI_PAGE_TRIGGER, UI_PAGE_BATTERY, UI_PAGE_ENVIRONMENT, UI_PAGE_CALIBRATION, UI_PAGE_DIAGNOSTICS, UI_PAGE_SETTINGS };
enum UiIntentType {
  UI_INTENT_NONE,
  UI_INTENT_SET_MODE,
  UI_INTENT_SET_SPEED,
  UI_INTENT_START,
  UI_INTENT_STOP,
  UI_INTENT_CENTER,
  UI_INTENT_MOTION_CALIBRATE,
  UI_INTENT_CLEAR_FAULT,
  UI_INTENT_BATTERY_CALIBRATE,
  UI_INTENT_RESET_BATTERY_CALIBRATION,
  UI_INTENT_SAVE_SETTINGS,
  UI_INTENT_RESTORE_DEFAULTS
};

struct UiIntent {
  UiIntentType type;
  int value;
  int secondaryValue;
};

void initMenu(uint8_t contrast, uint16_t statusTimeoutSeconds);
void updateMenu(const ControllerState& controller);
void handleUiEncoder(int delta, const ControllerState& controller);
void handleUiShortPress(const ControllerState& controller);
void handleUiLongPress(const ControllerState& controller);
void notifyUiActivity();
void showUiNotification(const char* text);
UiIntent consumeUiIntent();
UiState uiState();
UiPage uiPage();
uint8_t uiSelectedItem();
int uiEditValue();
bool uiSettingsDirty();
const char* uiNotificationText();
uint8_t uiDisplayContrast();
uint16_t uiStatusTimeoutSeconds();
bool uiDisplayDimmed();
bool uiDisplayOff();
bool uiSimulationEnabled();
const char* uiStateToString();
const char* uiPageToString();
void simulateUiEncoder(int delta);
void simulateUiShortPress();
void simulateUiLongPress();
