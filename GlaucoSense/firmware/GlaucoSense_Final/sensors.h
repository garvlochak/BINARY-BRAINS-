/*
 * GlaucoSense - sensors.h
 * Owner: M2 (sensors)
 * Purpose: Reads the MPX5010DP pressure sensor and the VL53L0X distance
 *          sensor, and captures a full trial's worth of samples.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Starts I2C and the VL53L0X distance sensor in continuous mode.
// Call once in setup(), after initActuators(). Returns true on success.
bool initSensors();

// Takes PRESSURE_ZERO_SAMPLES pressure readings and stores the average as
// the zero offset. Call this once at boot with the system open to
// atmosphere (0 kPa, pump and valve off).
void calibratePressureZero();

// Returns the current pressure in kPa: zero-offset corrected and smoothed
// with a PRESSURE_FILTER_SAMPLES-point moving average.
float readPressureKPa();

// Returns the current distance in millimetres from the VL53L0X.
// Returns -1 if the reading is invalid (sensor timeout or > TOF_MAX_VALID_MM).
int readDistanceMM();

// Averages distance readings over BASELINE_SAMPLE_MS and returns the
// result, in mm. Used at the start of a trial to record the membrane's
// resting position. Returns -1 if no valid reading was ever taken.
float measureBaselineMM();

// Records pressure + distance every SAMPLE_INTERVAL_MS for durationMs.
// Opens the valve at valveOpenOffsetMs (measured from the start of this
// call) and closes it again valveOpenMs later, while sampling continues.
// Fills timeMs/pressureKPa/distanceMM (each must have at least maxSamples
// slots) and writes how many samples were actually taken into *sampleCount.
// Safety: guarantees the valve is closed before returning, no matter what.
void captureTrial(unsigned long* timeMs, float* pressureKPa, int* distanceMM,
                   int maxSamples, int* sampleCount,
                   unsigned long durationMs,
                   unsigned long valveOpenOffsetMs, unsigned long valveOpenMs);

#endif // SENSORS_H
