#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import sys
import os

# Vérifie qu’un argument a été passé
if len(sys.argv) < 2:
    print("Usage: python3 nomscript.py <nomForme>")
    sys.exit(1)

nomForme = sys.argv[1]

# Chemin absolu vers le dossier des CSV
base_path = "/Users/paulbourmaud/Projects/gecko/gecko2/OUT_TESTS"
filename = os.path.join(base_path, f"results_summary_CandD_{nomForme}.csv")

if not os.path.exists(filename):
    print(f"❌ File not found: {filename}")
    sys.exit(1)

# Lecture du CSV
df = pd.read_csv(filename)

# On suppose que les colonnes sont :
# C, D, run1, run2, ..., runN
run_cols = [col for col in df.columns if col.lower().startswith("run")]
if not run_cols:
    print("❌ No run columns found (expected run1, run2, ...)")
    sys.exit(1)

data = []
for _, row in df.iterrows():
    c, d = row["C"], row["D"]
    runs = row[run_cols].dropna().values.astype(float)
    data.append((c, d, runs))

labels = [f"C={c}, D={d}" for c, d, _ in data]

# Stats
avg_times = [np.mean(runs) for _, _, runs in data]
medians = [np.median(runs) for _, _, runs in data]
q1 = [np.percentile(runs, 25) for _, _, runs in data]
q3 = [np.percentile(runs, 75) for _, _, runs in data]
x = np.arange(len(data))


# ---------- PLOT 1: "Boxplot" version (Average + Median + Quartiles) ----------
plt.figure(figsize=(12, 6))
plt.bar(x, avg_times, color="skyblue", label="Average")
plt.scatter(x, medians, color="red", marker="o", label="Median", zorder=5)
for i in range(len(data)):
    plt.vlines(x[i], q1[i], q3[i], color="black", linestyle="-", lw=2, label="Interquartile range" if i == 0 else "")
plt.xticks(x, labels, rotation=45, ha="right")
plt.ylabel("Execution time (s)")
plt.xlabel("Parameters (C, D)")
plt.title(f"{nomForme}: Average, Median, and Quartiles")
plt.legend()
plt.tight_layout()
plt.savefig(f"{nomForme}_var_CandD_boxplot.png")
plt.close()


# ---------- PLOT 2: Only Average ----------
plt.figure(figsize=(12, 6))
plt.bar(x, avg_times, color="skyblue", label="Average")
plt.xticks(x, labels, rotation=45, ha="right")
plt.ylabel("Execution time (s)")
plt.xlabel("Parameters (C, D)")
plt.title(f"{nomForme}: Average")
plt.legend()
plt.tight_layout()
plt.savefig(f"{nomForme}_var_CandD_mean.png")
plt.close()


# ---------- PLOT 3: Only Median ----------
plt.figure(figsize=(12, 6))
plt.bar(x, medians, color="lightcoral", label="Median")
plt.xticks(x, labels, rotation=45, ha="right")
plt.ylabel("Execution time (s)")
plt.xlabel("Parameters (C, D)")
plt.title(f"{nomForme}: Median")
plt.legend()
plt.tight_layout()
plt.savefig(f"{nomForme}_var_CandD_median.png")
plt.close()


# ---------- PLOT 4: Only Quartiles ----------
plt.figure(figsize=(12, 6))
for i in range(len(data)):
    plt.vlines(x[i], q1[i], q3[i], color="black", lw=3)
plt.scatter(x, q1, color="blue", label="Q1")
plt.scatter(x, q3, color="green", label="Q3")
plt.xticks(x, labels, rotation=45, ha="right")
plt.ylabel("Execution time (s)")
plt.xlabel("Parameters (C, D)")
plt.title(f"{nomForme}: Quartiles (Q1 & Q3)")
plt.legend()
plt.tight_layout()
plt.savefig(f"{nomForme}_var_CandD_quartiles.png")
plt.close()


print(f"✅ All plots saved successfully for {nomForme}!")