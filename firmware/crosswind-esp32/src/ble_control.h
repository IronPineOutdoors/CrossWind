#pragma once

#include "config.h"

typedef bool (*BleCommandHandler)(const String& command, const String& value);

void beginBle(BleCommandHandler handler);
void updateBleStatus(const ControllerState& state);
void sendBleResponse(const String& status, const String& message);
bool bleClientConnected();
