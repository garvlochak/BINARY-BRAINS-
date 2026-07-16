/*
 * GlaucoSense - sensors.cpp
 * Owner: M2 (sensors)
 * Purpose: Implements pressure + distance sensing and the trial capture
 *          routine (which also pulses the valve at the right moment).
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#include "sensors.h"
#include "config.h"
#include "actuators.h"
#include <Wire.h>
#include <VL53L0X.h>

static VL53L0X tofSensor;

static float pressureZeroOffsetKPa = 0.0f;
static float pressureFilterBuf[PRESSURE_FILTER_SAMPLES];
static int pressureFilterIndex = 0;
static bool pressureFilterFilled = false;

// Reads the sensor and converts millivolts straight to kPa, with no
// zero-offset or filtering applied yet.
static float readRawPressureKPa() {
  int mv = analogReadMilliVolts(PRESSURE_ADC_PIN);
  float voltageAtPin = mv / 1000.0f;

  // Undo the 12k/20k voltage divider to recover the sensor's real Vout.
  float sensorVoltage = voltageAtPin / PRESSURE_DIVIDER_RATIO;

  // MPX5010DP: Vout = 5.0 * (0.09*P + 0.04), solved for P:
  float pressureKPa = (sensorVoltage / PRESSURE_SUPPLY_V - PRESSURE_OFFSET_V)
                       / PRESSURE_SLOPE_V_PER_KPA;
  return pressureKPa;
}

// Simple ring-buffer moving average filter.
static float pressureMovingAverage(float newSample) {
  pressureFilterBuf[pressureFilterIndex] = newSample;
  pressureFilterIndex = (pressureFilterIndex + 1) % PRESSURE_FILTER_SAMPLES;
  if (pressureFilterIndex == 0) {
    pressureFilterFilled = true;
  }

  int count = pressureFilterFilled ? PRESSURE_FILTER_SAMPLES : pressureFilterIndex;
  if (count == 0) {
    return newSample;
  }

  float sum = 0;
  for (int i = 0; i < count; i++) {
    sum += pressureFilterBuf[i];
  }
  return sum / count;
}

bool initSensors() {
  analogReadResolution(ADC_RESOLUTION_BITS);
  analogSetAttenuation(ADC_ATTENUATION);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  tofSensor.setTimeout(TOF_TIMEOUT_MS);
  if (!tofSensor.init()) {
    return false;
  }

  tofSensor.setMeasurementTimingBudget(TOF_TIMING_BUDGET_US);
  tofSensor.startContinuous();

  return true;
}

void calibratePressureZero() {
  float sum = 0;
  for (int i = 0; i < PRESSURE_ZERO_SAMPLES; i++) {
    sum += readRawPressureKPa();
    delay(10);
  }
  pressureZeroOffsetKPa = sum / PRESSURE_ZERO_SAMPLES;
}

float readPressureKPa() {
  float zeroed = readRawPressureKPa() - pressureZeroOffsetKPa;
  return pressureMovingAverage(zeroed);
}

int readDistanceMM() {
  uint16_t distance = tofSensor.readRangeContinuousMillimeters();

  if (tofSensor.timeoutOccurred()) {
    return -1;
  }
  if (distance > TOF_MAX_VALID_MM) {
    return -1;
  }
  return (int)distance;
}

float measureBaselineMM() {
  unsigned long start = millis();
  float sum = 0;
  int count = 0;

  while (millis() - start < BASELINE_SAMPLE_MS) {
    int d = readDistanceMM();
    if (d >= 0) {
      sum += d;
      count++;
    }
    delay(SAMPLE_INTERVAL_MS);
  }

  if (count == 0) {
    return -1;
  }
  return sum / count;
}

void captureTrial(unsigned long* timeMs, float* pressureKPa, int* distanceMM,
                   int maxSamples, int* sampleCount,
                   unsigned long durationMs,
                   unsigned long valveOpenOffsetMs, unsigned long valveOpenMs) {
  unsigned long start = millis();
  int count = 0;
  bool hasOpened = false;
  bool hasClosed = false;

  while ((millis() - start) < durationMs && count < maxSamples) {
    unsigned long elapsed = millis() - start;

    if (!hasOpened && elapsed >= valveOpenOffsetMs) {
      valveOpen();
      hasOpened = true;
    }
    if (hasOpened && !hasClosed && elapsed >= (valveOpenOffsetMs + valveOpenMs)) {
      valveClose();
      hasClosed = true;
    }

    timeMs[count] = elapsed;
    pressureKPa[count] = readPressureKPa();
    distanceMM[count] = readDistanceMM();
    count++;

    delay(SAMPLE_INTERVAL_MS);
  }

  // SAFETY: no matter how the loop above ended (duration elapsed, buffer
  // full, valveOpenOffsetMs never reached, etc.) the valve must not be
  // left open.
  if (hasOpened && !hasClosed) {
    valveClose();
  }

  *sampleCount = count;
}
