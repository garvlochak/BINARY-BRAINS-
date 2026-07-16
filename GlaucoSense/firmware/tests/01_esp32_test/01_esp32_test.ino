/*
 * GlaucoSense - Test 01: ESP32 Alive Check
 * Owner: M1 (actuators / state machine)
 * Purpose: Confirms the ESP32 board, USB serial, and status LED wiring
 *          all work before any sensors or actuators are connected.
 *          Blinks the status LED and prints "ALIVE" over serial every
 *          half second.
 *
 * NOT a medical device. Bench-test prototype only.
 */

// This pin must match STATUS_LED_PIN in ../../GlaucoSense_Final/config.h
const int STATUS_LED_PIN = 32;

void setup() {
  Serial.begin(115200);
  pinMode(STATUS_LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(STATUS_LED_PIN, HIGH);
  Serial.println("ALIVE");
  delay(500);

  digitalWrite(STATUS_LED_PIN, LOW);
  delay(500);
}
