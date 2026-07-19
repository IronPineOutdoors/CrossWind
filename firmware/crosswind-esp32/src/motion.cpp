#include "motion.h"

#include <esp_system.h>

#include "limits.h"
#include "motor.h"

enum MotionRoutine { ROUTINE_MODE, ROUTINE_CENTER, ROUTINE_CALIBRATE };
enum CenterPhase { CENTER_SEEK_LEFT, CENTER_DWELL, CENTER_MOVE_TO_MIDDLE };
enum CalibrationPhase { CAL_SEEK_LEFT, CAL_DWELL, CAL_MEASURE_RIGHT };

static MotionState currentState = MOTION_IDLE;
static MotionRoutine routine = ROUTINE_MODE;
static CenterPhase centerPhase = CENTER_SEEK_LEFT;
static CalibrationPhase calibrationPhase = CAL_SEEK_LEFT;
static MotionEndpoint activeEndpoint = MOTION_ENDPOINT_NONE;
static MotionEndpoint releasingEndpoint = MOTION_ENDPOINT_NONE;
static Direction commandedDirection = DIR_RIGHT;
static MotionEvent pendingEvent = MOTION_EVENT_NONE;
static FaultCode pendingFault = FAULT_NONE;
static CalibrationStatus calibrationStatus = CALIBRATION_NOT_RUN;
static MotionDiagnostics counters = {};
static unsigned long stateStartedAt = 0;
static unsigned long movementStartedAt = 0;
static unsigned long releaseStartedAt = 0;
static unsigned long calibrationMeasureStartedAt = 0;
static unsigned long plannedMoveDuration = 0;
static unsigned long plannedDwellDuration = 0;
static uint32_t fullTravelTimeMs = 0;
static bool calibrationNeedsSave = false;
static bool approximatelyCentered = false;
static bool flushTriggerSent = false;
static bool randomFullSweep = false;

static bool endpointActive(MotionEndpoint endpoint) {
  if (endpoint == MOTION_ENDPOINT_LEFT) return leftLimitActive();
  if (endpoint == MOTION_ENDPOINT_RIGHT) return rightLimitActive();
  return false;
}

static MotionEndpoint endpointFor(Direction direction) {
  return direction == DIR_LEFT ? MOTION_ENDPOINT_LEFT : MOTION_ENDPOINT_RIGHT;
}

static MotionEndpoint oppositeEndpoint(MotionEndpoint endpoint) {
  return endpoint == MOTION_ENDPOINT_LEFT ? MOTION_ENDPOINT_RIGHT : MOTION_ENDPOINT_LEFT;
}

static Direction directionAwayFrom(MotionEndpoint endpoint) {
  return endpoint == MOTION_ENDPOINT_LEFT ? DIR_RIGHT : DIR_LEFT;
}

static void setState(MotionState next) {
#if CROSSWIND_MOTION_SIMULATION
  if (currentState != next) {
    Serial.print("SIM motion: ");
    Serial.print(motionPhaseToString());
    Serial.print(" -> ");
  }
#endif
  currentState = next;
  stateStartedAt = millis();
#if CROSSWIND_MOTION_SIMULATION
  Serial.println(motionPhaseToString());
#endif
}

static void reportFault(FaultCode fault) {
  stopMotor();
  pendingFault = fault;
  setState(MOTION_FAULTED);
  if (fault == FAULT_TRAVEL_TIMEOUT) counters.motionTimeouts++;
  if (fault == FAULT_LIMIT_STUCK) counters.stuckLimitFaults++;
  if (routine == ROUTINE_CALIBRATE) {
    calibrationStatus = CALIBRATION_FAILED;
    counters.calibrationFailures++;
  }
}

static bool validLimitCombination() {
  if (bothLimitsActive()) {
    reportFault(FAULT_BOTH_LIMITS);
    return false;
  }
  return true;
}

static void beginMove(Direction direction, uint8_t pwm) {
  MotionEndpoint target = endpointFor(direction);
  if (endpointActive(target)) {
    reportFault(FAULT_UNEXPECTED_LIMIT);
    return;
  }
  commandedDirection = direction;
  movementStartedAt = millis();
  if (endpointActive(oppositeEndpoint(target))) {
    releasingEndpoint = oppositeEndpoint(target);
    releaseStartedAt = movementStartedAt;
  } else {
    releasingEndpoint = MOTION_ENDPOINT_NONE;
  }
  driveMotor(direction, pwm);
  setState(direction == DIR_LEFT ? MOTION_MOVING_LEFT : MOTION_MOVING_RIGHT);
}

