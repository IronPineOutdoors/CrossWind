#pragma once

#include "config.h"

bool beginMotor();
void driveMotor(Direction dir, uint8_t pwm);
void stopMotor();
void updateMotorRamp();
uint8_t motorAppliedPwm();
Direction motorDirection();
bool motorPwmReady();
uint8_t motorRightPwm();
uint8_t motorLeftPwm();
bool motorRightEnable();
bool motorLeftEnable();
