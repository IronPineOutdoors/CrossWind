#pragma once

#include "config.h"

void resetModeState();
FaultCode updateMode(ControllerState& state);
const char* sweepStateToString();
