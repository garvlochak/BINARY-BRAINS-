/*
 * GlaucoSense - model_inference.h
 * Owner: M3 (ML)
 * Purpose: Turns Features into a stiffness class + confidence.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#ifndef MODEL_INFERENCE_H
#define MODEL_INFERENCE_H

#include "feature_extraction.h"

// ===========================================================================
// !!! PLACEHOLDER MODEL - MEMBER 3 (M3), YOU MUST REPLACE THIS !!!
//
// This is just a rough guess based on deformation size, so the rest of the
// team can test the full pipeline (state machine, display, logging) while
// the real model is being trained. Once machine_learning/train_model.py has
// exported model_config.h (FEAT_MEAN, FEAT_STD, COEF, INTERCEPT), replace the
// body of runInference() below with the real logistic-regression math.
// DO NOT demo or submit the project with this placeholder still in place.
// ===========================================================================

#define CLASS_SOFT   0
#define CLASS_NORMAL 1
#define CLASS_STIFF  2

struct InferenceResult {
  int stiffnessClass;  // CLASS_SOFT, CLASS_NORMAL, or CLASS_STIFF
  float confidence;     // 0.0 - 1.0 (placeholder value, not a real probability yet)
};

inline InferenceResult runInference(const Features& f) {
  InferenceResult result;

  if (f.max_deform_mm >= 3.0f) {
    result.stiffnessClass = CLASS_SOFT;     // deforms a lot -> soft membrane
  } else if (f.max_deform_mm <= 1.0f) {
    result.stiffnessClass = CLASS_STIFF;    // barely deforms -> stiff membrane
  } else {
    result.stiffnessClass = CLASS_NORMAL;
  }

  result.confidence = 0.5f; // fixed placeholder, real model will compute this

  return result;
}

#endif // MODEL_INFERENCE_H
