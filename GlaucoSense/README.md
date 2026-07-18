# GlaucoSense

A competition prototype by team **Binary Brains** for **Makers Conclave Finale**:
a proof-of-concept system for pressure-controlled artificial-cornea response
analysis and TinyML-based stiffness estimation.

> **SAFETY WARNING:** GlaucoSense is **NOT a medical device** and does **NOT**
> measure clinical intraocular pressure (IOP). It is tested **ONLY** on an
> artificial eye / balloon membrane. **Never** use this device on, or point it
> at, a real eye.

## Folder map

```
GlaucoSense/
├── firmware/
│   ├── diagnostics/              Interactive serial-menu tool - test ONE subsystem at a time, START HERE
│   ├── tests/                    Standalone bring-up sketches, run in order 01 -> 09
│   └── GlaucoSense_Final/       The full trial state machine (the real firmware)
├── machine_learning/
│   ├── data/                    raw_trials.csv, features.csv, labels.csv
│   ├── prepare_dataset.py       Cleans raw_trials.csv
│   ├── extract_features.py      Raw samples -> per-trial features
│   └── evaluate_model.py        Held-out evaluation report (training itself is done separately)
├── documentation/
│   ├── wiring_notes.txt         Full wiring, MOSFETs, flyback diodes, safety
│   ├── pin_map.txt              Quick pin reference
│   └── errors_and_fixes.txt     Common upload/sensor problems and fixes
└── README.md                    This file
```

## Team roles

| Member | Owns |
|--------|------|
| M1 | Actuators (pump/valve) + main state machine |
| M2 | Sensors (pressure + distance) |
| M3 | Feature extraction + ML model / TinyML inference |
| M4 | Display (OLED) + Serial CSV logging |

## Hardware

- **MCU:** ESP32-WROOM-32 Dev Module (classic ESP32), Arduino framework
- **Eye/object presence:** generic 3-pin digital IR obstacle sensor (VCC/GND/OUT)
- **Distance:** VL53L0X ToF sensor, I2C, continuous mode, 20ms timing budget (~50 Hz)
- **Pressure:** MPX5010DP (0-10 kPa differential, ratiometric to 5V), through a
  12k/20k voltage divider into the ESP32 ADC
- **Display (optional):** SSD1306 OLED 128x64, I2C address 0x3C - if it's
  not wired up, the firmware detects that at boot and runs fine without it;
  state transitions and trial results are always also printed to Serial.
- **Actuators:** 370 diaphragm pump + 5V normally-closed solenoid valve, each
  switched by a logic-level N-MOSFET (low-side)

Full wiring detail, including flyback diodes and the voltage divider warning,
is in [`documentation/wiring_notes.txt`](documentation/wiring_notes.txt). Pin
numbers are defined in exactly one place:
[`firmware/GlaucoSense_Final/config.h`](firmware/GlaucoSense_Final/config.h).

## Required Arduino libraries

Install these from the Arduino Library Manager:

- **VL53L0X by Pololu**
- **Adafruit SSD1306**
- **Adafruit GFX Library**

## Board settings (Arduino IDE 2.x)

- Board: **ESP32 Dev Module**
- Upload Speed: 115200 (if a higher speed fails to upload)
- Flash Size: match your board (most classic ESP32-WROOM-32 boards are 4MB)
- Port: whichever port your board's USB-UART chip (CP2102 or CH340) shows up as

Note: unlike the ESP32-S3, this board has no native USB - there is no "USB
CDC On Boot" setting to configure.

See [`documentation/errors_and_fixes.txt`](documentation/errors_and_fixes.txt)
if the upload fails.

## How to run a trial

**Diagnostics-first: don't jump straight to the full state machine.** Prove
every component works in isolation first with `firmware/diagnostics/diagnostics.ino`
- it has a Serial menu to test the IR sensor, VL53L0X, pressure sensor, pump,
valve, and OLED one at a time, each printing raw values continuously. Only
move on to `GlaucoSense_Final` once every component checks out individually.

1. Wire everything per `documentation/wiring_notes.txt` and `pin_map.txt`
   (read the "before powering on" checklist in wiring_notes.txt first -
   voltage divider and flyback diode orientation especially).
