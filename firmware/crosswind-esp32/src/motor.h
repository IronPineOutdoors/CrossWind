#pragma once

#include "config.h"

void beginMotor();
void driveMotor(Direction dir, uint8_t pwm);
void stopMotor();
void updateMotorRamp();
uint8_t motorAppliedPwm();
Direction motorDirection();
