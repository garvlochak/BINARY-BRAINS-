/*
 * GlaucoSense - actuators.h
 * Owner: M1 (actuators / state machine)
 * Purpose: Controls the pump and valve MOSFETs. Every function here is
 *          written so the pump and valve can NEVER be left running by
 *          accident - see actuators.cpp for the safety comments.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <Arduino.h>

// Sets PUMP_PIN and VALVE_PIN as outputs and forces them both OFF.
// Call this first thing in setup(), before anything else touches the pins.
void initActuators();

void pumpOn();
void pumpOff();

void valveOpen();
void valveClose();

// Forces pump off and valve closed. Used at boot and whenever the system
// enters the ERROR state.
void allOff();

#endif // ACTUATORS_H