2. Upload `firmware/diagnostics/diagnostics.ino`, open Serial Monitor at
   115200 baud, and work through tests 1-6 one at a time (send the number,
   send `m` to return to the menu). Fix anything that doesn't check out
   before moving on.
3. Optionally also run the numbered test sketches in `firmware/tests/`
   (01 through 09) for more focused single-purpose tests - see **Next steps**
   below for the full order.
4. Open `firmware/GlaucoSense_Final/GlaucoSense_Final.ino` in the Arduino IDE
   (it will pick up the other .h/.cpp files in the same folder automatically).
5. Upload it, then open the Serial Monitor at 115200 baud. You'll see
   `GLAUCOSENSE BOOT OK` printed 3 times at boot, confirming Serial is alive.
6. At boot, keep the pressure line open to atmosphere (pump/valve off) while
   the firmware calibrates its pressure zero offset.
7. Start a trial either automatically (hold an object/eye in front of the IR
   sensor - debounced, so it needs to be steady for ~50ms) or manually
   (typing `s` in Serial Monitor, or pressing the **START_BUTTON_PIN** button).
8. The non-blocking state machine runs: `IDLE -> EYE_DETECTED -> CHARGING ->
   TARGET_REACHED -> PUFF -> ACQUIRE -> INFER -> DISPLAY -> RESET -> IDLE`.
   Serial Monitor prints every state transition with a timestamp (e.g.
   `[12345 ms] CHARGING -> TARGET_REACHED | pressure=3.02`), the raw/feature
   CSV log, and the final result (class + confidence) - the OLED shows the
   same thing if one is wired up, but it's not required.
9. Copy the CSV rows printed on Serial into
   `machine_learning/data/raw_trials.csv` (and label each trial in
   `labels.csv`) to build up a training dataset.

## Machine learning pipeline

```
raw_trials.csv --(prepare_dataset.py)--> raw_trials_clean.csv
raw_trials_clean.csv --(extract_features.py)--> features.csv
features.csv + labels.csv --(evaluate_model.py)--> classification report
```

Model training itself is done separately by the team (not scripted here).
The classifier lives in three files under `firmware/GlaucoSense_Final/`:

- `model_inference.h` - interface only (struct, class constants, function declaration)
- `model_inference.cpp` - the implementation: scales features and scores
  each class with a logistic regression (the real math, not a placeholder)
- `model_data.h` - the trained model's numbers (FEAT_MEAN, FEAT_STD, COEF,
  INTERCEPT)

> **Current status:** `model_data.h` is trained on **synthetic (fabricated)
> data**, not real sensor readings - see
> [`machine_learning/data/SYNTHETIC_DATA_NOTICE.txt`](machine_learning/data/SYNTHETIC_DATA_NOTICE.txt)
> for why and what to do once real hardware data is available. The code
> path is real and correct; only the numbers need replacing.

## Next steps (in order)

1. Run `firmware/diagnostics/diagnostics.ino` and confirm every subsystem
   (IR sensor, VL53L0X, pressure sensor, pump, valve, OLED) works in
   isolation - do not skip this, especially after any rewiring.
2. Optionally, run firmware tests **01 -> 09** in `firmware/tests/`, one at
   a time, for more focused single-purpose tests:
   `01_esp32_test -> 02_pump_test -> 03_valve_test -> 04_pressure_raw_test ->
   05_pressure_calibration -> 06_tof_test -> 07_dual_sensor_test ->
   08_oled_test -> 09_ir_pump_bringup`
3. Do a reservoir/pump calibration pass - confirm the `CHARGING` state in
   `GlaucoSense_Final.ino` reaches `TARGET_PRESSURE_KPA` within
   `PUMP_TIMEOUT_MS` (and stays under `PRESSURE_HARD_LIMIT_KPA`) on your
   actual pump/reservoir setup, and adjust those values in `config.h` if
   needed.
4. Run a full trial on `GlaucoSense_Final` end-to-end on the artificial
   eye/balloon rig, and start collecting labeled data for the ML pipeline.
