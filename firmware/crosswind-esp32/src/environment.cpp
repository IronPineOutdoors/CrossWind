#include "environment.h"

#include <Adafruit_BME280.h>
#include <DHT.h>
#include <Wire.h>

static DHT dht(DHT_PIN, DHT_TYPE);
static Adafruit_BME280 bme;
static float lastTemperatureC = NAN;
static float lastHumidity = NAN;
static float lastPressureHpa = NAN;
static bool hasValidReading = false;
static bool lastReadFailed = false;
static bool bmeReady = false;
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

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  bmeReady = bme.begin(BME280_I2C_ADDRESS_PRIMARY, &Wire);
  if (!bmeReady) {
    bmeReady = bme.begin(BME280_I2C_ADDRESS_SECONDARY, &Wire);
  }

  if (bmeReady) {
    Serial.println("Environment sensor: BME280 on I2C");
  } else {
    lastReadFailed = true;
    Serial.println("WARNING: BME280 not found at 0x76 or 0x77");
  }
}

void updateEnvironment() {
  unsigned long now = millis();
  if (now - lastReadAttempt < ENV_UPDATE_INTERVAL_MS) {
    return;
  }
  lastReadAttempt = now;

  if (ENV_SENSOR_TYPE == ENV_SENSOR_DHT11) {
    float humidity = dht.readHumidity();
    float temperatureC = dht.readTemperature();

    if (isnan(humidity) || isnan(temperatureC)) {
      lastReadFailed = true;
      Serial.println("WARNING: DHT11 environment read failed; keeping last valid reading");
      return;
    }

    lastHumidity = humidity;
    lastTemperatureC = temperatureC;
    lastPressureHpa = NAN;
    hasValidReading = true;
    lastReadFailed = false;
    return;
  }

  if (!bmeReady) {
    lastReadFailed = true;
    return;
  }

  float temperatureC = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressureHpa = bme.readPressure() / 100.0F;

  if (isnan(temperatureC) || isnan(humidity) || isnan(pressureHpa)) {
    lastReadFailed = true;
    Serial.println("WARNING: BME280 environment read failed; keeping last valid reading");
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
