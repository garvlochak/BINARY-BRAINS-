/*
 * GlaucoSense - model_inference.h
 * Owner: M3 (ML)
 * Purpose: Interface for turning Features into a stiffness class +
 *          confidence. See model_inference.cpp for the implementation and
 *          model_data.h for where the trained model's numbers go.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#ifndef MODEL_INFERENCE_H
#define MODEL_INFERENCE_H

#include "feature_extraction.h"

#define CLASS_SOFT   0
#define CLASS_NORMAL 1
#define CLASS_STIFF  2

struct InferenceResult {
  int stiffnessClass;  // CLASS_SOFT, CLASS_NORMAL, or CLASS_STIFF
  float confidence;     // 0.0 - 1.0
};

InferenceResult runInference(const Features& f);

#endif // MODEL_INFERENCE_H
