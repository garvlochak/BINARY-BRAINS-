/*
 * GlaucoSense - feature_extraction.h
 * Owner: M3 (ML)
 * Purpose: Turns one trial's raw pressure/distance arrays into the small
 *          set of numbers (Features) the classifier uses.
 *          IMPORTANT: these definitions must exactly match
 *          machine_learning/extract_features.py so the model trained on
 *          PC data behaves the same way on the device.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#ifndef FEATURE_EXTRACTION_H
#define FEATURE_EXTRACTION_H

#include <math.h>

// How close (in mm) the deformation has to get back to baseline before we
// call it "recovered". Keep this equal to RECOVERY_TOLERANCE_MM in
// machine_learning/extract_features.py.
#define RECOVERY_TOLERANCE_MM 0.3f

struct Features {
  float baseline_mm;         // resting distance before the puff
  float max_deform_mm;       // largest |distance - baseline| seen during the trial
  unsigned long time_to_max_ms; // time (from start of capture) of that peak
  long recovery_ms;          // time from peak until deform <= RECOVERY_TOLERANCE_MM, -1 if never
  float peak_pressure_kpa;   // highest pressure seen during the trial
  float pressure_drop_kpa;   // peak_pressure_kpa minus the pressure at the last sample
};

// Computes Features from the raw arrays filled in by captureTrial().
// baselineMM should come from measureBaselineMM(). sampleCount is how many
// entries in timeMs/pressureKPa/distanceMM are valid. Distance samples of
// -1 (invalid ToF reading) are skipped.
inline Features extractFeatures(const unsigned long* timeMs, const float* pressureKPa,
                                 const int* distanceMM, int sampleCount, float baselineMM) {
  Features f;
  f.baseline_mm = baselineMM;
  f.max_deform_mm = 0;
  f.time_to_max_ms = 0;
  f.recovery_ms = -1;
  f.peak_pressure_kpa = -1000.0f; // overwritten by the first valid sample below
  f.pressure_drop_kpa = 0;

  int peakIndex = 0;
  float peakDeform = 0;

  for (int i = 0; i < sampleCount; i++) {
    if (pressureKPa[i] > f.peak_pressure_kpa) {
      f.peak_pressure_kpa = pressureKPa[i];
    }

    if (distanceMM[i] < 0) {
      continue; // skip invalid ToF readings
    }

    float deform = fabs((float)distanceMM[i] - baselineMM);
    if (deform > peakDeform) {
      peakDeform = deform;
      peakIndex = i;
    }
  }

  f.max_deform_mm = peakDeform;
  f.time_to_max_ms = timeMs[peakIndex];

  // Find the first sample after the peak where the membrane is back close
  // to baseline.
  for (int i = peakIndex; i < sampleCount; i++) {
    if (distanceMM[i] < 0) {
      continue;
    }
    float deform = fabs((float)distanceMM[i] - baselineMM);
    if (deform <= RECOVERY_TOLERANCE_MM) {
      f.recovery_ms = (long)(timeMs[i] - timeMs[peakIndex]);
      break;
    }
  }

  if (sampleCount > 0) {
    float lastPressure = pressureKPa[sampleCount - 1];
    f.pressure_drop_kpa = f.peak_pressure_kpa - lastPressure;
  }

  return f;
}

#endif // FEATURE_EXTRACTION_H
