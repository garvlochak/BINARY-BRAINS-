/*
 * GlaucoSense - GlaucoSense_Final.ino
 * Owner: M1 (actuators / state machine)
 * Purpose: Main trial state machine for the GlaucoSense prototype, fully
 *          non-blocking (millis()-timed, no delay() anywhere in loop()):
 *
 *   IDLE -> EYE_DETECTED -> CHARGING -> TARGET_REACHED -> PUFF -> ACQUIRE
 *        -> INFER -> DISPLAY -> RESET -> (back to IDLE)
 *
 *          with a dedicated ERROR state reachable from anywhere.
 *          IDLE exits on EITHER the IR sensor detecting an eye/object
 *          (isEyePresent()) OR a manual start (Serial 's' or the button).
 *
 * PIN MAP (classic ESP32-WROOM-32 / DevKitC) - authoritative copy is the
 * table at the top of config.h; this is a quick reference only:
 *   I2C_SDA_PIN=21  I2C_SCL_PIN=22  PUMP_PIN=25  VALVE_PIN=26
 *   PRESSURE_ADC_PIN=34 (ADC1_CH6)  IR_SENSOR_PIN=35
 *   START_BUTTON_PIN=27  STATUS_LED_PIN=32
 * I2C addresses: VL53L0X = default (0x29), SSD1306 OLED = 0x3C
 * IR active level: LOW (IR_ACTIVE_LEVEL in config.h) - most cheap IR
 * obstacle modules pull OUT low when they detect something; flip that one
 * #define if yours is the opposite.
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
  STATE_IDLE,
  STATE_EYE_DETECTED,
  STATE_CHARGING,
  STATE_TARGET_REACHED,
  STATE_PUFF,
  STATE_ACQUIRE,
  STATE_INFER,
  STATE_DISPLAY,
  STATE_RESET,
  STATE_ERROR
};

static TrialState currentState = STATE_IDLE;
static unsigned long stateEnteredAt = 0;

static float baselineMM = 0;
static int trialNumber = 0;

// Non-blocking baseline accumulation (filled during EYE_DETECTED).
static float baselineSum = 0;
static int baselineCount = 0;

// Trial sample buffers (reused every trial) + non-blocking sampling timers.
static unsigned long trialTimeMs[MAX_TRIAL_SAMPLES];
static float trialPressureKPa[MAX_TRIAL_SAMPLES];
static int trialDistanceMM[MAX_TRIAL_SAMPLES];
static int trialSampleCount = 0;
static unsigned long recordStartAt = 0;
static unsigned long lastSampleAt = 0;

static InferenceResult lastResult;

static const char* stateName(TrialState state) {
  switch (state) {
    case STATE_IDLE:           return "IDLE";
    case STATE_EYE_DETECTED:   return "EYE_DETECTED";
    case STATE_CHARGING:       return "CHARGING";
    case STATE_TARGET_REACHED: return "TARGET_REACHED";
    case STATE_PUFF:           return "PUFF";
    case STATE_ACQUIRE:        return "ACQUIRE";
    case STATE_INFER:          return "INFER";
    case STATE_DISPLAY:        return "DISPLAY";
    case STATE_RESET:          return "RESET";
    case STATE_ERROR:          return "ERROR";
  }
  return "?";
}

// Moves to newState and always prints "[ms] OLD -> NEW | label=value" so
// Serial Monitor shows exactly what's happening, with no OLED required.
// sensorLabel/sensorValue are optional (pass nullptr/0 to omit them).
static void enterState(TrialState newState, const char* sensorLabel = nullptr, float sensorValue = 0) {
  const char* oldName = stateName(currentState);
  const char* newName = stateName(newState);
  currentState = newState;
  stateEnteredAt = millis();
  digitalWrite(STATUS_LED_PIN, newState == STATE_ERROR ? HIGH : LOW);
  logStateTransition(oldName, newName, sensorLabel, sensorValue);
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

// Takes one pressure+distance sample into the trial buffers, if there's
// room. elapsed is milliseconds since recordStartAt.
static void sampleTrialPoint(unsigned long elapsed) {
  if (trialSampleCount >= MAX_TRIAL_SAMPLES) {
    return;
  }
  trialTimeMs[trialSampleCount] = elapsed;
  trialPressureKPa[trialSampleCount] = readPressureKPa();
  trialDistanceMM[trialSampleCount] = readDistanceMM();
  trialSampleCount++;
}

void setup() {
  // SAFE STATE FIRST: force the pump and valve off before ANYTHING else -
  // even before Serial - so they can never be left running out of a
  // previous session or a mid-flash reset.
  initActuators();

  Serial.begin(115200);
  // Boot banner, repeated so it's easy to spot in the monitor and confirms
  // Serial is alive before anything else runs. Classic ESP32 uses a
  // USB-UART bridge chip (CP2102/CH340) - plain Serial.begin() is correct
  // here; there is no "USB CDC On Boot" setting for this board (that was
  // an ESP32-S3-only option from an earlier revision of this project).
  for (int i = 0; i < 3; i++) {
    Serial.println("GLAUCOSENSE BOOT OK");
    delay(200);
  }

  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  if (!initDisplay()) {
    // The OLED is optional - if it's not there, every display_manager
    // function silently does nothing, and the state machine + Serial
    // Monitor still work fully without it.
    Serial.println("No OLED found - continuing in Serial-only mode");
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

  Serial.println("Ready - waiting for eye detection or manual start ('s' or button)");
}

void loop() {
  switch (currentState) {

    case STATE_IDLE: {
      showStateScreen("IDLE");
      if (startRequested() || isEyePresent()) {
        baselineSum = 0;
        baselineCount = 0;
        trialNumber = nextTrialNumber();
        enterState(STATE_EYE_DETECTED);
      }
      break;
    }

    case STATE_EYE_DETECTED: {
      showStateScreen("EYE_DETECTED");

      if (millis() - lastSampleAt >= SAMPLE_INTERVAL_MS) {
        lastSampleAt = millis();
        int d = readDistanceMM();
        if (d >= 0) {
          baselineSum += d;
          baselineCount++;
        }
      }

      if (millis() - stateEnteredAt >= BASELINE_SAMPLE_MS) {
        if (baselineCount == 0) {
          Serial.println("ERROR: could not get a valid baseline distance reading");
          enterState(STATE_ERROR);
          break;
        }
        baselineMM = baselineSum / (float)baselineCount;
        pumpOn();
        enterState(STATE_CHARGING, "baseline_mm", baselineMM);
      }
      break;
    }

    case STATE_CHARGING: {
      showStateScreen("CHARGING");
      float pressure = readPressureKPa();

      // Hard safety ceiling - independent of the target, always checked.
      if (pressure >= PRESSURE_HARD_LIMIT_KPA) {
        pumpOff();
        Serial.print("ERROR: pressure exceeded hard safety limit (");
        Serial.print(pressure, 2);
        Serial.println(" kPa) - pump stopped immediately");
        enterState(STATE_ERROR);
        break;
      }

      if (pressure >= TARGET_PRESSURE_KPA) {
        pumpOff();
        enterState(STATE_TARGET_REACHED, "pressure", pressure);
        break;
      }

      if (millis() - stateEnteredAt >= PUMP_TIMEOUT_MS) {
        pumpOff();
        Serial.println("ERROR: pump timed out before reaching target pressure");
        enterState(STATE_ERROR);
      }
      break;
    }

    case STATE_TARGET_REACHED: {
      showStateScreen("TARGET_OK");
      if (millis() - stateEnteredAt >= STABILIZE_MS) {
        valveOpen();
        recordStartAt = millis();
        lastSampleAt = millis();
        trialSampleCount = 0;
        enterState(STATE_PUFF);
      }
      break;
    }

    case STATE_PUFF: {
      showStateScreen("PUFF");
      unsigned long elapsed = millis() - recordStartAt;

      if (millis() - lastSampleAt >= SAMPLE_INTERVAL_MS) {
        lastSampleAt = millis();
        sampleTrialPoint(elapsed);
      }

      if (elapsed >= VALVE_OPEN_MS) {
        valveClose();
        enterState(STATE_ACQUIRE);
      }
      break;
    }

    case STATE_ACQUIRE: {
      showStateScreen("ACQUIRE");
      unsigned long elapsed = millis() - recordStartAt;

      if (millis() - lastSampleAt >= SAMPLE_INTERVAL_MS) {
        lastSampleAt = millis();
        sampleTrialPoint(elapsed);
      }

      if (elapsed >= RECORD_MS || trialSampleCount >= MAX_TRIAL_SAMPLES) {
        // SAFETY: guarantee the valve is closed, even if PUFF's close was
        // somehow missed.
        valveClose();

        for (int i = 0; i < trialSampleCount; i++) {
          logRawSample(trialNumber, trialTimeMs[i], trialPressureKPa[i], trialDistanceMM[i]);
        }

        enterState(STATE_INFER);
      }
      break;
    }

    case STATE_INFER: {
      showStateScreen("INFER");

      Features f = extractFeatures(trialTimeMs, trialPressureKPa, trialDistanceMM,
                                    trialSampleCount, baselineMM);
      logFeatures(trialNumber, f);
      lastResult = runInference(f);

      showResultScreen(trialNumber, lastResult);
      Serial.print("Result: ");
      Serial.print(stiffnessClassName(lastResult.stiffnessClass));
      Serial.print(" (confidence ");
      Serial.print(lastResult.confidence * 100.0f, 0);
      Serial.println("%)");

      enterState(STATE_DISPLAY, "confidence", lastResult.confidence);
      break;
    }

    case STATE_DISPLAY: {
      if (millis() - stateEnteredAt >= RESULT_DISPLAY_MS) {
        enterState(STATE_RESET);
      }
      break;
    }

    case STATE_RESET: {
      trialSampleCount = 0;
      baselineSum = 0;
      baselineCount = 0;
      enterState(STATE_IDLE);
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
