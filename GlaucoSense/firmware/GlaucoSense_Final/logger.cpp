/*
 * GlaucoSense - logger.cpp
 * Owner: M4 (display / logging)
 * Purpose: Implements the trial counter and Serial CSV logging.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#include "logger.h"
#include <Arduino.h>

static int trialCounter = 1;

void initLogger() {
  trialCounter = 1;
}

int nextTrialNumber() {
  int n = trialCounter;
  trialCounter++;
  return n;
}

void logRawHeader() {
  Serial.println("trial,time_ms,pressure_kpa,distance_mm");
}

void logRawSample(int trialNumber, unsigned long timeMs, float pressureKPa, int distanceMM) {
  Serial.print(trialNumber);
  Serial.print(",");
  Serial.print(timeMs);
  Serial.print(",");
  Serial.print(pressureKPa, 3);
  Serial.print(",");
  Serial.println(distanceMM);
}

void logFeaturesHeader() {
  Serial.println("trial,baseline_mm,max_deform_mm,time_to_max_ms,recovery_ms,peak_pressure_kpa,pressure_drop_kpa");
}

void logFeatures(int trialNumber, const Features& f) {
  Serial.print(trialNumber);
  Serial.print(",");
  Serial.print(f.baseline_mm, 2);
  Serial.print(",");
  Serial.print(f.max_deform_mm, 2);
  Serial.print(",");
  Serial.print(f.time_to_max_ms);
  Serial.print(",");
  Serial.print(f.recovery_ms);
  Serial.print(",");
  Serial.print(f.peak_pressure_kpa, 3);
  Serial.print(",");
  Serial.println(f.pressure_drop_kpa, 3);
}

void logStateTransition(const char* oldStateName, const char* newStateName,
                         const char* sensorLabel, float sensorValue) {
  Serial.print("[");
  Serial.print(millis());
  Serial.print(" ms] ");
  Serial.print(oldStateName);
  Serial.print(" -> ");
  Serial.print(newStateName);
  if (sensorLabel != nullptr) {
    Serial.print(" | ");
    Serial.print(sensorLabel);
    Serial.print("=");
    Serial.print(sensorValue, 2);
  }
  Serial.println();
}
