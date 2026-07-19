#pragma once

#include "config.h"

void beginInputs();
void updateInputs();
bool consumeStartPressed();
bool consumeModePressed();
bool consumeFirePressed();
bool consumeArmPressed();
bool consumeMenuPressed();
bool consumeMenuLongPressed();
int consumeEncoderDelta();
bool emergencyStopActive();
uint8_t readSpeedPwm();
int readSpeedRaw();
void setSpeedPwm(uint8_t pwm);