static bool checkMovementSafety() {
  if (!validLimitCombination()) return false;
  MotionEndpoint target = endpointFor(commandedDirection);
  MotionEndpoint unexpected = oppositeEndpoint(target);

  if (releasingEndpoint != MOTION_ENDPOINT_NONE) {
    if (!endpointActive(releasingEndpoint)) {
      releasingEndpoint = MOTION_ENDPOINT_NONE;
    } else if (millis() - releaseStartedAt >= LIMIT_RELEASE_TIMEOUT_MS) {
      reportFault(FAULT_LIMIT_STUCK);
      return false;
    }
  } else if (endpointActive(unexpected)) {
    reportFault(FAULT_UNEXPECTED_LIMIT);
    return false;
  }

  if (millis() - movementStartedAt >= MAX_TRAVEL_TIME_MS) {
    reportFault(FAULT_TRAVEL_TIMEOUT);
    return false;
  }
  return true;
}

static bool reachedTargetEndpoint() {
  MotionEndpoint target = endpointFor(commandedDirection);
  if (!endpointActive(target)) return false;
  stopMotor();
  activeEndpoint = target;
  approximatelyCentered = false;
  if (target == MOTION_ENDPOINT_LEFT) counters.leftEndpointActivations++;
  else counters.rightEndpointActivations++;
  pendingEvent = MOTION_EVENT_ENDPOINT;
  setState(MOTION_ENDPOINT_REACHED);
  return true;
}

static unsigned long randomBetween(unsigned long minimum, unsigned long maximum) {
  return minimum >= maximum ? minimum : (unsigned long)random((long)minimum, (long)maximum + 1L);
}

static Direction chooseSafeDirection() {
  if (leftLimitActive()) return DIR_RIGHT;
  if (rightLimitActive()) return DIR_LEFT;
  return random(0, 2) == 0 ? DIR_LEFT : DIR_RIGHT;
}

static void beginModeMovement(ControllerState& controller) {
  Direction direction = chooseSafeDirection();
  uint8_t pwm = controller.speed;
  plannedMoveDuration = 0;
  plannedDwellDuration = LIMIT_DWELL_MS;

  if (controller.mode == RANDOM) {
    pwm = (uint8_t)randomBetween(RANDOM_SPEED_MIN_PWM, RANDOM_SPEED_MAX_PWM);
    plannedDwellDuration = randomBetween(ENDPOINT_DWELL_MIN_MS, ENDPOINT_DWELL_MAX_MS);
    plannedMoveDuration = randomBetween(RANDOM_MOVE_MIN_MS, RANDOM_MOVE_MAX_MS);
    randomFullSweep = random(0, 100) < RANDOM_FULL_SWEEP_PERCENT;
    counters.randomMovements++;
  } else if (controller.mode == FLUSH) {
    plannedMoveDuration = FLUSH_MOVE_TIME_MS;
    plannedDwellDuration = FLUSH_PAUSE_TIME_MS;
    flushTriggerSent = false;
  }

  controller.direction = direction;
  beginMove(direction, pwm);
}

static void updateSweepOrRandom(ControllerState& controller) {
  if (currentState == MOTION_STARTING) {
    beginModeMovement(controller);
    return;
  }

  if (currentState == MOTION_MOVING_LEFT || currentState == MOTION_MOVING_RIGHT) {
    if (!checkMovementSafety()) return;
    if (reachedTargetEndpoint()) {
      plannedDwellDuration = controller.mode == RANDOM
        ? randomBetween(ENDPOINT_DWELL_MIN_MS, ENDPOINT_DWELL_MAX_MS)
        : LIMIT_DWELL_MS;
      return;
    }
    if (controller.mode == RANDOM && !randomFullSweep && plannedMoveDuration > 0 && millis() - movementStartedAt >= plannedMoveDuration) {
      stopMotor();
      setState(MOTION_PAUSED);
    }
    return;
  }

  if (currentState == MOTION_ENDPOINT_REACHED) {
    setState(MOTION_PAUSED);
    return;
  }

  if (currentState == MOTION_PAUSED && millis() - stateStartedAt >= plannedDwellDuration) {
    Direction next = activeEndpoint == MOTION_ENDPOINT_NONE ? chooseSafeDirection() : directionAwayFrom(activeEndpoint);
    if (next != commandedDirection) counters.directionReversals++;
    if (activeEndpoint != MOTION_ENDPOINT_NONE) counters.completedSweeps++;
    activeEndpoint = MOTION_ENDPOINT_NONE;
    controller.direction = next;
    setState(MOTION_REVERSING);
    return;
  }

  if (currentState == MOTION_REVERSING) {
    if (controller.mode == RANDOM) beginModeMovement(controller);
    else beginMove(controller.direction, controller.speed);
  }
}

