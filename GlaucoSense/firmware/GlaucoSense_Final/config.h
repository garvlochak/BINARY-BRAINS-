/*
 * GlaucoSense - config.h
 * Owner: M1 (shared by the whole team)
 * Purpose: THE single source of truth for every pin number and tuning
 *          value used by the final firmware. Do not hardcode a pin or
 *          timing number anywhere else in GlaucoSense_Final - #include
 *          this file and use these names instead.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ---------------------------------------------------------------------
// Pin assignments (classic ESP32-WROOM-32 Dev Module)
// GPIO6-11 are reserved for the module's integrated SPI flash on this
// board and must NEVER be used here - that's why these differ from the
// old ESP32-S3 pin plan.
// ---------------------------------------------------------------------
#define I2C_SDA_PIN        21
#define I2C_SCL_PIN        22
#define PUMP_PIN           25
#define VALVE_PIN          26
#define PRESSURE_ADC_PIN   34   // ADC1_CH6 (input-only pin, fine for an analog reading)
#define START_BUTTON_PIN   27   // INPUT_PULLUP, pressed = LOW
#define STATUS_LED_PIN     32

// ---------------------------------------------------------------------
// I2C device addresses
// ---------------------------------------------------------------------
#define OLED_I2C_ADDR      0x3C

// ---------------------------------------------------------------------
// Pressure sensor (MPX5010DP, 0-10 kPa, ratiometric to 5V)
// Transfer function : Vout = 5.0 * (0.09*P_kPa + 0.04)
// Vout then passes through a 12k/20k resistor divider (ratio 0.625)
// before reaching PRESSURE_ADC_PIN. NEVER wire Vout straight to the ESP32.
// ---------------------------------------------------------------------
#define PRESSURE_DIVIDER_RATIO      0.625f
#define PRESSURE_SUPPLY_V           5.0f
#define PRESSURE_SLOPE_V_PER_KPA    0.09f
#define PRESSURE_OFFSET_V           0.04f
#define ADC_ATTENUATION             ADC_11db
#define ADC_RESOLUTION_BITS         12
#define PRESSURE_ZERO_SAMPLES       50   // samples averaged during calibratePressureZero()
#define PRESSURE_FILTER_SAMPLES     5    // 5-point moving average

// ---------------------------------------------------------------------
// Distance sensor (VL53L0X ToF, continuous mode)
// ---------------------------------------------------------------------
#define TOF_TIMING_BUDGET_US   20000    // ~50 Hz
#define TOF_MAX_VALID_MM       1000     // anything above this is treated as invalid
#define TOF_TIMEOUT_MS         500

// ---------------------------------------------------------------------
// Trial state machine timing / tuning
// ---------------------------------------------------------------------
#define TARGET_PRESSURE_KPA    3.0f
#define PUMP_TIMEOUT_MS        10000
#define STABILIZE_MS           500
#define VALVE_OPEN_MS          80
#define RECORD_MS              1500
#define SAMPLE_INTERVAL_MS     20
#define BASELINE_SAMPLE_MS     500      // time spent averaging the resting distance

// Big enough to hold RECORD_MS / SAMPLE_INTERVAL_MS samples plus some slack
#define MAX_TRIAL_SAMPLES      100

#endif // CONFIG_H
