/*
 * GlaucoSense - display_manager.h
 * Owner: M4 (display / logging)
 * Purpose: All the screens shown on the SSD1306 OLED.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include "feature_extraction.h"
#include "model_inference.h"

// Starts the OLED over I2C. Returns true if the display was found.
bool initDisplay();

void showSplashScreen();                                  // "GlaucoSense / Binary Brains"
void showStateScreen(const char* stateName);               // big text of the current state
void showLiveValues(float pressureKPa, int distanceMM);     // live sensor readout
void showResultScreen(int trialNumber, const InferenceResult& result); // class + confidence
void showErrorScreen(const char* message);

#endif // DISPLAY_MANAGER_H