static void updateFlush(ControllerState& controller) {
  if (currentState == MOTION_STARTING) {
    plannedDwellDuration = FLUSH_READY_TIME_MS;
    setState(MOTION_PAUSED);
  } else if (currentState == MOTION_PAUSED && millis() - stateStartedAt >= plannedDwellDuration) {
    beginModeMovement(controller);
  } else if (currentState == MOTION_MOVING_LEFT || currentState == MOTION_MOVING_RIGHT) {
    if (!flushTriggerSent && ENABLE_AUTOMATIC_TRIGGER) {
      pendingEvent = MOTION_EVENT_TRIGGER_REQUESTED;
      flushTriggerSent = true;
    }
    if (!checkMovementSafety()) return;
    if (reachedTargetEndpoint() || millis() - movementStartedAt >= FLUSH_MOVE_TIME_MS) {
      stopMotor();
      plannedDwellDuration = FLUSH_PAUSE_TIME_MS;
      setState(MOTION_PAUSED);
    }
  } else if (currentState == MOTION_ENDPOINT_REACHED) {
    plannedDwellDuration = FLUSH_PAUSE_TIME_MS;
    setState(MOTION_PAUSED);
  }
}

static void updateCentering() {
  if (currentState == MOTION_STARTING) {
    centerPhase = CENTER_SEEK_LEFT;
    if (leftLimitActive()) {
      activeEndpoint = MOTION_ENDPOINT_LEFT;
      centerPhase = CENTER_DWELL;
      setState(MOTION_PAUSED);
    } else {
      beginMove(DIR_LEFT, CENTERING_SPEED_PWM);
    }
    return;
  }
  if (currentState == MOTION_MOVING_LEFT || currentState == MOTION_MOVING_RIGHT) {
    if (!checkMovementSafety()) return;
    if (centerPhase == CENTER_SEEK_LEFT && reachedTargetEndpoint()) {
      if (activeEndpoint != MOTION_ENDPOINT_LEFT) {
        reportFault(FAULT_UNEXPECTED_LIMIT);
        return;
      }
      centerPhase = CENTER_DWELL;
      setState(MOTION_PAUSED);
    } else if (centerPhase == CENTER_MOVE_TO_MIDDLE) {
      if (reachedTargetEndpoint()) {
        reportFault(FAULT_CALIBRATION_FAILED);
      } else {
        uint32_t travel = fullTravelTimeMs > 0 ? fullTravelTimeMs : DEFAULT_FULL_TRAVEL_TIME_MS;
        if (millis() - movementStartedAt >= travel * CENTERING_PERCENT / 100UL) {
          stopMotor();
          approximatelyCentered = true;
          counters.centeringSuccesses++;
          pendingEvent = MOTION_EVENT_CENTERED;
          setState(MOTION_IDLE);
        }
      }
    }
  } else if (centerPhase == CENTER_DWELL && currentState == MOTION_PAUSED && millis() - stateStartedAt >= LIMIT_DWELL_MS) {
    centerPhase = CENTER_MOVE_TO_MIDDLE;
    beginMove(DIR_RIGHT, CENTERING_SPEED_PWM);
    setState(MOTION_CENTERING);
  } else if (currentState == MOTION_CENTERING) {
    setState(MOTION_MOVING_RIGHT);
  }
}

static void updateCalibration() {
  if (currentState == MOTION_STARTING) {
    calibrationPhase = CAL_SEEK_LEFT;
    if (leftLimitActive()) {
      activeEndpoint = MOTION_ENDPOINT_LEFT;
      calibrationPhase = CAL_DWELL;
      setState(MOTION_PAUSED);
    } else {
      beginMove(DIR_LEFT, CALIBRATION_SPEED_PWM);
    }
    return;
  }
  if (currentState == MOTION_MOVING_LEFT || currentState == MOTION_MOVING_RIGHT) {
    if (!checkMovementSafety()) return;
    if (calibrationPhase == CAL_SEEK_LEFT && reachedTargetEndpoint()) {
      if (activeEndpoint != MOTION_ENDPOINT_LEFT) {
        reportFault(FAULT_UNEXPECTED_LIMIT);
        return;
      }
      calibrationPhase = CAL_DWELL;
      setState(MOTION_PAUSED);
    } else if (calibrationPhase == CAL_MEASURE_RIGHT && reachedTargetEndpoint()) {
      uint32_t measured = millis() - calibrationMeasureStartedAt;
      if (activeEndpoint != MOTION_ENDPOINT_RIGHT || measured < MIN_VALID_CALIBRATION_TIME_MS || measured > MAX_VALID_CALIBRATION_TIME_MS) {
        reportFault(FAULT_CALIBRATION_FAILED);
        return;
      }
      fullTravelTimeMs = measured;
      calibrationStatus = CALIBRATION_VALID;
      calibrationNeedsSave = true;
      counters.calibrationSuccesses++;
      pendingEvent = MOTION_EVENT_CALIBRATED;
      setState(MOTION_IDLE);
    }
  } else if (calibrationPhase == CAL_DWELL && currentState == MOTION_PAUSED && millis() - stateStartedAt >= LIMIT_DWELL_MS) {
    calibrationPhase = CAL_MEASURE_RIGHT;
    calibrationMeasureStartedAt = millis();
    beginMove(DIR_RIGHT, CALIBRATION_SPEED_PWM);
  }
}

