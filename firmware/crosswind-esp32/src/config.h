#ifndef CROSSWIND_CONFIG_H
#define CROSSWIND_CONFIG_H

#include <Arduino.h>

/*
  Crosswind ESP32 Phase 1 configuration

  Wiring summary:
    GPIO18 -> BTS7960 RPWM
    GPIO19 -> BTS7960 LPWM
    GPIO23 -> BTS7960 R_EN
    GPIO13 -> BTS7960 L_EN
    GPIO14 -> opto-isolated relay input for thrower pedal dry contact
    GPIO21 -> OLED SDA
    GPIO22 -> OLED SCL
    BME280 -> environment sensor on shared I2C bus, 0x76 or 0x77
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

const char FIRMWARE_VERSION[] = "Crosswind ESP32 Phase 1 v1.14-ui";

#ifndef CROSSWIND_MOTION_SIMULATION
#define CROSSWIND_MOTION_SIMULATION 0
#endif

#ifndef CROSSWIND_BATTERY_SIMULATION
#define CROSSWIND_BATTERY_SIMULATION 0
#endif

#ifndef CROSSWIND_UI_SIMULATION
#define CROSSWIND_UI_SIMULATION 0
#endif

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
const int ESTOP_PIN = -1;
const int ARM_BUTTON_PIN = 16;
const int FIRE_BUTTON_PIN = 17;
const int SPEED_POT_PIN = 39;
const int ROTARY_ENCODER_CLK_PIN = 32;
const int ROTARY_ENCODER_DT_PIN = 33;
const int ROTARY_ENCODER_SW_PIN = 25;
const int THROWER_TRIGGER_PIN = 14;
const int RGB_RED_PIN = 27;
// GPIO12 is an ESP32 strapping pin; use GPIO5 for green if boot/upload becomes unreliable.
const int RGB_GREEN_PIN = 12;
const int RGB_BLUE_PIN = 4;
const int OLED_SDA_PIN = 21;
const int OLED_SCL_PIN = 22;
const int MOTOR_CURRENT_SENSE_PIN = -1;

// Future expansion placeholders for Phase 2 and production hardware.
const int PITCH_ACTUATOR_PIN = -1;
const int BATTERY_VOLTAGE_PIN = 36;  // ADC1_CH0, input-only, BLE-safe.

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
const uint16_t ENDPOINT_DWELL_MIN_MS = 750;
const uint16_t ENDPOINT_DWELL_MAX_MS = 1500;
const uint32_t RANDOM_MOVE_MIN_MS = 1000UL;
const uint32_t RANDOM_MOVE_MAX_MS = 5000UL;
const uint8_t RANDOM_SPEED_MIN_PWM = 60;
const uint8_t RANDOM_SPEED_MAX_PWM = 180;
const uint8_t RANDOM_FULL_SWEEP_PERCENT = 40;
const uint32_t LIMIT_RELEASE_TIMEOUT_MS = 1500UL;
const uint8_t CENTERING_SPEED_PWM = 70;
const uint8_t CALIBRATION_SPEED_PWM = 70;
const uint32_t DEFAULT_FULL_TRAVEL_TIME_MS = 10000UL;
const uint32_t MIN_VALID_CALIBRATION_TIME_MS = 1000UL;
const uint32_t MAX_VALID_CALIBRATION_TIME_MS = 30000UL;
const uint8_t CENTERING_PERCENT = 50;
const uint32_t FLUSH_MOVE_TIME_MS = 2500UL;
const uint32_t FLUSH_READY_TIME_MS = 1000UL;
const uint32_t FLUSH_PAUSE_TIME_MS = 1500UL;
const uint32_t MOTION_FIXED_RANDOM_SEED = 0UL;
const uint32_t MOTION_SIMULATED_FULL_TRAVEL_MS = 4000UL;
const int LIMIT_ACTIVE_STATE = LOW;

const uint16_t DEBOUNCE_DELAY_MS = 50;
const uint16_t LIMIT_DEBOUNCE_MS = 20;
const uint16_t DIRECTION_CHANGE_DEADTIME_MS = 50;
const uint8_t WATCHDOG_TIMEOUT_SECONDS = 5;
const uint16_t BLE_STATUS_INTERVAL_MS = 1000;
const uint16_t BLE_COMMAND_MIN_INTERVAL_MS = 100;
const uint16_t SERIAL_STATUS_INTERVAL_MS = 1000;
const uint16_t UI_DISPLAY_REFRESH_MS = 125;
const uint16_t UI_BOOT_SCREEN_MS = 1500;
const uint16_t UI_LONG_PRESS_MS = 800;
const uint16_t UI_NOTIFICATION_MS = 1800;
const uint8_t UI_DEFAULT_CONTRAST = 180;
const uint8_t UI_MIN_CONTRAST = 30;
const uint8_t UI_MAX_CONTRAST = 255;
const uint8_t UI_CONTRAST_STEP = 15;
const uint16_t UI_DEFAULT_STATUS_TIMEOUT_SECONDS = 30;
const uint16_t UI_MIN_STATUS_TIMEOUT_SECONDS = 10;
const uint16_t UI_MAX_STATUS_TIMEOUT_SECONDS = 120;
const uint16_t UI_STATUS_TIMEOUT_STEP_SECONDS = 5;
const uint32_t UI_DIM_TIMEOUT_MS = 60000UL;
const uint32_t UI_OFF_TIMEOUT_MS = 120000UL;
const bool UI_STAY_AWAKE_WHILE_RUNNING = true;

enum BatteryMeasurementPoint { BATTERY_POINT_TOOL_INPUT, BATTERY_POINT_MOTOR_12V_BUS, BATTERY_POINT_CUSTOM };
enum BatteryProfile { BATTERY_PROFILE_TOOL_20V, BATTERY_PROFILE_12V_BUS, BATTERY_PROFILE_CUSTOM };
const bool BATTERY_MONITOR_DEFAULT_ENABLED = false;
const BatteryMeasurementPoint BATTERY_MEASUREMENT_POINT = BATTERY_POINT_TOOL_INPUT;
const BatteryProfile BATTERY_DEFAULT_PROFILE = BATTERY_PROFILE_TOOL_20V;
const float BATTERY_DIVIDER_UPPER_OHMS = 100000.0F;
const float BATTERY_DIVIDER_LOWER_OHMS = 12000.0F;
const float BATTERY_ADC_MAX_PIN_VOLTAGE = 3.3F;
const uint16_t BATTERY_ADC_MAX_RAW = 4095;
const adc_attenuation_t BATTERY_ADC_ATTENUATION = ADC_11db;
const float BATTERY_DEFAULT_CALIBRATION_MULTIPLIER = 1.0F;
const float BATTERY_DEFAULT_CALIBRATION_OFFSET_V = 0.0F;
const uint16_t BATTERY_SAMPLE_INTERVAL_MS = 100;
const uint16_t BATTERY_STARTUP_SETTLE_MS = 2000;
const uint8_t BATTERY_FILTER_MIN_SAMPLES = 8;
const float BATTERY_FILTER_ALPHA = 0.20F;
const uint8_t BATTERY_LOW_CONFIRM_SAMPLES = 5;
const uint8_t BATTERY_CRITICAL_CONFIRM_SAMPLES = 10;
const uint8_t BATTERY_RECOVERY_CONFIRM_SAMPLES = 10;
const uint8_t BATTERY_SENSOR_ERROR_CONFIRM_SAMPLES = 5;
const bool BATTERY_PERCENTAGE_ENABLED = false;
const bool ENABLE_BATTERY_CRITICAL_FAULT = true;
const bool ENABLE_BATTERY_SENSOR_FAULT = false;
const float BATTERY_FULL_V = 21.0F;
const float BATTERY_NOMINAL_V = 18.0F;
const float BATTERY_LOW_WARNING_V = 16.5F;
const float BATTERY_CRITICAL_V = 15.0F;
const float BATTERY_RECOVERY_V = 17.0F;
const float BATTERY_ABSOLUTE_MIN_VALID_V = 5.0F;
const float BATTERY_ABSOLUTE_MAX_VALID_V = 26.0F;
const float BATTERY_CALIBRATION_KNOWN_MIN_V = 5.0F;
const float BATTERY_CALIBRATION_KNOWN_MAX_V = 30.0F;
const float BATTERY_CALIBRATION_MULTIPLIER_MIN = 0.75F;
const float BATTERY_CALIBRATION_MULTIPLIER_MAX = 1.25F;
const float BATTERY_SIMULATION_DEFAULT_V = 20.0F;
const float BATTERY_12V_FULL_V = 12.6F;
const float BATTERY_12V_NOMINAL_V = 12.0F;
const float BATTERY_12V_LOW_WARNING_V = 11.5F;
const float BATTERY_12V_CRITICAL_V = 10.8F;
const float BATTERY_12V_RECOVERY_V = 11.8F;
const float BATTERY_12V_ABSOLUTE_MIN_VALID_V = 8.0F;
const float BATTERY_12V_ABSOLUTE_MAX_VALID_V = 15.0F;

enum SpeedInputType { SPEED_INPUT_POTENTIOMETER, SPEED_INPUT_ROTARY_ENCODER };
const SpeedInputType SPEED_INPUT_TYPE = SPEED_INPUT_ROTARY_ENCODER;
const uint8_t ROTARY_ENCODER_SPEED_STEP = 5;

const uint16_t ENV_UPDATE_INTERVAL_MS = 2000;
const float TEMP_WARNING_F = 120.0F;
const float TEMP_FAULT_F = 150.0F;
const bool ENABLE_TEMP_FAULTS = true;

// BME280 uses the existing I2C bus on GPIO21/GPIO22. Most breakout boards
// use address 0x76 or 0x77; firmware tries both.
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
const bool ENABLE_MOTOR_SESSION_TIMEOUT = false;
const uint32_t MOTOR_SESSION_TIMEOUT_MS = 0;
const bool ENABLE_MOTOR_OVERCURRENT_FAULT = false;
const int MOTOR_OVERCURRENT_THRESHOLD_RAW = 4095;

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
  FAULT_ESTOP = 7,
  FAULT_RUN_TIMEOUT = 8,
  FAULT_OVERCURRENT = 9,
  FAULT_LIMIT_STUCK = 10,
  FAULT_UNEXPECTED_LIMIT = 11,
  FAULT_CALIBRATION_FAILED = 12,
  FAULT_BATTERY_CRITICAL = 13,
  FAULT_BATTERY_SENSOR = 14,
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
