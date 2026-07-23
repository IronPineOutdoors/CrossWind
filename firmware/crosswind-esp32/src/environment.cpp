#include "environment.h"

#include <Adafruit_BME280.h>
#include <Wire.h>

static Adafruit_BME280 bme;
static float lastTemperatureC = NAN;
static float lastHumidity = NAN;
static float lastPressureHpa = NAN;
static bool hasValidReading = false;
static bool lastReadFailed = false;
static bool bmeReady = false;
static unsigned long lastReadAttempt = 0;
static unsigned long lastInitAttempt = 0;
static constexpr uint16_t ENV_INIT_RETRY_INTERVAL_MS = 2000;
static constexpr float BME280_MIN_TEMPERATURE_C = -40.0F;
static constexpr float BME280_MAX_TEMPERATURE_C = 85.0F;
static constexpr float BME280_MIN_HUMIDITY_PERCENT = 0.0F;
static constexpr float BME280_MAX_HUMIDITY_PERCENT = 100.0F;
static constexpr float BME280_MIN_PRESSURE_HPA = 300.0F;
static constexpr float BME280_MAX_PRESSURE_HPA = 1100.0F;

static float cToF(float tempC) {
  return tempC * 9.0F / 5.0F + 32.0F;
}

static bool readingIsPlausible(float temperatureC, float humidity, float pressureHpa) {
  return isfinite(temperatureC) && isfinite(humidity) && isfinite(pressureHpa) &&
         temperatureC >= BME280_MIN_TEMPERATURE_C && temperatureC <= BME280_MAX_TEMPERATURE_C &&
         humidity >= BME280_MIN_HUMIDITY_PERCENT && humidity <= BME280_MAX_HUMIDITY_PERCENT &&
         pressureHpa >= BME280_MIN_PRESSURE_HPA && pressureHpa <= BME280_MAX_PRESSURE_HPA;
}

static bool beginBme280() {
  bmeReady = bme.begin(BME280_I2C_ADDRESS_PRIMARY, &Wire);
  if (!bmeReady) {
    bmeReady = bme.begin(BME280_I2C_ADDRESS_SECONDARY, &Wire);
  }
  return bmeReady;
}

void initEnvironment() {
  lastInitAttempt = millis();
  beginBme280();

  if (bmeReady) {
    Serial.println("Environment sensor: BME280 on I2C");
  } else {
    lastReadFailed = true;
    Serial.println("WARNING: BME280 not found at 0x76 or 0x77");
  }
}

void updateEnvironment() {
  unsigned long now = millis();

  if (!bmeReady) {
    if (now - lastInitAttempt < ENV_INIT_RETRY_INTERVAL_MS) {
      return;
    }
    lastInitAttempt = now;
    if (beginBme280()) {
      lastReadFailed = false;
      Serial.println("Environment sensor: BME280 recovered on I2C");
    } else {
      lastReadFailed = true;
    }
    return;
  }

  if (now - lastReadAttempt < ENV_UPDATE_INTERVAL_MS) {
    return;
  }
  lastReadAttempt = now;

  float temperatureC = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressureHpa = bme.readPressure() / 100.0F;

  if (!readingIsPlausible(temperatureC, humidity, pressureHpa)) {
    lastReadFailed = true;
    bmeReady = false;
    lastInitAttempt = now;
    Serial.println("WARNING: BME280 environment read failed; retrying initialization");
    return;
  }

  lastTemperatureC = temperatureC;
  lastHumidity = humidity;
  lastPressureHpa = pressureHpa;
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

float getPressureHpa() {
  return hasValidReading ? lastPressureHpa : NAN;
}

bool environmentDataValid() {
  return hasValidReading && !lastReadFailed;
}

EnvironmentStatus getEnvironmentStatus() {
  if (!hasValidReading || lastReadFailed) {
    return ENV_STATUS_ERROR;
  }

  float temperatureF = getTemperatureF();
  if (ENABLE_TEMP_FAULTS && temperatureF >= TEMP_FAULT_F) {
    return ENV_STATUS_TEMP_FAULT;
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
