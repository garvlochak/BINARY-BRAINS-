/*
 * GlaucoSense - display_manager.cpp
 * Owner: M4 (display / logging)
 * Purpose: Draws every screen shown on the 128x64 SSD1306 OLED.
 *          The OLED is OPTIONAL - if initDisplay() doesn't find one, every
 *          function below silently does nothing instead of crashing/hanging,
 *          so the rest of the firmware (state machine, actuators, Serial
 *          logging) works fine with no screen wired up at all.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#include "display_manager.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
static bool displayFound = false;

bool initDisplay() {
  displayFound = display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR);
  return displayFound;
}

void showSplashScreen() {
  if (!displayFound) return;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 16);
  display.println("GlaucoSense");

  display.setTextSize(1);
  display.setCursor(0, 44);
  display.println("Binary Brains");

  display.display();
}

void showStateScreen(const char* stateName) {
  if (!displayFound) return;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("GlaucoSense");
  display.drawLine(0, 10, SCREEN_WIDTH - 1, 10, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 24);
  display.println(stateName);

  display.display();
}

void showLiveValues(float pressureKPa, int distanceMM) {
  if (!displayFound) return;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Recording...");

  display.setCursor(0, 20);
  display.print("Pressure: ");
  display.print(pressureKPa, 2);
  display.println(" kPa");

  display.setCursor(0, 34);
  display.print("Distance: ");
  if (distanceMM >= 0) {
    display.print(distanceMM);
    display.println(" mm");
  } else {
    display.println("invalid");
  }

  display.display();
}

void showResultScreen(int trialNumber, const InferenceResult& result) {
  if (!displayFound) return;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Trial #");
  display.println(trialNumber);

  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println(stiffnessClassName(result.stiffnessClass));

  display.setTextSize(1);
  display.setCursor(0, 48);
  display.print("Confidence: ");
  display.print(result.confidence * 100.0f, 0);
  display.println("%");

  display.display();
}

void showErrorScreen(const char* message) {
  if (!displayFound) return;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("ERROR");

  display.setTextSize(1);
  display.setCursor(0, 24);
  display.println(message);

  display.display();
}
