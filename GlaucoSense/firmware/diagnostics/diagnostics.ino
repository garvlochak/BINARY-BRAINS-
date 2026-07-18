/*
 * GlaucoSense - diagnostics.ino
 * Owner: all members (bring-up / debugging tool)
 * Purpose: Interactive Serial-menu firmware that tests ONE subsystem at a
 *          time, printing raw values continuously, so every component can
 *          be proven working in isolation BEFORE bringing up the full
 *          non-blocking state machine in GlaucoSense_Final.ino.
 *
 *          Open Serial Monitor at 115200 baud, send a number (1-6) to pick
 *          a test, send 'm' at any time to return to the menu.
 *
 * Requires libraries: "VL53L0X by Pololu", "Adafruit SSD1306", "Adafruit GFX"
 * (only needed if you actually run tests 2 and 6 - the sketch still
 * compiles and runs the other tests without these sensors attached).
 *
 * PIN MAP (classic ESP32-WROOM-32 / DevKitC) - must match
 * firmware/GlaucoSense_Final/config.h:
 *   I2C_SDA=21  I2C_SCL=22  PUMP=25  VALVE=26
 *   PRESSURE_ADC=34 (ADC1_CH6, input-only)  IR_SENSOR=35 (input-only)
 *
 * SAFETY: pump/valve pins are forced to a safe OFF state at the very start
 * of setup(), before anything else runs. Keep the pump's output tube open/
 * safe while running test [4] - it will cycle on/off repeatedly.
 *
 * NOT a medical device. Bench-test prototype only, artificial eye / balloon
 * membrane only - never a real eye.
 */

#include <Wire.h>
#include <VL53L0X.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// These must match firmware/GlaucoSense_Final/config.h
const int I2C_SDA_PIN      = 21;
const int I2C_SCL_PIN      = 22;
const int PUMP_PIN         = 25;
const int VALVE_PIN        = 26;
const int PRESSURE_ADC_PIN = 34; // ADC1_CH6
const int IR_SENSOR_PIN    = 35;
const int OLED_I2C_ADDR    = 0x3C;

const float PRESSURE_DIVIDER_RATIO = 0.625f;

const unsigned long PRINT_INTERVAL_MS = 200; // 5x per second
const unsigned long TOGGLE_INTERVAL_MS = 1000; // 1s on / 1s off

VL53L0X tofSensor;
Adafruit_SSD1306 display(128, 64, &Wire, -1);

enum DiagMode { MODE_MENU, MODE_IR, MODE_TOF, MODE_PRESSURE, MODE_PUMP, MODE_VALVE, MODE_OLED };
DiagMode mode = MODE_MENU;

unsigned long lastActionAt = 0;
bool toggleState = false;
bool tofReady = false;
bool oledReady = false;
int oledCounter = 0;

void printMenu() {
  Serial.println();
  Serial.println("=== GlaucoSense Diagnostics ===");
  Serial.println("[1] IR sensor        - raw digitalRead(), 5x/sec");
  Serial.println("[2] VL53L0X distance - I2C scan + continuous distance_mm");
  Serial.println("[3] MPX5010DP        - raw ADC counts + converted kPa");
  Serial.println("[4] Pump             - 1s ON / 1s OFF toggle test");
  Serial.println("[5] Valve            - 1s ON / 1s OFF toggle test");
  Serial.println("[6] OLED             - live counter test pattern");
  Serial.println("Send a number to pick a test. Send 'm' any time to return here.");
  Serial.println();
}

void scanI2CBus() {
  Serial.println("Scanning I2C bus...");
  int found = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("  Found I2C device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      found++;
    }
  }
  if (found == 0) {
    Serial.println("  No I2C devices found - check wiring (SDA/SCL swapped? GND common? power on?)");
  }
}

void beginTofTest() {
  Serial.println("-- VL53L0X distance test (send 'm' to return) --");
  scanI2CBus();

  tofSensor.setTimeout(500);
  if (!tofSensor.init()) {
    Serial.println("ERROR: VL53L0X not found / init failed. Check wiring and I2C address above.");
    tofReady = false;
    return;
  }
  tofSensor.setMeasurementTimingBudget(20000);
  tofSensor.startContinuous();
  tofReady = true;
  Serial.println("distance_mm,valid");
}

void beginOledTest() {
  Serial.println("-- OLED test (send 'm' to return) --");
  oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR);
  if (!oledReady) {
    Serial.println("ERROR: SSD1306 not found at 0x3C. Check wiring/address.");
    return;
  }
  oledCounter = 0;
  Serial.println("OLED found - showing a live counter.");
}

void runIRTest() {
  if (millis() - lastActionAt < PRINT_INTERVAL_MS) return;
  lastActionAt = millis();

  int raw = digitalRead(IR_SENSOR_PIN);
  Serial.print("IR raw=");
  Serial.println(raw == HIGH ? "HIGH" : "LOW");
}

