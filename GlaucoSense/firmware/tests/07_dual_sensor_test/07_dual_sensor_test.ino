/*
 * GlaucoSense - Test 07: Dual Sensor Test
 * Owner: M2 (sensors)
 * Purpose: Reads pressure and distance together at ~50Hz and prints them
 *          as one CSV stream, to confirm both sensors can be sampled at
 *          the same time without slowing each other down.
 *          CSV format: time_ms,pressure_kpa,distance_mm
 *
 * Requires library: "VL53L0X by Pololu"
 *
 * NOT a medical device. Bench-test prototype only.
 */

#include <Wire.h>
#include <VL53L0X.h>

// These must match the pins in ../../GlaucoSense_Final/config.h
const int I2C_SDA_PIN = 21;
const int I2C_SCL_PIN = 22;
const int PRESSURE_ADC_PIN = 34; // ADC1_CH6
const float DIVIDER_RATIO = 0.625f;

// This must match SAMPLE_INTERVAL_MS in config.h (20ms = ~50 Hz)
const unsigned long SAMPLE_INTERVAL_MS = 20;

VL53L0X tofSensor;
unsigned long nextSampleTime = 0;

float readPressureKPa() {
  int mv = analogReadMilliVolts(PRESSURE_ADC_PIN);
  float voltageAtPin = mv / 1000.0f;
  float sensorVoltage = voltageAtPin / DIVIDER_RATIO;
  return (sensorVoltage / 5.0f - 0.04f) / 0.09f;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  tofSensor.setTimeout(500);
  if (!tofSensor.init()) {
    Serial.println("ERROR: VL53L0X not found. Check wiring.");
    while (1) {
      delay(1000);
    }
  }
  tofSensor.setMeasurementTimingBudget(20000);
  tofSensor.startContinuous();

  Serial.println("time_ms,pressure_kpa,distance_mm");
}

void loop() {
  // Use millis()-based scheduling (instead of delay()) so both sensors are
  // sampled on a steady ~50Hz clock rather than drifting slower over time.
  if (millis() >= nextSampleTime) {
    nextSampleTime += SAMPLE_INTERVAL_MS;

    float pressure = readPressureKPa();

    uint16_t distance = tofSensor.readRangeContinuousMillimeters();
    if (tofSensor.timeoutOccurred() || distance > 1000) {
      distance = 0; // 0 marks an invalid reading in this log
    }

    Serial.print(millis());
    Serial.print(",");
    Serial.print(pressure, 3);
    Serial.print(",");
    Serial.println(distance);
  }
}
