#ifndef CROSSWIND_CONFIG_H
#define CROSSWIND_CONFIG_H

#include <Arduino.h>
#include <DHT.h>

/*
  Crosswind ESP32 Phase 1 configuration

  Wiring summary:
    GPIO18 -> BTS7960 RPWM
    GPIO19 -> BTS7960 LPWM
    GPIO23 -> BTS7960 R_EN
    GPIO13 -> BTS7960 L_EN
    GPIO14 -> opto-isolated relay input for thrower pedal dry contact
    GPIO26 -> DHT11 data
    GPIO21 -> OLED SDA
    GPIO22 -> OLED SCL
    BME280 -> optional environment sensor on shared I2C bus, 0x76 or 0x77
    GPIO34 -> left roller limit switch with external pullup
    GPIO35 -> right roller limit switch with external pullup
    GPIO27 -> DIYables RGB LED module R input, PWM
    GPIO12 -> DIYables RGB LED module G input, PWM
    GPIO4  -> DIYables RGB LED module B input, PWM
    GPIO32 -> rotary encoder CLK to GND, INPUT_PULLUP
    GPIO33 -> rotary encoder DT to GND, INPUT_PULLUP
    GPIO25 -> rotary encoder SW to GND, INPUT_PULLUP, menu/select button
    GPIO16 -> ARM button to GND, INPUT_PULLUP
    GPIO17 -> FIRE / TEST button to GND, INPUT_PULLUP
    GPIO39 -> speed potentiometer wiper, 0-3.3V only

  YL-99 limit modules are currently active LOW. GPIO34/GPIO35 require
  external pullup resistors because ESP32 input-only pins do not have internal
  pullups.

  DIYables common cathode RGB LED module wiring:
    GND -> common ground
    R   -> GPIO27
    G   -> GPIO12
    B   -> GPIO4
  Common cathode means HIGH/PWM > 0 turns a color ON.
  GPIO12 is a strapping pin on many ESP32 boards; if boot/upload issues appear,
  move RGB_GREEN_PIN to a non-strapping PWM-capable GPIO such as GPIO5.
*/

const char FIRMWARE_VERSION[] = "Crosswind ESP32 Phase 1 v1.7-limit-safety";

const int RPWM_PIN = 18;
const int LPWM_PIN = 19;
const int R_EN_PIN = 23;
const int L_EN_PIN = 13;
const int LEFT_LIMIT_PIN = 34;
const int RIGHT_LIMIT_PIN = 35;
const bool ENABLE_LIMIT_SWITCHES = true;
const bool ENABLE_LIMIT_FAULTS = true;
const bool LIMIT_SWITCHES_USE_INTERNAL_PULLUPS = false;
const int START_STOP_BUTTON_PIN = -1;
const int MODE_BUTTON_PIN = -1;
const int ARM_BUTTON_PIN = 16;
const int FIRE_BUTTON_PIN = 17;
const int SPEED_POT_PIN = 39;
const int ROTARY_ENCODER_CLK_PIN = 32;
const int ROTARY_ENCODER_DT_PIN = 33;
const int ROTARY_ENCODER_SW_PIN = 25;
const int THROWER_TRIGGER_PIN = 14;
const int RGB_RED_PIN = 27;
const int RGB_GREEN_PIN = 12;
const int RGB_BLUE_PIN = 4;
const int OLED_SDA_PIN = 21;
const int OLED_SCL_PIN = 22;
const int DHT_PIN = 26;

// Future expansion placeholders for Phase 2 and production hardware.
const int PITCH_ACTUATOR_PIN = -1;
const int BATTERY_VOLTAGE_PIN = -1;

const int PWM_CHANNEL_RIGHT = 0;
const int PWM_CHANNEL_LEFT = 1;
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;
const int RGB_RED_CHANNEL = 2;
const int RGB_GREEN_CHANNEL = 3;
const int RGB_BLUE_CHANNEL = 4;
const int RGB_PWM_FREQ = 1000;
const int RGB_PWM_RESOLUTION = 8;

const uint8_t MIN_PWM = 45;
const uint8_t MAX_PWM = 255;
const uint8_t DEFAULT_PWM = 120;
const uint8_t RAMP_STEP = 5;
const uint16_t RAMP_INTERVAL_MS = 20;
const uint16_t LIMIT_DWELL_MS = 1000;
const uint32_t MAX_TRAVEL_TIME_MS = 30000UL;
const int LIMIT_ACTIVE_STATE = LOW;

const uint16_t DEBOUNCE_DELAY_MS = 50;
const uint16_t LIMIT_DEBOUNCE_MS = 20;
const uint16_t DIRECTION_CHANGE_DEADTIME_MS = 50;
const uint8_t WATCHDOG_TIMEOUT_SECONDS = 5;
const uint16_t BLE_STATUS_INTERVAL_MS = 1000;
const uint16_t SERIAL_STATUS_INTERVAL_MS = 1000;

enum SpeedInputType { SPEED_INPUT_POTENTIOMETER, SPEED_INPUT_ROTARY_ENCODER };
const SpeedInputType SPEED_INPUT_TYPE = SPEED_INPUT_ROTARY_ENCODER;
const uint8_t ROTARY_ENCODER_SPEED_STEP = 5;

enum EnvSensorType { ENV_SENSOR_DHT11, ENV_SENSOR_BME280 };
const EnvSensorType ENV_SENSOR_TYPE = ENV_SENSOR_DHT11;
const uint8_t DHT_TYPE = DHT11;
const uint16_t ENV_UPDATE_INTERVAL_MS = 2000;
const float TEMP_WARNING_F = 120.0F;
const float TEMP_FAULT_F = 150.0F;
const bool ENABLE_TEMP_FAULTS = true;

// BME280 uses the existing I2C bus on GPIO21/GPIO22 and is selected by
// changing ENV_SENSOR_TYPE to ENV_SENSOR_BME280. Most breakout boards use
// address 0x76 or 0x77; firmware tries both.
const uint8_t BME280_I2C_ADDRESS_PRIMARY = 0x76;
const uint8_t BME280_I2C_ADDRESS_SECONDARY = 0x77;

// Thrower trigger relay settings. The relay contacts should be dry-contact
// only, wired in parallel with the factory pedal. The ESP32 must not inject
// voltage into the thrower pedal circuit.
const bool ENABLE_THROWER_TRIGGER = true;
const bool ALLOW_TRIGGER_WHEN_STOPPED = true;
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
  FAULT_LIMIT = 6,
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
