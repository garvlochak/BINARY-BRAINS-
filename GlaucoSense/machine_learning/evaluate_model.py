"""
GlaucoSense - evaluate_model.py
Owner: M3 (ML)
Purpose: Held-out train/test split evaluation of the stiffness classifier -
         prints a classification report and confusion matrix so the team
         can see per-class accuracy, not just one overall score.

NOT a medical device. Bench-test prototype only.
"""

import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import classification_report, confusion_matrix

FEATURES_CSV = "data/features.csv"
LABELS_CSV = "data/labels.csv"

FEATURE_COLUMNS = [
    "baseline_mm", "max_deform_mm", "time_to_max_ms",
    "recovery_ms", "peak_pressure_kpa", "pressure_drop_kpa",
]
CLASS_NAMES = ["SOFT", "NORMAL", "STIFF"]


def main():
    features = pd.read_csv(FEATURES_CSV)
    labels = pd.read_csv(LABELS_CSV)
    data = features.merge(labels, on="trial")

    if len(data) < 8 or data["label"].nunique() < 2:
        print("Not enough labeled trials yet for a held-out evaluation. "
              "Collect more trials across all three classes, then rerun this script.")
        return

    X = data[FEATURE_COLUMNS].to_numpy(dtype=float)
    y = data["label"].to_numpy(dtype=int)

    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.25, random_state=0, stratify=y
    )

    scaler = StandardScaler()
    X_train_scaled = scaler.fit_transform(X_train)
    X_test_scaled = scaler.transform(X_test)

    model = LogisticRegression(max_iter=1000)
    model.fit(X_train_scaled, y_train)
    y_pred = model.predict(X_test_scaled)

    labels_present = sorted(set(y_train) | set(y_test))
    names_present = [CLASS_NAMES[i] for i in labels_present]

    print("Classification report:")
    print(classification_report(y_test, y_pred, labels=labels_present, target_names=names_present))

    print("Confusion matrix (rows=actual, cols=predicted, order=" + str(names_present) + "):")
    print(confusion_matrix(y_test, y_pred, labels=labels_present))


if __name__ == "__main__":
    main()
