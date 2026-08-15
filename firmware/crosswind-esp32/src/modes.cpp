#include "modes.h"

#include "limits.h"
#include "motor.h"

enum SweepState { SWEEP_IDLE, SWEEP_TRAVEL, SWEEP_REVERSING, SWEEP_RELEASING };
enum CenterState { CENTER_IDLE, CENTER_FIND_END, CENTER_MEASURE, CENTER_RETURN, CENTER_DONE };

static SweepState sweepState = SWEEP_IDLE;
static CenterState centerState = CENTER_IDLE;
static unsigned long stateStartedAt = 0;
static unsigned long travelStartedAt = 0;
static unsigned long measuredTravelMs = 0;

static Direction awayFromActiveLimit(Direction fallback) {
  if (leftLimitActive() && !rightLimitActive()) return DIR_RIGHT;
  if (rightLimitActive() && !leftLimitActive()) return DIR_LEFT;
  return fallback;
}

static bool limitInDirection(Direction direction) {
  return direction == DIR_LEFT ? leftLimitActive() : rightLimitActive();
}

static Direction opposite(Direction direction) {
  return direction == DIR_LEFT ? DIR_RIGHT : DIR_LEFT;
}

void resetModeState() {
  sweepState = SWEEP_IDLE;
  centerState = CENTER_IDLE;
  stateStartedAt = 0;
  travelStartedAt = 0;
  measuredTravelMs = 0;
  stopMotor();
}

const char* sweepStateToString() {
  if (centerState != CENTER_IDLE) {
    switch (centerState) {
      case CENTER_FIND_END: return "CENTER_FIND";
      case CENTER_MEASURE: return "CENTER_MEASURE";
      case CENTER_RETURN: return "CENTER_RETURN";
      case CENTER_DONE: return "CENTER_DONE";
      default: break;
    }
  }
  switch (sweepState) {
    case SWEEP_IDLE: return "IDLE";
    case SWEEP_TRAVEL: return "TRAVEL";
    case SWEEP_REVERSING: return "REVERSE";
    case SWEEP_RELEASING: return "RELEASE";
    default: return "UNKNOWN";
  }
}

static FaultCode updateBoundedSweep(ControllerState& state) {
  unsigned long now = millis();
  if (sweepState == SWEEP_IDLE) {
    state.direction = awayFromActiveLimit(state.direction);
    sweepState = (leftLimitActive() || rightLimitActive()) ? SWEEP_RELEASING : SWEEP_TRAVEL;
    stateStartedAt = now;
  }

  if (sweepState == SWEEP_TRAVEL) {
    if (limitInDirection(state.direction)) {
      stopMotor();
      state.direction = opposite(state.direction);
      sweepState = SWEEP_REVERSING;
      stateStartedAt = now;
      return FAULT_NONE;
    }
    if (now - stateStartedAt >= MAX_TRAVEL_TIME_MS) return FAULT_TRAVEL_TIMEOUT;
    driveMotor(state.direction, state.speed);
    return FAULT_NONE;
  }

  if (sweepState == SWEEP_REVERSING) {
    stopMotor();
    if (now - stateStartedAt < DIRECTION_CHANGE_DEADTIME_MS) return FAULT_NONE;
    sweepState = SWEEP_RELEASING;
    stateStartedAt = now;
  }

  driveMotor(state.direction, state.speed);
  if (!leftLimitActive() && !rightLimitActive()) {
    sweepState = SWEEP_TRAVEL;
    stateStartedAt = now;
  } else if (now - stateStartedAt >= LIMIT_DWELL_MS) {
    return FAULT_LIMIT;
  }
  return FAULT_NONE;
}

static FaultCode updateCentering(ControllerState& state) {
  unsigned long now = millis();
  if (centerState == CENTER_IDLE) {
    state.direction = awayFromActiveLimit(state.direction);
    centerState = (leftLimitActive() || rightLimitActive()) ? CENTER_MEASURE : CENTER_FIND_END;
    stateStartedAt = now;
  }

  if (centerState == CENTER_FIND_END) {
    if (limitInDirection(state.direction)) {
      stopMotor();
      state.direction = opposite(state.direction);
      centerState = CENTER_MEASURE;
      stateStartedAt = now;
      return FAULT_NONE;
    }
    if (now - stateStartedAt >= MAX_TRAVEL_TIME_MS) return FAULT_TRAVEL_TIMEOUT;
    driveMotor(state.direction, state.speed);
    return FAULT_NONE;
  }

  if (centerState == CENTER_MEASURE) {
    if (now - stateStartedAt < DIRECTION_CHANGE_DEADTIME_MS) {
      stopMotor();
      return FAULT_NONE;
    }
    driveMotor(state.direction, state.speed);
    if (!limitInDirection(opposite(state.direction)) && travelStartedAt == 0) travelStartedAt = now;
    if (limitInDirection(state.direction)) {
      if (travelStartedAt == 0) return FAULT_LIMIT;
      measuredTravelMs = now - travelStartedAt;
      stopMotor();
      state.direction = opposite(state.direction);
      centerState = CENTER_RETURN;
      stateStartedAt = now;
      return FAULT_NONE;
    }
    if (travelStartedAt != 0 && now - travelStartedAt >= MAX_TRAVEL_TIME_MS) return FAULT_TRAVEL_TIMEOUT;
    if (travelStartedAt == 0 && now - stateStartedAt >= LIMIT_DWELL_MS) return FAULT_LIMIT;
    return FAULT_NONE;
  }

  if (centerState == CENTER_RETURN) {
    if (now - stateStartedAt < DIRECTION_CHANGE_DEADTIME_MS) {
      stopMotor();
      return FAULT_NONE;
    }
    if (limitInDirection(opposite(state.direction)) &&
        now - stateStartedAt >= DIRECTION_CHANGE_DEADTIME_MS + LIMIT_DWELL_MS) {
      return FAULT_LIMIT;
    }
    if (now - stateStartedAt >= DIRECTION_CHANGE_DEADTIME_MS + measuredTravelMs / 2) {
      stopMotor();
      state.running = false;
      centerState = CENTER_DONE;
      return FAULT_NONE;
    }
    driveMotor(state.direction, state.speed);
  }
  return FAULT_NONE;
}

FaultCode updateMode(ControllerState& state) {
  if (!state.running || state.faultActive) {
    resetModeState();
    return FAULT_NONE;
  }
  if (bothLimitsActive()) return FAULT_BOTH_LIMITS;

  switch (state.mode) {
    case CENTERING: return updateCentering(state);
    case SWEEP:
    case RANDOM:
    case FLUSH: return updateBoundedSweep(state);
    default: return FAULT_UNKNOWN;
  }
}
