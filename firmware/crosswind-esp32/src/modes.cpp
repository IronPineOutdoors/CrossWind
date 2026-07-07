#include "modes.h"

#include "motor.h"

enum SweepState { SWEEP_IDLE, SWEEP_RUNNING };

static SweepState sweepState = SWEEP_IDLE;

void resetModeState() {
  sweepState = SWEEP_IDLE;
  stopMotor();
}

const char* sweepStateToString() {
  switch (sweepState) {
    case SWEEP_IDLE: return "IDLE";
    case SWEEP_RUNNING: return "RUN";
    default: return "UNKNOWN";
  }
}

static void updateSweep(ControllerState& state) {
  if (!state.running || state.faultActive) {
    resetModeState();
    return;
  }

  sweepState = SWEEP_RUNNING;
  driveMotor(state.direction, state.speed);
}

void updateMode(ControllerState& state) {
  switch (state.mode) {
    case SWEEP:
      updateSweep(state);
      break;
    case RANDOM:
    case FLUSH:
      // Future automatic trigger hooks live here. Keep disabled unless
      // ENABLE_AUTOMATIC_TRIGGER is intentionally enabled and safety-tested.
      (void)ENABLE_AUTOMATIC_TRIGGER;
      updateSweep(state);
      break;
    case CENTERING:
      // CENTERING should never trigger the thrower. Phase 1 keeps safe sweep behavior.
      updateSweep(state);
      break;
  }
}
