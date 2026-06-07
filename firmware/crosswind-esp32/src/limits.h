#pragma once

#if defined(__GNUC__)
#include_next <limits.h>
#endif

#ifdef __cplusplus
extern "C++" {
#endif

void beginLimits();
void updateLimits();
bool leftLimitActive();
bool rightLimitActive();
bool bothLimitsActive();

#ifdef __cplusplus
}
#endif
