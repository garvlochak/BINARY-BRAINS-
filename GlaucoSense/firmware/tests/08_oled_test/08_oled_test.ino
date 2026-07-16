/*
 * GlaucoSense - Test 08: OLED Splash Screen Test
 * Owner: M4 (display / logging)
 * Purpose: Confirms the SSD1306 OLED wiring and I2C address by showing a
 *          simple splash screen.
 *
 * Requires libraries: "Adafruit SSD1306", "Adafruit GFX Library"
 *
 * NOT a medical device. Bench-test prototype only.
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// These must match the pins/address in ../../GlaucoSense_Final/config.h
const int I2C_SDA_PIN = 21;
const int I2C_SCL_PIN = 22;
const int OLED_I2C_ADDR = 0x3C;

const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println("ERROR: SSD1306 not found. Check wiring/address.");
    while (1) {
      delay(1000);
    }
  }

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

void loop() {
  // Splash screen only - nothing to update.
}
