/*
 * GlaucoSense - model_inference.cpp
 * Owner: M3 (ML)
 * Purpose: Implements runInference() - turns Features into a stiffness
 *          class + confidence using the trained logistic regression
 *          weights in model_data.h.
 *
 * !!! IMPORTANT - READ THIS !!!
 * model_data.h currently holds weights trained on SYNTHETIC (fabricated)
 * data, not real sensor readings - see model_data.h and
 * machine_learning/data/SYNTHETIC_DATA_NOTICE.txt. The math below is real
 * and ready to go; only the numbers in model_data.h need replacing once
 * real trial data has been collected and trained on.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#include "model_inference.h"
#include "model_data.h"
#include <math.h>

// Features struct -> plain array, in the exact order model_data.h expects
// (must match FEATURE_COLUMNS in machine_learning/evaluate_model.py).
static void featuresToArray(const Features& f, float* out) {
  out[0] = f.baseline_mm;
  out[1] = f.max_deform_mm;
  out[2] = (float)f.time_to_max_ms;
  out[3] = (float)f.recovery_ms;
  out[4] = f.peak_pressure_kpa;
  out[5] = f.pressure_drop_kpa;
}

InferenceResult runInference(const Features& f) {
  float x[MODEL_NUM_FEATURES];
  featuresToArray(f, x);

  // Scale each feature the same way it was scaled during training.
  float scaled[MODEL_NUM_FEATURES];
  for (int i = 0; i < MODEL_NUM_FEATURES; i++) {
    scaled[i] = (x[i] - FEAT_MEAN[i]) / FEAT_STD[i];
  }

  // Score each class: score = intercept + dot(coefficients, scaled features).
  float scores[MODEL_NUM_CLASSES];
  for (int c = 0; c < MODEL_NUM_CLASSES; c++) {
    float score = INTERCEPT[c];
    for (int i = 0; i < MODEL_NUM_FEATURES; i++) {
      score += COEF[c][i] * scaled[i];
    }
    scores[c] = score;
  }

  // The winning class is whichever score is highest.
  int bestClass = 0;
  for (int c = 1; c < MODEL_NUM_CLASSES; c++) {
    if (scores[c] > scores[bestClass]) {
      bestClass = c;
    }
  }

  // Softmax turns the scores into a 0-1 confidence for the winning class.
  float sumExp = 0;
  for (int c = 0; c < MODEL_NUM_CLASSES; c++) {
    sumExp += exp(scores[c] - scores[bestClass]);
  }

  InferenceResult result;
  result.stiffnessClass = bestClass;
  result.confidence = 1.0f / sumExp;
  return result;
}
