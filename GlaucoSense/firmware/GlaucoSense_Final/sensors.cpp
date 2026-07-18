/*
 * GlaucoSense - sensors.cpp
 * Owner: M2 (sensors)
 * Purpose: Implements IR presence detection, pressure sensing, and distance
 *          sensing. Every function does one read and returns immediately -
 *          no internal while-loops or delay() in the main-loop path - so
 *          GlaucoSense_Final.ino's non-blocking state machine can call
 *          these every iteration without ever stalling.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#include "sensors.h"
#include "config.h"
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

  // IR sensor OUT pin. A loose/disconnected signal wire will float and can
  // cause false triggers - double check this connection if isEyePresent()
  // ever behaves erratically.
  pinMode(IR_SENSOR_PIN, INPUT);

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

// Debounce state: irCandidateValue/irCandidateCount track a value that
// might become the new stable state; it only takes effect once it has been
// read IR_DEBOUNCE_COUNT times in a row.
static bool irCandidateValue = false;
static int irCandidateCount = 0;
static unsigned long irLastReadMs = 0;
static bool irStableState = false;

bool isEyePresent() {
  unsigned long now = millis();
  if (now - irLastReadMs < IR_DEBOUNCE_INTERVAL_MS) {
    return irStableState; // too soon to sample again, return last known-good value
  }
  irLastReadMs = now;

  int raw = digitalRead(IR_SENSOR_PIN);
  bool detected = (raw == IR_ACTIVE_LEVEL);

  if (detected == irCandidateValue) {
    irCandidateCount++;
  } else {
    irCandidateValue = detected;
    irCandidateCount = 1;
  }

  if (irCandidateCount >= IR_DEBOUNCE_COUNT) {
    irStableState = irCandidateValue;
  }

  return irStableState;
}
