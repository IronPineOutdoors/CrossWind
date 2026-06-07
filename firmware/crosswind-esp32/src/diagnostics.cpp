#include "diagnostics.h"

#include "inputs.h"
#include "limits.h"
#include "motor.h"
#include "trigger.h"

const char* modeToString(Mode mode) {
  switch (mode) {
    case SWEEP: return "SWEEP";
    case RANDOM: return "RANDOM";
    case FLUSH: return "FLUSH";
    case CENTERING: return "CENTERING";
    default: return "UNKNOWN";
  }
}

const char* directionToString(Direction dir) {
  return dir == DIR_RIGHT ? "RIGHT" : "LEFT";
}

const char* faultToString(FaultCode fault) {
  switch (fault) {
    case FAULT_NONE: return "NONE";
    case FAULT_TRAVEL_TIMEOUT: return "TRAVEL_TIMEOUT";
    case FAULT_BOTH_LIMITS: return "BOTH_LIMITS";
    case FAULT_BUTTON_STUCK: return "BUTTON_STUCK";
    case FAULT_STARTUP_BOTH_LIMITS: return "STARTUP_BOTH_LIMITS";
    default: return "UNKNOWN";
  }
}

void printStartupDiagnostics(const ControllerState& state) {
  Serial.println();
  Serial.println("Crosswind ESP32 startup diagnostics");
  Serial.print("  Firmware: ");
  Serial.println(FIRMWARE_VERSION);
  Serial.print("  Mode: ");
  Serial.println(modeToString(state.mode));
  Serial.print("  Left limit active: ");
  Serial.println(leftLimitActive() ? "YES" : "NO");
  Serial.print("  Right limit active: ");
  Serial.println(rightLimitActive() ? "YES" : "NO");
  Serial.print("  Pot raw: ");
  Serial.println(readSpeedRaw());
  Serial.print("  Current speed PWM: ");
  Serial.println(state.speed);
  Serial.print("  Last stored fault: ");
  Serial.println(faultToString(state.lastFault));
  Serial.print("  Thrower trigger enabled: ");
  Serial.println(ENABLE_THROWER_TRIGGER ? "YES" : "NO");
  Serial.print("  Trigger active: ");
  Serial.println(isTriggerActive() ? "YES" : "NO");
  Serial.println();
}

void printRuntimeStatus(const ControllerState& state) {
  static unsigned long lastPrint = 0;
  unsigned long now = millis();
  if (now - lastPrint < SERIAL_STATUS_INTERVAL_MS) {
    return;
  }
  lastPrint = now;

  Serial.println(buildStatusPayload(state));
}

String buildStatusPayload(const ControllerState& state) {
  String payload = "fw=" + String(FIRMWARE_VERSION);
  payload += ";mode=" + String(modeToString(state.mode));
  payload += ";running=" + String(state.running ? "1" : "0");
  payload += ";fault=" + String(state.faultActive ? faultToString(state.lastFault) : "NONE");
  payload += ";dir=" + String(directionToString(state.direction));
  payload += ";speed=" + String(state.speed);
  payload += ";motorPwm=" + String(motorAppliedPwm());
  payload += ";leftLimit=" + String(leftLimitActive() ? "1" : "0");
  payload += ";rightLimit=" + String(rightLimitActive() ? "1" : "0");
  payload += ";potRaw=" + String(readSpeedRaw());
  payload += ";triggerEnabled=" + String(ENABLE_THROWER_TRIGGER ? "1" : "0");
  payload += ";triggerActive=" + String(isTriggerActive() ? "1" : "0");
  payload += ";lastTriggerMs=" + String(getLastTriggerTime());
  return payload;
}
