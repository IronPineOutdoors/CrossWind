#pragma once

#include "config.h"

void beginInputs();
void updateInputs();
bool consumeStartPressed();
bool consumeModePressed();
bool consumeFirePressed();
bool consumeArmPressed();
bool consumeMenuPressed();
int8_t consumeEncoderStep();
void setEncoderMenuActive(bool active);
bool emergencyStopActive();
uint8_t readSpeedPwm();
int readSpeedRaw();
