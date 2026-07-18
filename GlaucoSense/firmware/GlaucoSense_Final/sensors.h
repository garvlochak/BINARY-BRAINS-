/*
 * GlaucoSense - sensors.h
 * Owner: M2 (sensors)
 * Purpose: Low-level reads for the IR presence sensor, the MPX5010DP
 *          pressure sensor, and the VL53L0X distance sensor. Every function
 *          here does ONE read/check and returns immediately - no internal
 *          timing loops or delay() calls - so the non-blocking state
 *          machine in GlaucoSense_Final.ino can call these every loop()
 *          iteration without ever stalling.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Starts I2C, the VL53L0X distance sensor in continuous mode, and the IR
// sensor pin. Call once in setup(), after initActuators(). Returns true on
// success (false if the VL53L0X isn't found).
bool initSensors();

// Takes PRESSURE_ZERO_SAMPLES pressure readings and stores the average as
// the zero offset. Call this once at boot with the system open to
// atmosphere (0 kPa, pump and valve off). This one function is allowed to
// block briefly since it only runs once, before the main loop starts.
void calibratePressureZero();

// Returns the current pressure in kPa: zero-offset corrected and smoothed
// with a PRESSURE_FILTER_SAMPLES-point moving average.
float readPressureKPa();

// Returns the current distance in millimetres from the VL53L0X.
// Returns -1 if the reading is invalid (sensor timeout or > TOF_MAX_VALID_MM).
int readDistanceMM();

// Returns true if the IR sensor's raw reading has matched IR_ACTIVE_LEVEL
// for IR_DEBOUNCE_COUNT consecutive calls spaced at least
// IR_DEBOUNCE_INTERVAL_MS apart (call this every loop() iteration - it
// tracks its own debounce state internally and returns instantly each time).
bool isEyePresent();

#endif // SENSORS_H
