#pragma once

#include "config.h"

typedef bool (*BleCommandHandler)(const String& command, const String& value);
typedef void (*BleDisconnectHandler)();

void beginBle(BleCommandHandler handler, BleDisconnectHandler disconnectHandler);
void updateBleStatus(const ControllerState& state);
void sendBleResponse(const String& status, const String& message);
