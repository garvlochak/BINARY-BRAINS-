/*
 * GlaucoSense - Test 06: VL53L0X Distance Test
 * Owner: M2 (sensors)
 * Purpose: Reads the VL53L0X ToF sensor in continuous mode (~50Hz) and
 *          prints CSV distance readings. Flags sensor timeouts and any
 *          reading over 1000mm as invalid instead of trusting them.
 *
 * Requires library: "VL53L0X by Pololu"
 *
 * NOT a medical device. Bench-test prototype only.
 */

#include <Wire.h>
#include <VL53L0X.h>

// These must match I2C_SDA_PIN / I2C_SCL_PIN in ../../GlaucoSense_Final/config.h
const int I2C_SDA_PIN = 21;
const int I2C_SCL_PIN = 22;

// This must match TOF_MAX_VALID_MM in config.h
const int TOF_MAX_VALID_MM = 1000;

VL53L0X sensor;

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  sensor.setTimeout(500);
  if (!sensor.init()) {
    Serial.println("ERROR: VL53L0X not found. Check wiring.");
    while (1) {
      delay(1000);
    }
  }

  sensor.setMeasurementTimingBudget(20000); // 20ms budget, ~50 Hz
  sensor.startContinuous();

  Serial.println("distance_mm,valid");
}

void loop() {
  uint16_t distance = sensor.readRangeContinuousMillimeters();
  bool valid = true;

  if (sensor.timeoutOccurred()) {
    Serial.println("ERROR: sensor timeout");
    valid = false;
  }
  if (distance > TOF_MAX_VALID_MM) {
    valid = false;
  }

  Serial.print(distance);
  Serial.print(",");
  Serial.println(valid ? "1" : "0");
}
