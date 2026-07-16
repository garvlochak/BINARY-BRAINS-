/*
 * GlaucoSense - GlaucoSense_Final.ino
 * Owner: M1 (actuators / state machine)
 * Purpose: Main trial state machine for the GlaucoSense prototype.
 *          READY -> BASELINE -> PUMPING -> STABILIZE -> RECORD -> ANALYZE -> READY
 *          with an ERROR state reachable from anywhere something goes wrong.
 *
 * SAFETY WARNING: This device is a competition proof-of-concept for
 * pressure-controlled artificial-cornea response analysis and TinyML-based
 * stiffness estimation. It is NOT a medical device and does NOT measure
 * clinical intraocular pressure (IOP). Test ONLY on an artificial eye /
 * balloon membrane. NEVER point this device at, or use it on, a real eye.
 *
 * Team: Binary Brains - Makers Conclave Finale
 */

#include "config.h"
#include "sensors.h"
#include "actuators.h"
#include "feature_extraction.h"
#include "model_inference.h"
#include "display_manager.h"
#include "logger.h"

enum TrialState {
  STATE_READY,
  STATE_BASELINE,
  STATE_PUMPING,
  STATE_STABILIZE,
  STATE_RECORD,
  STATE_ANALYZE,
  STATE_ERROR
};

static TrialState currentState = STATE_READY;
static unsigned long stateEnteredAt = 0;

static float baselineMM = 0;
static int trialNumber = 0;

// Buffers for one trial's worth of samples (reused every trial).
static unsigned long trialTimeMs[MAX_TRIAL_SAMPLES];
static float trialPressureKPa[MAX_TRIAL_SAMPLES];
static int trialDistanceMM[MAX_TRIAL_SAMPLES];
static int trialSampleCount = 0;

static void enterState(TrialState newState) {
  currentState = newState;
  stateEnteredAt = millis();
  // Light the status LED whenever we are in the ERROR state, off otherwise.
  digitalWrite(STATUS_LED_PIN, newState == STATE_ERROR ? HIGH : LOW);
}

// Returns true once, the moment a trial start is requested over Serial ('s')
// or by pressing the start button (active LOW because of INPUT_PULLUP).
static bool startRequested() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 's' || c == 'S') {
      return true;
    }
  }
  if (digitalRead(START_BUTTON_PIN) == LOW) {
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);

  // SAFE STATE FIRST: force the pump and valve off before anything else
  // in setup() has a chance to run.
  initActuators();

  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  if (!initDisplay()) {
    Serial.println("ERROR: OLED not found, check wiring");
  }
  showSplashScreen();
  delay(1500);

  if (!initSensors()) {
    Serial.println("ERROR: VL53L0X not found, check wiring");
    enterState(STATE_ERROR);
    return;
  }

  Serial.println("Calibrating pressure zero offset - keep the system open to air...");
  calibratePressureZero();

  initLogger();
  logRawHeader();
  logFeaturesHeader();

  enterState(STATE_READY);
}

void loop() {
  switch (currentState) {

    case STATE_READY: {
      showStateScreen("READY");
      if (startRequested()) {
        trialNumber = nextTrialNumber();
        enterState(STATE_BASELINE);
      }
      break;
    }

    case STATE_BASELINE: {
      showStateScreen("BASELINE");
      baselineMM = measureBaselineMM();
      if (baselineMM < 0) {
        Serial.println("ERROR: could not get a valid baseline distance reading");
        enterState(STATE_ERROR);
        break;
      }
      enterState(STATE_PUMPING);
      break;
    }

    case STATE_PUMPING: {
      showStateScreen("PUMPING");
      bool reachedTarget = chargeReservoir(TARGET_PRESSURE_KPA, PUMP_TIMEOUT_MS);
      if (!reachedTarget) {
        Serial.println("ERROR: pump timed out before reaching target pressure");
        enterState(STATE_ERROR);
        break;
      }
      enterState(STATE_STABILIZE);
      break;
    }

    case STATE_STABILIZE: {
      showStateScreen("STABILIZE");
      if (millis() - stateEnteredAt >= STABILIZE_MS) {
        enterState(STATE_RECORD);
      }
      break;
    }

    case STATE_RECORD: {
      showStateScreen("RECORD");

      // Open the valve for one VALVE_OPEN_MS puff right at the start of the
      // recording window (offset 0), while sampling pressure + distance.
      captureTrial(trialTimeMs, trialPressureKPa, trialDistanceMM,
                   MAX_TRIAL_SAMPLES, &trialSampleCount,
                   RECORD_MS, 0, VALVE_OPEN_MS);

      // Extra safety: make sure the valve is closed before moving on.
      valveClose();

      for (int i = 0; i < trialSampleCount; i++) {
        logRawSample(trialNumber, trialTimeMs[i], trialPressureKPa[i], trialDistanceMM[i]);
      }

      enterState(STATE_ANALYZE);
      break;
    }

    case STATE_ANALYZE: {
      showStateScreen("ANALYZE");

      Features f = extractFeatures(trialTimeMs, trialPressureKPa, trialDistanceMM,
                                    trialSampleCount, baselineMM);
      logFeatures(trialNumber, f);

      InferenceResult result = runInference(f);
      showResultScreen(trialNumber, result);

      delay(3000); // give the user time to read the result screen
      enterState(STATE_READY);
      break;
    }

    case STATE_ERROR: {
      showErrorScreen("Check wiring, then reset board");
      // SAFETY: never leave the pump/valve running while stuck in ERROR.
      allOff();
      break;
    }
  }
}
