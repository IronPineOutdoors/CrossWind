#ifndef CROSSWIND_CONFIG_H
#define CROSSWIND_CONFIG_H

#include <Arduino.h>
#include <DHT.h>

/*
  Crosswind ESP32 Phase 1 configuration

  Wiring summary:
    GPIO25 -> BTS7960 RPWM
    GPIO13 -> BTS7960 LPWM
    GPIO27 -> BTS7960 R_EN
    GPIO14 -> BTS7960 L_EN
    GPIO26 -> DHT11 data
    GPIO21 -> OLED SDA
    GPIO22 -> OLED SCL
    GPIO32 -> left roller limit switch to GND, INPUT_PULLUP
    GPIO33 -> right roller limit switch to GND, INPUT_PULLUP
    GPIO18 -> start/stop button to GND, INPUT_PULLUP
    GPIO19 -> mode button to GND, INPUT_PULLUP
    GPIO39 -> speed potentiometer wiper, 0-3.3V only
    GPIO23 -> opto-isolated relay input for thrower pedal dry contact
    GPIO2  -> status LED

  Normally closed limit switches are preferred for Phase 1. With INPUT_PULLUP,
  normal travel reads LOW and an opened limit switch reads HIGH.
*/

const char FIRMWARE_VERSION[] = "Crosswind ESP32 Phase 1 v1.4-env";

const int RPWM_PIN = 25;
const int LPWM_PIN = 13;
const int R_EN_PIN = 27;
const int L_EN_PIN = 14;
const int LEFT_LIMIT_PIN = 32;
const int RIGHT_LIMIT_PIN = 33;
const int START_STOP_BUTTON_PIN = 18;
const int MODE_BUTTON_PIN = 19;
const int SPEED_POT_PIN = 39;
const int THROWER_TRIGGER_PIN = 23;
const int STATUS_LED_PIN = 2;
const int OLED_SDA_PIN = 21;
const int OLED_SCL_PIN = 22;
const int DHT_PIN = 26;

// Future expansion placeholders for Phase 2 and production hardware.
const int PITCH_ACTUATOR_PIN = -1;
const int BATTERY_VOLTAGE_PIN = -1;
const int ROTARY_ENCODER_A_PIN = -1;
const int ROTARY_ENCODER_B_PIN = -1;

const int PWM_CHANNEL_RIGHT = 0;
const int PWM_CHANNEL_LEFT = 1;
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;

const uint8_t MIN_PWM = 45;
const uint8_t MAX_PWM = 255;
const uint8_t DEFAULT_PWM = 120;
const uint8_t RAMP_STEP = 5;
const uint16_t RAMP_INTERVAL_MS = 20;
const uint16_t LIMIT_DWELL_MS = 1000;
const uint32_t MAX_TRAVEL_TIME_MS = 30000UL;
const int LIMIT_ACTIVE_STATE = 1;

const uint16_t DEBOUNCE_DELAY_MS = 50;
const uint16_t LIMIT_DEBOUNCE_MS = 20;
const uint16_t DIRECTION_CHANGE_DEADTIME_MS = 50;
const uint8_t WATCHDOG_TIMEOUT_SECONDS = 5;
const uint16_t BLE_STATUS_INTERVAL_MS = 1000;
const uint16_t SERIAL_STATUS_INTERVAL_MS = 1000;

enum EnvSensorType { ENV_SENSOR_DHT11, ENV_SENSOR_BME280 };
const EnvSensorType ENV_SENSOR_TYPE = ENV_SENSOR_DHT11;
const uint8_t DHT_TYPE = DHT11;
const uint16_t ENV_UPDATE_INTERVAL_MS = 2000;
const float TEMP_WARNING_F = 120.0F;
const float TEMP_FAULT_F = 150.0F;
const bool ENABLE_TEMP_FAULTS = true;

// BME280 Beta upgrade notes: use the existing I2C bus on GPIO21/GPIO22.
// Most breakout boards use address 0x76 or 0x77. See environment.cpp for
// the compile-time hook; the Alpha build stays on DHT11.
const uint8_t BME280_I2C_ADDRESS_PRIMARY = 0x76;
const uint8_t BME280_I2C_ADDRESS_SECONDARY = 0x77;

// Thrower trigger relay settings. The relay contacts should be dry-contact
// only, wired in parallel with the factory pedal. The ESP32 must not inject
// voltage into the thrower pedal circuit.
const bool ENABLE_THROWER_TRIGGER = true;
const bool ALLOW_TRIGGER_WHEN_STOPPED = false;
const bool ENABLE_AUTOMATIC_TRIGGER = false;
const uint16_t TRIGGER_PULSE_MS = 500;
const uint32_t MIN_TRIGGER_INTERVAL_MS = 3000UL;
const int TRIGGER_ACTIVE_STATE = HIGH;

const char BLE_DEVICE_NAME[] = "Crosswind-ESP32";
const char BLE_SERVICE_UUID[] = "12345678-1234-1234-1234-1234567890ab";
const char BLE_COMMAND_CHAR_UUID[] = "12345678-1234-1234-1234-1234567890ac";
const char BLE_STATUS_CHAR_UUID[] = "12345678-1234-1234-1234-1234567890ad";
const char BLE_RESPONSE_CHAR_UUID[] = "12345678-1234-1234-1234-1234567890ae";

enum Direction { DIR_RIGHT, DIR_LEFT };
enum Mode { SWEEP, RANDOM, FLUSH, CENTERING };
enum FaultCode {
  FAULT_NONE = 0,
  FAULT_TRAVEL_TIMEOUT = 1,
  FAULT_BOTH_LIMITS = 2,
  FAULT_BUTTON_STUCK = 3,
  FAULT_STARTUP_BOTH_LIMITS = 4,
  FAULT_TEMP = 5,
  FAULT_UNKNOWN = 255
};

struct ControllerState {
  Mode mode;
  Direction direction;
  bool running;
  bool faultActive;
  FaultCode lastFault;
  uint8_t speed;
};

#endif
