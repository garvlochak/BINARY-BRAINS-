/*
 * GlaucoSense - Test 04: Raw Pressure Reading
 * Owner: M2 (sensors)
 * Purpose: Reads the MPX5010DP pressure sensor with no zero-offset or
 *          filtering applied, and prints it as CSV so it can be checked
 *          against a known pressure or just watched for noise.
 *
 * SAFETY: MPX5010DP Vout must go through the 12k/20k voltage divider before
 * reaching this GPIO - never connect Vout directly to the ESP32 ADC pin.
 *
 * NOT a medical device. Bench-test prototype only.
 */

// This pin must match PRESSURE_ADC_PIN in ../../GlaucoSense_Final/config.h
const int PRESSURE_ADC_PIN = 34; // ADC1_CH6 (input-only pin, fine for analog reading)

// Voltage divider ratio (Rtop=12k, Rbottom=20k) between the sensor and this pin.
const float DIVIDER_RATIO = 0.625f;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  Serial.println("adc_millivolts,sensor_voltage_v,pressure_kpa");
}

void loop() {
  int mv = analogReadMilliVolts(PRESSURE_ADC_PIN);
  float voltageAtPin = mv / 1000.0f;

  // Undo the voltage divider to get the sensor's real Vout.
  float sensorVoltage = voltageAtPin / DIVIDER_RATIO;

  // MPX5010DP: Vout = 5.0 * (0.09*P + 0.04), solved for P (kPa):
  float pressureKPa = (sensorVoltage / 5.0f - 0.04f) / 0.09f;

  Serial.print(mv);
  Serial.print(",");
  Serial.print(sensorVoltage, 3);
  Serial.print(",");
  Serial.println(pressureKPa, 3);

  delay(100);
}
