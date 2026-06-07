#pragma once

#include "config.h"

void beginInputs();
void updateInputs();
bool consumeStartPressed();
bool consumeModePressed();
uint8_t readSpeedPwm();
int readSpeedRaw();
