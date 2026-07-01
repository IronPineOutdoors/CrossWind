#include "environment.h"

#include <DHT.h>

static DHT dht(DHT_PIN, DHT_TYPE);
static float lastTemperatureC = NAN;
static float lastHumidity = NAN;
static bool hasValidReading = false;
static bool lastReadFailed = false;
static unsigned long lastReadAttempt = 0;

static float cToF(float tempC) {
  return tempC * 9.0F / 5.0F + 32.0F;
}

void initEnvironment() {
  if (ENV_SENSOR_TYPE == ENV_SENSOR_DHT11) {
    dht.begin();
    Serial.println("Environment sensor: DHT11 on GPIO26");
    return;
  }

  // Future Beta hook:
  // Replace this branch with an Adafruit_BME280 instance on Wire using
  // BME280_I2C_ADDRESS_PRIMARY (0x76), then BME280_I2C_ADDRESS_SECONDARY
  // (0x77) as fallback. Keep the public functions in this module unchanged
  // so the rest of the controller does not care which sensor is installed.
  Serial.println("Environment sensor: BME280 hook selected, driver not enabled in Alpha firmware");
}

void updateEnvironment() {
  unsigned long now = millis();
  if (now - lastReadAttempt < ENV_UPDATE_INTERVAL_MS) {
    return;
  }
  lastReadAttempt = now;

  if (ENV_SENSOR_TYPE != ENV_SENSOR_DHT11) {
    return;
  }

  float humidity = dht.readHumidity();
  float temperatureC = dht.readTemperature();

  if (isnan(humidity) || isnan(temperatureC)) {
    lastReadFailed = true;
    Serial.println("WARNING: DHT11 environment read failed; keeping last valid reading");
    return;
  }

  lastHumidity = humidity;
  lastTemperatureC = temperatureC;
  hasValidReading = true;
  lastReadFailed = false;
}

float getTemperatureF() {
  return hasValidReading ? cToF(lastTemperatureC) : NAN;
}

float getTemperatureC() {
  return hasValidReading ? lastTemperatureC : NAN;
}

float getHumidity() {
  return hasValidReading ? lastHumidity : NAN;
}

bool environmentDataValid() {
  return hasValidReading;
}

EnvironmentStatus getEnvironmentStatus() {
  if (!hasValidReading) {
    return ENV_STATUS_ERROR;
  }

  float temperatureF = getTemperatureF();
  if (ENABLE_TEMP_FAULTS && temperatureF >= TEMP_FAULT_F) {
    return ENV_STATUS_TEMP_FAULT;
  }
  if (lastReadFailed) {
    return ENV_STATUS_ERROR;
  }
  if (temperatureF >= TEMP_WARNING_F) {
    return ENV_STATUS_HOT;
  }
  return ENV_STATUS_READY;
}

const char* environmentStatusToString(EnvironmentStatus status) {
  switch (status) {
    case ENV_STATUS_READY: return "READY";
    case ENV_STATUS_HOT: return "HOT";
    case ENV_STATUS_TEMP_FAULT: return "TEMP FAULT";
    case ENV_STATUS_ERROR: return "ENV ERR";
    default: return "ENV ERR";
  }
}

bool environmentTempFaultActive() {
  return getEnvironmentStatus() == ENV_STATUS_TEMP_FAULT;
}
