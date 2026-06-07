#include "modes.h"

#include "limits.h"
#include "motor.h"

enum SweepState { SWEEP_IDLE, SWEEP_MOVING_RIGHT, SWEEP_DWELL_RIGHT, SWEEP_MOVING_LEFT, SWEEP_DWELL_LEFT };

static SweepState sweepState = SWEEP_IDLE;
static unsigned long motionStart = 0;
static unsigned long dwellStart = 0;

static bool travelTimedOut() {
  return motionStart > 0 && millis() - motionStart >= MAX_TRAVEL_TIME_MS;
}

static void beginMotion(ControllerState& state, Direction dir) {
  state.direction = dir;
  motionStart = millis();
  driveMotor(dir, state.speed);
}

void resetModeState() {
  sweepState = SWEEP_IDLE;
  motionStart = 0;
  dwellStart = 0;
  stopMotor();
}

static void fault(ControllerState& state, FaultCode code) {
  stopMotor();
  state.running = false;
  state.faultActive = true;
  state.lastFault = code;
  resetModeState();
}

static void updateSweep(ControllerState& state) {
  if (!state.running || state.faultActive) {
    resetModeState();
    return;
  }

  if (bothLimitsActive()) {
    fault(state, FAULT_BOTH_LIMITS);
    return;
  }

  unsigned long now = millis();
  switch (sweepState) {
    case SWEEP_IDLE:
      if (rightLimitActive() && !leftLimitActive()) {
        sweepState = SWEEP_DWELL_RIGHT;
        dwellStart = now;
        stopMotor();
      } else if (leftLimitActive() && !rightLimitActive()) {
        sweepState = SWEEP_DWELL_LEFT;
        dwellStart = now;
        stopMotor();
      } else {
        sweepState = state.direction == DIR_LEFT ? SWEEP_MOVING_LEFT : SWEEP_MOVING_RIGHT;
        beginMotion(state, state.direction);
      }
      break;

    case SWEEP_MOVING_RIGHT:
      if (rightLimitActive()) {
        stopMotor();
        state.direction = DIR_LEFT;
        motionStart = 0;
        dwellStart = now;
        sweepState = SWEEP_DWELL_RIGHT;
      } else if (travelTimedOut()) {
        fault(state, FAULT_TRAVEL_TIMEOUT);
      } else {
        driveMotor(DIR_RIGHT, state.speed);
      }
      break;

    case SWEEP_DWELL_RIGHT:
      stopMotor();
      if (now - dwellStart >= LIMIT_DWELL_MS) {
        sweepState = SWEEP_MOVING_LEFT;
        beginMotion(state, DIR_LEFT);
      }
      break;

    case SWEEP_MOVING_LEFT:
      if (leftLimitActive()) {
        stopMotor();
        state.direction = DIR_RIGHT;
        motionStart = 0;
        dwellStart = now;
        sweepState = SWEEP_DWELL_LEFT;
      } else if (travelTimedOut()) {
        fault(state, FAULT_TRAVEL_TIMEOUT);
      } else {
        driveMotor(DIR_LEFT, state.speed);
      }
      break;

    case SWEEP_DWELL_LEFT:
      stopMotor();
      if (now - dwellStart >= LIMIT_DWELL_MS) {
        sweepState = SWEEP_MOVING_RIGHT;
        beginMotion(state, DIR_RIGHT);
      }
      break;
  }
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
