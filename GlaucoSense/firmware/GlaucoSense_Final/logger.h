/*
 * GlaucoSense - logger.h
 * Owner: M4 (display / logging)
 * Purpose: Trial counter + CSV logging over Serial, in the same format the
 *          machine_learning/ scripts expect in data/raw_trials.csv and
 *          data/features.csv.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#ifndef LOGGER_H
#define LOGGER_H

#include "feature_extraction.h"

// Resets the trial counter to 1. Call once in setup().
void initLogger();

// Returns the current trial number, then increments it for next time.
int nextTrialNumber();

// Prints "trial,time_ms,pressure_kpa,distance_mm" - call once at boot.
void logRawHeader();

// Prints one raw sample row, matching machine_learning/data/raw_trials.csv
void logRawSample(int trialNumber, unsigned long timeMs, float pressureKPa, int distanceMM);

// Prints the feature-row CSV header - call once at boot.
void logFeaturesHeader();

// Prints one feature row, matching machine_learning/data/features.csv
void logFeatures(int trialNumber, const Features& f);

#endif // LOGGER_H
