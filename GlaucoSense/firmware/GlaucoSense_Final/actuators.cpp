/*
 * GlaucoSense - actuators.cpp
 * Owner: M1 (actuators / state machine)
 * Purpose: Implements pump and valve control. Pump and solenoid valve are
 *          each switched by a low-side N-MOSFET (see documentation/wiring_notes.txt
 *          for flyback diode + common ground requirements).
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#include "actuators.h"
#include "config.h"
#include "sensors.h"

void initActuators() {
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(VALVE_PIN, OUTPUT);
  // Safe state first: force both actuators off before anything else runs.
  allOff();
}

void pumpOn() {
  digitalWrite(PUMP_PIN, HIGH);
}

void pumpOff() {
  digitalWrite(PUMP_PIN, LOW);
}

void valveOpen() {
  digitalWrite(VALVE_PIN, HIGH);
}

void valveClose() {
  digitalWrite(VALVE_PIN, LOW);
}

void allOff() {
  pumpOff();
  valveClose();
}

bool chargeReservoir(float targetKPa, unsigned long timeoutMs) {
  unsigned long start = millis();
  bool reachedTarget = false;

  pumpOn();

  while (millis() - start < timeoutMs) {
    if (readPressureKPa() >= targetKPa) {
      reachedTarget = true;
      break;
    }
    delay(SAMPLE_INTERVAL_MS);
  }

  // SAFETY: this line must always run, whether we broke out because we hit
  // the target pressure or because the while loop timed out. The pump must
  // never be left running.
  pumpOff();

  return reachedTarget;
}
