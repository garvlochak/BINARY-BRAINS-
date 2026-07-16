/*
 * GlaucoSense - Test 05: Pressure Zero-Offset Calibration
 * Owner: M2 (sensors)
 * Purpose: At boot, averages 50 samples with the system open to atmosphere
 *          to find the pressure sensor's zero offset. Then continuously
 *          applies a 5-point moving average filter and prints raw vs
 *          calibrated pressure as CSV.
 *
 * SAFETY: keep the pump and valve off and the pressure line open to air
 * while this sketch calibrates at boot.
 *
 * NOT a medical device. Bench-test prototype only.
 */

// This pin must match PRESSURE_ADC_PIN in ../../GlaucoSense_Final/config.h
const int PRESSURE_ADC_PIN = 34; // ADC1_CH6
const float DIVIDER_RATIO = 0.625f;

// These must match PRESSURE_ZERO_SAMPLES and PRESSURE_FILTER_SAMPLES in config.h
const int ZERO_SAMPLES = 50;
const int FILTER_SIZE = 5;

float zeroOffsetKPa = 0.0f;
float filterBuf[FILTER_SIZE];
int filterIndex = 0;
bool filterFilled = false;

float readRawPressureKPa() {
  int mv = analogReadMilliVolts(PRESSURE_ADC_PIN);
  float voltageAtPin = mv / 1000.0f;
  float sensorVoltage = voltageAtPin / DIVIDER_RATIO;
  return (sensorVoltage / 5.0f - 0.04f) / 0.09f;
}

// Simple ring-buffer moving average.
float movingAverage(float newSample) {
  filterBuf[filterIndex] = newSample;
  filterIndex = (filterIndex + 1) % FILTER_SIZE;
  if (filterIndex == 0) {
    filterFilled = true;
  }

  int count = filterFilled ? FILTER_SIZE : filterIndex;
  if (count == 0) {
    return newSample;
  }

  float sum = 0;
  for (int i = 0; i < count; i++) {
    sum += filterBuf[i];
  }
  return sum / count;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  Serial.println("Calibrating zero offset - keep pressure at 0 kPa (pump/valve off, open to air)");
  float sum = 0;
  for (int i = 0; i < ZERO_SAMPLES; i++) {
    sum += readRawPressureKPa();
    delay(20);
  }
  zeroOffsetKPa = sum / ZERO_SAMPLES;

  Serial.print("Zero offset (kPa): ");
  Serial.println(zeroOffsetKPa, 4);

  Serial.println("raw_kpa,filtered_kpa,calibrated_kpa");
}

void loop() {
  float raw = readRawPressureKPa();
  float filtered = movingAverage(raw);
  float calibrated = filtered - zeroOffsetKPa;

  Serial.print(raw, 4);
  Serial.print(",");
  Serial.print(filtered, 4);
  Serial.print(",");
  Serial.println(calibrated, 4);

  delay(20);
}
