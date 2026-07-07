#pragma once

#include <Arduino.h>

void initStatusLed();
void setStatusLed(uint8_t r, uint8_t g, uint8_t b);
void setStatusOff();
void showReadyStatus();
void showArmedStatus();
void showFaultStatus();
void showWarningStatus();
void updateStatusLed();
