#include "trigger.h"

static bool triggerActive = false;
static unsigned long triggerStartedAt = 0;
static unsigned long lastTriggerTime = 0;

static int inactiveState() {
  return TRIGGER_ACTIVE_STATE == HIGH ? LOW : HIGH;
}

static void writeTriggerRelay(bool active) {
  digitalWrite(THROWER_TRIGGER_PIN, active ? TRIGGER_ACTIVE_STATE : inactiveState());
}

void initTrigger() {
  pinMode(THROWER_TRIGGER_PIN, OUTPUT);
  triggerActive = false;
  writeTriggerRelay(false);
}

bool requestThrowerTrigger() {
  if (!ENABLE_THROWER_TRIGGER) {
    Serial.println("Trigger blocked: trigger feature disabled");
    return false;
  }

  unsigned long now = millis();
  if (triggerActive) {
    Serial.println("Trigger blocked: relay already active");
    return false;
  }

  if (lastTriggerTime > 0 && now - lastTriggerTime < MIN_TRIGGER_INTERVAL_MS) {
    Serial.println("Trigger blocked: minimum trigger interval");
    return false;
  }

  triggerActive = true;
  triggerStartedAt = now;
  lastTriggerTime = now;
  writeTriggerRelay(true);
  Serial.println("Trigger requested: relay active");
  return true;
}

void updateTrigger() {
  if (!triggerActive) {
    return;
  }

  if (millis() - triggerStartedAt >= TRIGGER_PULSE_MS) {
    triggerActive = false;
    writeTriggerRelay(false);
    Serial.println("Trigger complete: relay off");
  }
}

void cancelThrowerTrigger() {
  if (triggerActive) {
    Serial.println("Trigger cancelled: relay off");
  }
  triggerActive = false;
  writeTriggerRelay(false);
}

bool isTriggerActive() {
  return triggerActive;
}

unsigned long getLastTriggerTime() {
  return lastTriggerTime;
}
