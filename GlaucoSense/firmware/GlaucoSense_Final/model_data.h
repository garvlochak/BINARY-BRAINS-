/*
 * GlaucoSense - model_data.h
 * Owner: M3 (ML)
 * Purpose: Trained model weights used by model_inference.cpp.
 *
 * !!! IMPORTANT - READ THIS !!!
 * The numbers below were trained on SYNTHETIC (fabricated) data, generated
 * to unblock the software/ML pipeline while real hardware data collection
 * was blocked. They are NOT derived from real sensor readings and must NOT
 * be presented as validated results. Replace this whole file once real
 * trial data has been collected and a model has been trained on it - see
 * machine_learning/data/SYNTHETIC_DATA_NOTICE.txt for details.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#ifndef MODEL_DATA_H
#define MODEL_DATA_H

// Must match the number of columns in machine_learning/data/features.csv
// (excluding the "trial" column) and their order:
// baseline_mm, max_deform_mm, time_to_max_ms, recovery_ms,
// peak_pressure_kpa, pressure_drop_kpa
#define MODEL_NUM_FEATURES 6

// 0 = SOFT, 1 = NORMAL, 2 = STIFF (see CLASS_SOFT/CLASS_NORMAL/CLASS_STIFF
// in model_inference.h)
#define MODEL_NUM_CLASSES 3

// Mean and standard deviation of each feature in the training data, used to
// scale a new reading the same way before classifying it (from
// scikit-learn's StandardScaler: .mean_ and .scale_).
const float FEAT_MEAN[MODEL_NUM_FEATURES] = { 8.35566667f, 1.95100000f, 430.66666667f, 170.96666667f, 2.99200000f, 1.23211667f };
const float FEAT_STD[MODEL_NUM_FEATURES]  = { 1.62926913f, 1.21213132f, 353.16599811f, 125.76604824f, 0.08442077f, 0.21352846f };

// One row of coefficients per class, plus one intercept per class (from
// scikit-learn's LogisticRegression: .coef_ and .intercept_).
const float COEF[MODEL_NUM_CLASSES][MODEL_NUM_FEATURES] = {
  { -0.15151964f, 2.16637068f, 0.12820032f, 0.72878075f, -0.21654573f, 0.52763482f }, // CLASS_SOFT
  { 0.16975282f, -0.02607335f, -1.02200953f, -0.42317038f, 0.33133885f, -0.09457676f }, // CLASS_NORMAL
  { -0.01823318f, -2.14029733f, 0.89380921f, -0.30561037f, -0.11479311f, -0.43305806f }, // CLASS_STIFF
};
const float INTERCEPT[MODEL_NUM_CLASSES] = { -0.29850637f, 0.58703209f, -0.28852572f };

#endif // MODEL_DATA_H
