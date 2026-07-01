#pragma once

#include "config.h"

void beginInputs();
void updateInputs();
bool consumeStartPressed();
bool consumeModePressed();
bool consumeFirePressed();
bool consumeArmPressed();
bool consumeMenuPressed();
uint8_t readSpeedPwm();
int readSpeedRaw();
