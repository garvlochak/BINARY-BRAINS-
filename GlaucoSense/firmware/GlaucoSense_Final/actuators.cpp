/*
 * GlaucoSense - actuators.cpp
 * Owner: M1 (actuators / state machine)
 * Purpose: Implements pump and valve control. Pump and solenoid valve are
 *          each switched by a low-side N-MOSFET (see documentation/wiring_notes.txt
 *          for flyback diode + common ground requirements). Every function
 *          here is a single, immediate pin write - no timing/polling logic -
 *          so it's safe to call from a non-blocking state machine.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#include "actuators.h"
#include "config.h"

void initActuators() {
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(VALVE_PIN, OUTPUT);
  // SAFE STATE FIRST: force both actuators off immediately, before anything
  // else in the program runs. A physical ~10k pull-down resistor on each
  // MOSFET gate is still required (human wiring check) so the pump/valve
  // can't switch on from a floating gate before this line executes.
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
