#pragma once

#include "config.h"

void initTrigger();
bool requestThrowerTrigger();
void updateTrigger();
void cancelThrowerTrigger();
bool isTriggerActive();
unsigned long getLastTriggerTime();
