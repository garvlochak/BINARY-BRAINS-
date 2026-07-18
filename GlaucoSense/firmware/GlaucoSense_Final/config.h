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
// PIN MAP (classic ESP32-WROOM-32 Dev Module / ESP32-DevKitC)
// ---------------------------------------------------------------------
//   Signal              GPIO   Notes
//   ------------------  -----  ------------------------------------------
//   I2C_SDA_PIN         21     VL53L0X + SSD1306 OLED, shared bus
//   I2C_SCL_PIN         22     VL53L0X + SSD1306 OLED, shared bus
//   PUMP_PIN            25     Pump MOSFET gate (output-capable)
//   VALVE_PIN           26     Valve MOSFET gate (output-capable)
//   PRESSURE_ADC_PIN    34     MPX5010DP via voltage divider (ADC1_CH6, input-only)
//   IR_SENSOR_PIN       35     Generic IR obstacle sensor OUT (input-only)
//   START_BUTTON_PIN    27     Manual trial start, INPUT_PULLUP, pressed = LOW
//   STATUS_LED_PIN      32     Status LED
//
// Classic-ESP32 rules these follow (do not reassign without re-checking):
//   - GPIO 6-11 are wired to the module's internal SPI flash - NEVER use.
//   - GPIO 34/35/36/39 are INPUT-ONLY (no pull-up/down, cannot drive an
//     output) - used here only for sensor inputs (pressure, IR), never
//     for the pump/valve/LED.
//   - GPIO 0/2/12/15 are strapping/boot pins - avoided entirely here.
//   - GPIO 1/3 are UART0 (flashing + Serial) - avoided entirely here.
//   - ADC2 pins don't work while WiFi is active - PRESSURE_ADC_PIN uses
//     GPIO34, which is ADC1, not ADC2.
// ---------------------------------------------------------------------
#define I2C_SDA_PIN        21
#define I2C_SCL_PIN        22
#define PUMP_PIN           25
#define VALVE_PIN          26
#define PRESSURE_ADC_PIN   34   // ADC1_CH6 (input-only pin, fine for an analog reading)
#define IR_SENSOR_PIN      35   // input-only pin, fine for a digital input
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
// before reaching PRESSURE_ADC_PIN. NEVER wire Vout straight to the ESP32 -
// at full scale Vout is ~4.7V, which exceeds the ESP32's 3.3V ADC limit.
// ---------------------------------------------------------------------
#define PRESSURE_DIVIDER_RATIO      0.625f
#define PRESSURE_SUPPLY_V           5.0f
#define PRESSURE_SLOPE_V_PER_KPA    0.09f
#define PRESSURE_OFFSET_V           0.04f
#define ADC_ATTENUATION             ADC_11db
#define ADC_RESOLUTION_BITS         12
#define PRESSURE_ZERO_SAMPLES       50   // samples averaged during calibratePressureZero()
#define PRESSURE_FILTER_SAMPLES     5    // 5-point moving average

// Hard safety ceiling, independent of TARGET_PRESSURE_KPA: if the reading
// ever exceeds this, the pump is force-stopped immediately, no matter what
// state the trial is in. Keeps a target-pressure bug from over-pressurizing
// the reservoir. Sensor is rated to 10 kPa; keep well under that.
#define PRESSURE_HARD_LIMIT_KPA     8.0f

// ---------------------------------------------------------------------
// Distance sensor (VL53L0X ToF, continuous mode)
// ---------------------------------------------------------------------
#define TOF_TIMING_BUDGET_US   20000    // ~50 Hz
#define TOF_MAX_VALID_MM       1000     // anything above this is treated as invalid
#define TOF_TIMEOUT_MS         500

// ---------------------------------------------------------------------
// IR obstacle sensor (generic digital module, eye/object presence)
// ---------------------------------------------------------------------
// Most cheap IR obstacle-sensor modules (e.g. FC-51 style) pull their OUT
// pin LOW when they detect something in range (and light an onboard LED at
// the same time), sitting HIGH when clear. If yours behaves the opposite
// way, flip this one line - nothing else needs to change.
#define IR_ACTIVE_LEVEL        LOW

// Require the detection level to be stable for this many consecutive reads
// before believing it - kills false triggers from noise or a loose wire.
#define IR_DEBOUNCE_COUNT      5
#define IR_DEBOUNCE_INTERVAL_MS 10   // ~5 reads x 10ms = ~50ms to confirm

// ---------------------------------------------------------------------
// Trial state machine timing / tuning
// ---------------------------------------------------------------------
#define TARGET_PRESSURE_KPA    3.0f
#define PUMP_TIMEOUT_MS        10000   // hard max pump-on time during CHARGING
#define STABILIZE_MS           500
#define VALVE_OPEN_MS          80
#define RECORD_MS              1500
#define SAMPLE_INTERVAL_MS     20
#define BASELINE_SAMPLE_MS     500      // time spent averaging the resting distance
#define RESULT_DISPLAY_MS      3000     // how long to show the result before resetting

// Big enough to hold RECORD_MS / SAMPLE_INTERVAL_MS samples plus some slack
#define MAX_TRIAL_SAMPLES      100

#endif // CONFIG_H