void runTofTest() {
  if (!tofReady) return;
  if (millis() - lastActionAt < PRINT_INTERVAL_MS) return;
  lastActionAt = millis();

  uint16_t distance = tofSensor.readRangeContinuousMillimeters();
  bool valid = !tofSensor.timeoutOccurred() && distance <= 1000;

  if (tofSensor.timeoutOccurred()) {
    Serial.println("ERROR: sensor timeout");
  }
  Serial.print(distance);
  Serial.print(",");
  Serial.println(valid ? "1" : "0");
}

void runPressureTest() {
  if (millis() - lastActionAt < PRINT_INTERVAL_MS) return;
  lastActionAt = millis();

  int rawCounts = analogRead(PRESSURE_ADC_PIN);
  int mv = analogReadMilliVolts(PRESSURE_ADC_PIN);
  float voltageAtPin = mv / 1000.0f;
  float sensorVoltage = voltageAtPin / PRESSURE_DIVIDER_RATIO;
  float pressureKPa = (sensorVoltage / 5.0f - 0.04f) / 0.09f;

  Serial.print("raw_counts=");
  Serial.print(rawCounts);
  Serial.print("  adc_mv=");
  Serial.print(mv);
  Serial.print("  pressure_kpa=");
  Serial.println(pressureKPa, 3);
}

void runPumpTest() {
  if (millis() - lastActionAt < TOGGLE_INTERVAL_MS) return;
  lastActionAt = millis();

  toggleState = !toggleState;
  digitalWrite(PUMP_PIN, toggleState ? HIGH : LOW);
  Serial.print("Pump pin -> ");
  Serial.println(toggleState ? "HIGH (ON)" : "LOW (OFF)");
}

void runValveTest() {
  if (millis() - lastActionAt < TOGGLE_INTERVAL_MS) return;
  lastActionAt = millis();

  toggleState = !toggleState;
  digitalWrite(VALVE_PIN, toggleState ? HIGH : LOW);
  Serial.print("Valve pin -> ");
  Serial.println(toggleState ? "HIGH (OPEN)" : "LOW (CLOSED)");
}

void runOledTest() {
  if (!oledReady) return;
  if (millis() - lastActionAt < 1000) return;
  lastActionAt = millis();

  oledCounter++;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 16);
  display.println("TEST");
  display.setTextSize(3);
  display.setCursor(0, 36);
  display.println(oledCounter);
  display.display();

  Serial.print("OLED counter: ");
  Serial.println(oledCounter);
}

void enterMode(char key) {
  switch (key) {
    case '1':
      mode = MODE_IR;
      Serial.println("-- IR sensor test (send 'm' to return) --");
      Serial.println("Watch this alongside the sensor's onboard LED to determine its active level.");
      break;
    case '2':
      mode = MODE_TOF;
      beginTofTest();
      break;
    case '3':
      mode = MODE_PRESSURE;
      Serial.println("-- MPX5010DP pressure test (send 'm' to return) --");
      break;
    case '4':
      mode = MODE_PUMP;
      toggleState = false;
      digitalWrite(PUMP_PIN, LOW);
      Serial.println("-- Pump toggle test (send 'm' to return) --");
      Serial.println("Keep the pump's output tube open/safe - it will cycle on/off every second.");
      break;
    case '5':
      mode = MODE_VALVE;
      toggleState = false;
      digitalWrite(VALVE_PIN, LOW);
      Serial.println("-- Valve toggle test (send 'm' to return) --");
      break;
    case '6':
      mode = MODE_OLED;
      beginOledTest();
      break;
    default:
      return; // unrecognised key, ignore
  }
  lastActionAt = 0; // let the first action/print happen immediately
}

void setup() {
  // SAFE STATE FIRST: pump/valve pins forced OFF before anything else.
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(VALVE_PIN, LOW);

  Serial.begin(115200);
  delay(200);
  Serial.println("GLAUCOSENSE DIAGNOSTICS BOOT OK");
  // Classic ESP32 uses a USB-UART bridge (CP2102/CH340) - plain
  // Serial.begin() over the USB cable is correct; there is no "USB CDC On
  // Boot" setting for this board.

  pinMode(IR_SENSOR_PIN, INPUT);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  printMenu();
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'm' || c == 'M') {
      mode = MODE_MENU;
      printMenu();
    } else {
      enterMode(c);
    }
  }

  switch (mode) {
    case MODE_IR:       runIRTest();       break;
    case MODE_TOF:       runTofTest();       break;
    case MODE_PRESSURE: runPressureTest(); break;
    case MODE_PUMP:      runPumpTest();      break;
    case MODE_VALVE:     runValveTest();     break;
    case MODE_OLED:      runOledTest();      break;
    case MODE_MENU:
    default:
      break;
  }
}
