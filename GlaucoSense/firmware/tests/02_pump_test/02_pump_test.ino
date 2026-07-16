/*
 * GlaucoSense - Test 02: Pump Test
 * Owner: M1 (actuators / state machine)
 * Purpose: Switches the diaphragm pump on for 3 seconds and off for 3
 *          seconds, on repeat, so you can confirm the MOSFET wiring works
 *          before it's driven by the real state machine.
 *
 * SAFETY: pump is a low-side switched load through an N-MOSFET. Make sure
 * there is a flyback diode across the pump and a common ground with the
 * ESP32 (see documentation/wiring_notes.txt). The pump starts OFF.
 *
 * NOT a medical device. Bench-test prototype only.
 */

// This pin must match PUMP_PIN in ../../GlaucoSense_Final/config.h
const int PUMP_PIN = 25;

void setup() {
  Serial.begin(115200);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW); // safe state: pump off at boot
}

void loop() {
  Serial.println("Pump ON");
  digitalWrite(PUMP_PIN, HIGH);
  delay(3000);

  Serial.println("Pump OFF");
  digitalWrite(PUMP_PIN, LOW);
  delay(3000);
}
