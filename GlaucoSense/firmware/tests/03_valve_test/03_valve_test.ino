/*
 * GlaucoSense - Test 03: Valve Test
 * Owner: M1 (actuators / state machine)
 * Purpose: Sends the valve a single 80ms open pulse every time the letter
 *          'o' is received over Serial. Confirms the solenoid valve MOSFET
 *          wiring before the real state machine drives it.
 *
 * SAFETY: the valve is normally-closed (NC) - de-energized = closed. It is
 * a low-side switched load through an N-MOSFET; make sure there is a
 * flyback diode across it and a common ground with the ESP32
 * (see documentation/wiring_notes.txt). The valve starts closed.
 *
 * NOT a medical device. Bench-test prototype only.
 */

// These must match VALVE_PIN and VALVE_OPEN_MS in ../../GlaucoSense_Final/config.h
const int VALVE_PIN = 26;
const unsigned long VALVE_OPEN_MS = 80;

void setup() {
  Serial.begin(115200);
  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(VALVE_PIN, LOW); // safe state: valve closed at boot
  Serial.println("Send 'o' to pulse the valve open for 80ms");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'o') {
      Serial.println("Valve OPEN");
      digitalWrite(VALVE_PIN, HIGH);
      delay(VALVE_OPEN_MS);
      digitalWrite(VALVE_PIN, LOW); // always close again after the pulse
      Serial.println("Valve CLOSED");
    }
  }
}
