#pragma once

#include "config.h"

void initDisplay();
void updateDisplay(const ControllerState& state, bool systemArmed, bool menuActive, uint8_t menuSelection);
