#pragma once

#include <Arduino.h>

#include "config.h"

enum EnvironmentStatus {
  ENV_STATUS_READY,
  ENV_STATUS_HOT,
  ENV_STATUS_TEMP_FAULT,
  ENV_STATUS_ERROR
};

void initEnvironment();
void updateEnvironment();
float getTemperatureF();
float getTemperatureC();
float getHumidity();
float getPressureHpa();
bool environmentDataValid();
EnvironmentStatus getEnvironmentStatus();
const char* environmentStatusToString(EnvironmentStatus status);
bool environmentTempFaultActive();
