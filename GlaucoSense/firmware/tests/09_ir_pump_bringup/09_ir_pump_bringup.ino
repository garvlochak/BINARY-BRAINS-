/*
 * GlaucoSense - Bring-up Test: Generic IR Sensor + Pump
 * Owner: M1 (actuators / state machine)
 * Purpose: TEMPORARY bring-up sketch for getting the pump reliably switching
 *          in response to a generic 3-pin IR obstacle sensor (VCC/GND/OUT,
 *          onboard LED + sensitivity dial). This is NOT the VL53L0X ToF
 *          sensor used in the final GlaucoSense_Final firmware - it's a
 *          simpler digital HIGH/LOW presence sensor, used here only because
 *          the pressure sensor + solenoid valve aren't wired in yet.
 *          Once those are ready, go back to the official 01-08 test
 *          sketches and GlaucoSense_Final - this file is a stepping stone,
 *          not part of the final architecture.
 *
 * SAFETY: pump is a low-side switched load through an N-MOSFET. Make sure
 * there is a flyback diode across the pump and a common ground with the
 * ESP32 (see documentation/wiring_notes.txt). The pump starts OFF.
 *
 * NOT a medical device. Bench-test prototype only.
 */

const int IR_SENSOR_PIN = 33; // generic IR module OUT pin
const int PUMP_PIN = 25;      // matches PUMP_PIN in GlaucoSense_Final/config.h

// Most cheap IR obstacle-sensor modules pull OUT LOW when they detect
// something in range (and light their onboard LED at the same time), and
// sit HIGH when nothing is in range. If the pump behaves backwards from
// what you expect (e.g. it runs when nothing is in front of the sensor,
// or stops when the LED is lit), flip this to false and re-upload.
const bool IR_ACTIVE_LOW = true;

void setup() {
  Serial.begin(115200);
  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW); // safe state: pump off at boot

  Serial.println("Watch this alongside the sensor's onboard LED.");
  Serial.println("raw_pin,object_detected,pump_state");
}

void loop() {
  int raw = digitalRead(IR_SENSOR_PIN);
  bool objectDetected = IR_ACTIVE_LOW ? (raw == LOW) : (raw == HIGH);

  digitalWrite(PUMP_PIN, objectDetected ? HIGH : LOW);

  Serial.print(raw == HIGH ? "HIGH" : "LOW");
  Serial.print(",");
  Serial.print(objectDetected ? "DETECTED" : "clear");
  Serial.print(",");
  Serial.println(objectDetected ? "PUMP ON" : "pump off");

  delay(200);
}
