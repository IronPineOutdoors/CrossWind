#pragma once

#include "config.h"

enum MotionState {
  MOTION_IDLE,
  MOTION_STARTING,
  MOTION_MOVING_LEFT,
  MOTION_MOVING_RIGHT,
  MOTION_DECELERATING,
  MOTION_ENDPOINT_REACHED,
  MOTION_PAUSED,
  MOTION_REVERSING,
  MOTION_CENTERING,
  MOTION_STOPPING,
  MOTION_FAULTED
};

enum MotionEndpoint { MOTION_ENDPOINT_NONE, MOTION_ENDPOINT_LEFT, MOTION_ENDPOINT_RIGHT };
enum MotionEvent { MOTION_EVENT_NONE, MOTION_EVENT_ENDPOINT, MOTION_EVENT_TRIGGER_REQUESTED, MOTION_EVENT_CENTERED, MOTION_EVENT_CALIBRATED };
enum CalibrationStatus { CALIBRATION_NOT_RUN, CALIBRATION_RUNNING, CALIBRATION_VALID, CALIBRATION_FAILED };

struct MotionDiagnostics {
  uint32_t leftEndpointActivations;
  uint32_t rightEndpointActivations;
  uint32_t completedSweeps;
  uint32_t directionReversals;
  uint32_t randomMovements;
  uint32_t centeringAttempts;
  uint32_t centeringSuccesses;
  uint32_t calibrationAttempts;
  uint32_t calibrationSuccesses;
  uint32_t calibrationFailures;
  uint32_t motionTimeouts;
  uint32_t stuckLimitFaults;
};

void initMotion(uint32_t storedFullTravelTimeMs);
bool startMotion(ControllerState& controller);
void stopMotion();
void updateMotion(ControllerState& controller);
bool requestCentering(ControllerState& controller);
bool requestCalibration(ControllerState& controller);
MotionState motionState();
Direction motionCommandedDirection();
MotionEndpoint motionActiveEndpoint();
const char* motionPhaseToString();
MotionEvent consumeMotionEvent();
FaultCode consumeMotionFault();
CalibrationStatus motionCalibrationStatus();
uint32_t calibratedFullTravelTimeMs();
bool motionCalibrationNeedsSave();
void markMotionCalibrationSaved();
bool motionIsActive();
bool motionIsApproximatelyCentered();
const MotionDiagnostics& motionDiagnostics();
