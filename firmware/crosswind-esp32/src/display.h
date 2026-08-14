#pragma once

#include "config.h"

enum MenuPage : uint8_t {
  MENU_CLOSED,
  MENU_ROOT,
  MENU_MOTOR,
  MENU_MODE,
  MENU_ENVIRONMENT,
  MENU_DIAGNOSTICS,
  MENU_ABOUT
};

void initDisplay();
void updateDisplay(const ControllerState& state, bool systemArmed, MenuPage menuPage,
                   uint8_t menuSelection, bool menuEditing);
