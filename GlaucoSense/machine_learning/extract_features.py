"""
GlaucoSense - extract_features.py
Owner: M3 (ML)
Purpose: Turns a cleaned per-sample trial log (data/raw_trials_clean.csv,
         produced by prepare_dataset.py) into one feature row per trial
         (data/features.csv). Feature definitions MUST match
         firmware/GlaucoSense_Final/feature_extraction.h exactly, so a model
         trained on this data behaves the same way once it runs on the
         ESP32.

NOT a medical device. Bench-test prototype only.
"""

import pandas as pd

RAW_CSV = "data/raw_trials_clean.csv"
FEATURES_CSV = "data/features.csv"

# Keep this equal to RECOVERY_TOLERANCE_MM in feature_extraction.h
RECOVERY_TOLERANCE_MM = 0.3

# raw_trials_clean.csv has no separate baseline reading (on the device that
# comes from measureBaselineMM(), called before recording starts), so here
# we approximate it with the first samples of each trial, before the
# puff-induced deformation has had time to develop.
BASELINE_WINDOW_MS = 100


def extract_features_for_trial(trial_df):
    trial_df = trial_df.sort_values("time_ms").reset_index(drop=True)

    baseline_rows = trial_df[trial_df["time_ms"] < BASELINE_WINDOW_MS]
    if len(baseline_rows) == 0:
        baseline_rows = trial_df.iloc[[0]]
    baseline_mm = baseline_rows["distance_mm"].mean()

    deform = (trial_df["distance_mm"] - baseline_mm).abs()
    peak_index = deform.idxmax()

    max_deform_mm = deform.loc[peak_index]
    time_to_max_ms = trial_df.loc[peak_index, "time_ms"]

    recovery_ms = -1
    after_peak = trial_df.loc[peak_index:]
    recovered = after_peak[(after_peak["distance_mm"] - baseline_mm).abs() <= RECOVERY_TOLERANCE_MM]
    if len(recovered) > 0:
        recovery_ms = recovered.iloc[0]["time_ms"] - time_to_max_ms

    peak_pressure_kpa = trial_df["pressure_kpa"].max()
    pressure_drop_kpa = peak_pressure_kpa - trial_df.iloc[-1]["pressure_kpa"]

    return {
        "baseline_mm": round(float(baseline_mm), 2),
        "max_deform_mm": round(float(max_deform_mm), 2),
        "time_to_max_ms": int(time_to_max_ms),
        "recovery_ms": int(recovery_ms),
        "peak_pressure_kpa": round(float(peak_pressure_kpa), 3),
        "pressure_drop_kpa": round(float(pressure_drop_kpa), 3),
    }


def main():
    raw = pd.read_csv(RAW_CSV)

    rows = []
    for trial, trial_df in raw.groupby("trial"):
        features = extract_features_for_trial(trial_df)
        features["trial"] = trial
        rows.append(features)

    columns = ["trial", "baseline_mm", "max_deform_mm", "time_to_max_ms",
               "recovery_ms", "peak_pressure_kpa", "pressure_drop_kpa"]
    features_df = pd.DataFrame(rows)[columns]
    features_df.to_csv(FEATURES_CSV, index=False)
    print(f"Wrote {len(features_df)} trial(s) of features to {FEATURES_CSV}")


if __name__ == "__main__":
    main()
