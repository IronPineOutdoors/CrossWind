#include "modes.h"

#include "motion.h"

void resetModeState() {
  stopMotion();
}

const char* sweepStateToString() {
  return motionPhaseToString();
}

void updateMode(ControllerState& state) {
  updateMotion(state);
}
