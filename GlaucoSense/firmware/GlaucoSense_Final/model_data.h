/*
 * GlaucoSense - model_data.h
 * Owner: M3 (ML)
 * Purpose: Landing spot for your trained model's exported numbers.
 *          NOT USED YET - model_inference.cpp still runs the placeholder
 *          threshold classifier. Once you've trained your own model
 *          (using machine_learning/features.csv + labels.csv, however you
 *          like), fill in the arrays below and update model_inference.cpp
 *          to use them instead of the threshold rules.
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

// TODO(M3): fill in with your trained model's numbers once training is done.
// Mean and standard deviation of each feature in the training data, used to
// scale a new reading the same way before classifying it (e.g. from
// scikit-learn's StandardScaler: .mean_ and .scale_).
const float FEAT_MEAN[MODEL_NUM_FEATURES] = { 0, 0, 0, 0, 0, 0 };
const float FEAT_STD[MODEL_NUM_FEATURES]  = { 1, 1, 1, 1, 1, 1 };

// TODO(M3): fill in with your trained model's numbers once training is done.
// One row of coefficients per class, plus one intercept per class
// (e.g. from scikit-learn's LogisticRegression: .coef_ and .intercept_).
const float COEF[MODEL_NUM_CLASSES][MODEL_NUM_FEATURES] = {
  { 0, 0, 0, 0, 0, 0 }, // CLASS_SOFT
  { 0, 0, 0, 0, 0, 0 }, // CLASS_NORMAL
  { 0, 0, 0, 0, 0, 0 }, // CLASS_STIFF
};
const float INTERCEPT[MODEL_NUM_CLASSES] = { 0, 0, 0 };

#endif // MODEL_DATA_H
