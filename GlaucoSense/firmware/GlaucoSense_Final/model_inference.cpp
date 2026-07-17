/*
 * GlaucoSense - model_inference.cpp
 * Owner: M3 (ML)
 * Purpose: Implements runInference() - turns Features into a stiffness
 *          class + confidence.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#include "model_inference.h"

// ===========================================================================
// !!! PLACEHOLDER MODEL - MEMBER 3 (M3), YOU MUST REPLACE THIS !!!
//
// This is just a rough guess based on deformation size, so the rest of the
// team can test the full pipeline (state machine, display, logging) while
// the real model is being trained separately. Once you've trained your own
// model, fill in its numbers in model_data.h and rewrite the body of
// runInference() below to do the real math (scale features using
// FEAT_MEAN/FEAT_STD, then score each class with COEF/INTERCEPT and pick
// the highest score) instead of these threshold rules.
// DO NOT demo or submit the project with this placeholder still in place.
// ===========================================================================

InferenceResult runInference(const Features& f) {
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
