#pragma once

#include "config.h"

void resetModeState();
void updateMode(ControllerState& state);
const char* sweepStateToString();