void initMotion(uint32_t storedFullTravelTimeMs) {
  fullTravelTimeMs = storedFullTravelTimeMs;
  calibrationStatus = fullTravelTimeMs > 0 ? CALIBRATION_VALID : CALIBRATION_NOT_RUN;
  uint32_t seed = MOTION_FIXED_RANDOM_SEED;
  if (seed == 0) seed = CROSSWIND_MOTION_SIMULATION ? 1UL : esp_random();
  randomSeed(seed);
  stopMotion();
}

bool startMotion(ControllerState& controller) {
  if (controller.faultActive || bothLimitsActive()) return false;
  routine = controller.mode == CENTERING ? ROUTINE_CENTER : ROUTINE_MODE;
  if (routine == ROUTINE_CENTER) counters.centeringAttempts++;
  approximatelyCentered = false;
  activeEndpoint = leftLimitActive() ? MOTION_ENDPOINT_LEFT : rightLimitActive() ? MOTION_ENDPOINT_RIGHT : MOTION_ENDPOINT_NONE;
  pendingFault = FAULT_NONE;
  setState(MOTION_STARTING);
  return true;
}

void stopMotion() {
  stopMotor();
  currentState = MOTION_IDLE;
  routine = ROUTINE_MODE;
  releasingEndpoint = MOTION_ENDPOINT_NONE;
  activeEndpoint = MOTION_ENDPOINT_NONE;
  flushTriggerSent = false;
}

void updateMotion(ControllerState& controller) {
  if (!controller.running || controller.faultActive) {
    stopMotion();
    return;
  }
  if (currentState == MOTION_IDLE) {
    if (!startMotion(controller)) reportFault(FAULT_BOTH_LIMITS);
  }
  if (routine == ROUTINE_CENTER) updateCentering();
  else if (routine == ROUTINE_CALIBRATE) updateCalibration();
  else if (controller.mode == FLUSH) updateFlush(controller);
  else updateSweepOrRandom(controller);
}

bool requestCentering(ControllerState& controller) {
  if (controller.running || controller.faultActive || bothLimitsActive()) return false;
  controller.mode = CENTERING;
  controller.running = true;
  return startMotion(controller);
}

bool requestCalibration(ControllerState& controller) {
  if (controller.running || controller.faultActive || bothLimitsActive()) return false;
  routine = ROUTINE_CALIBRATE;
  calibrationStatus = CALIBRATION_RUNNING;
  counters.calibrationAttempts++;
  controller.running = true;
  approximatelyCentered = false;
  setState(MOTION_STARTING);
  return true;
}

MotionState motionState() { return currentState; }
Direction motionCommandedDirection() { return commandedDirection; }
MotionEndpoint motionActiveEndpoint() { return activeEndpoint; }
MotionEvent consumeMotionEvent() { MotionEvent event = pendingEvent; pendingEvent = MOTION_EVENT_NONE; return event; }
FaultCode consumeMotionFault() { FaultCode fault = pendingFault; pendingFault = FAULT_NONE; return fault; }
CalibrationStatus motionCalibrationStatus() { return calibrationStatus; }
uint32_t calibratedFullTravelTimeMs() { return fullTravelTimeMs; }
bool motionCalibrationNeedsSave() { return calibrationNeedsSave; }
void markMotionCalibrationSaved() { calibrationNeedsSave = false; }
bool motionIsActive() { return currentState != MOTION_IDLE && currentState != MOTION_FAULTED; }
bool motionIsApproximatelyCentered() { return approximatelyCentered; }
const MotionDiagnostics& motionDiagnostics() { return counters; }

const char* motionPhaseToString() {
  switch (currentState) {
    case MOTION_IDLE: return "IDLE";
    case MOTION_STARTING: return "STARTING";
    case MOTION_MOVING_LEFT: return "MOVING_LEFT";
    case MOTION_MOVING_RIGHT: return "MOVING_RIGHT";
    case MOTION_DECELERATING: return "DECELERATING";
    case MOTION_ENDPOINT_REACHED: return "ENDPOINT";
    case MOTION_PAUSED: return "PAUSED";
    case MOTION_REVERSING: return "REVERSING";
    case MOTION_CENTERING: return "CENTERING";
    case MOTION_STOPPING: return "STOPPING";
    case MOTION_FAULTED: return "FAULTED";
    default: return "UNKNOWN";
  }
}
