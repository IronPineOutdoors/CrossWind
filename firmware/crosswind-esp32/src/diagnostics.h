#pragma once

#include "config.h"

const char* modeToString(Mode mode);
const char* directionToString(Direction dir);
const char* faultToString(FaultCode fault);
void printStartupDiagnostics(const ControllerState& state);
void printRuntimeStatus(const ControllerState& state);
String buildStatusPayload(const ControllerState& state);
