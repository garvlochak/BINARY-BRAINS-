"""
GlaucoSense - prepare_dataset.py
Owner: M3 (ML)
Purpose: Cleans the raw per-sample trial log (data/raw_trials.csv) before
         feature extraction: drops samples with an invalid distance
         reading, drops samples with an out-of-range pressure reading, and
         drops whole trials that don't have enough samples to trust.
         Writes the result to data/raw_trials_clean.csv (the original raw
         log is left untouched).

NOT a medical device. Bench-test prototype only.
"""

import pandas as pd

RAW_CSV = "data/raw_trials.csv"
CLEAN_CSV = "data/raw_trials_clean.csv"

MIN_PRESSURE_KPA = -0.5
MAX_PRESSURE_KPA = 10.5
MIN_SAMPLES_PER_TRIAL = 30


def main():
    df = pd.read_csv(RAW_CSV)
    rows_before = len(df)

    df = df[df["distance_mm"] > 0]
    df = df[(df["pressure_kpa"] >= MIN_PRESSURE_KPA) & (df["pressure_kpa"] <= MAX_PRESSURE_KPA)]

    samples_per_trial = df.groupby("trial").size()
    good_trials = samples_per_trial[samples_per_trial >= MIN_SAMPLES_PER_TRIAL].index
    df = df[df["trial"].isin(good_trials)]

    rows_after = len(df)
    print(f"Kept {rows_after} of {rows_before} rows across {df['trial'].nunique()} trial(s)")

    df.to_csv(CLEAN_CSV, index=False)
    print(f"Wrote cleaned data to {CLEAN_CSV}")


if __name__ == "__main__":
    main()
