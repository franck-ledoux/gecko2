#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np
import sys

# Vérifie qu’un argument a été passé
if len(sys.argv) < 2:
    print("Usage: python3 nomscript.py <nomForme>")
    sys.exit(1)

nomForme = sys.argv[1]
data = [
    (0.5, 10, [204.63, 88.61, 115.82, 356.54, 216.19, 165.17, 210.30, 103.79, 121.59, 170.78]),
    (0.5, 100, [112.75, 97.53, 131.45, 123.81, 181.78, 110.94, 178.36, 213.32, 167.31, 204.14]),
    (0.5, 10000, [364.37,148.80,82.33,187.00,156.07,108.23,223.89,270.38,132.14,108.01]),
    (0.5, 243618, [90.53,81.45,111.12,120.54,793.59,309.93,172.24,138.95,263.56,210.38]),
    (1.42, 10, [95.84,116.05,134.14,130.22,163.41,115.68,92.58,176.68,92.76,101.58]),
    (1.42, 100, [99.95,111.96,382.36,99.68,115.68,140.18,80.57,198.18,7483.73,3380.83]),
    (1.42, 10000, [92.47,186.69,132.09,107.12,168.56,109.18,134.13,202.09,112.44,114.15]),
    (1.42, 243618, [90.46,80.04,113.85,105.40,399.39,217.54,186.50,188.97,159.53,73.61]),
    (3.0, 10, [341.55,160.44,219.91,163.17,168.05,93.58,91.49,119.88,95.44,143.41]),
    (3.0, 100, [91.11,120.81,147.05,161.23,108.58,92.21,176.96,146.58,88.53,74.95]),
    (3.0, 10000, [82.16,117.49,146.56,105.13,124.69,326.25,179.11,89.45,85.83,83.75]),
    (3.0, 243618, [245.97,736.26,212.48,202.71,225.75,192.89,104.19,116.32,97.00,122.46]),
    (6.0, 10, [88.32,98.79,127.88,106.48,103.21,106.29,84.19,184.11,175.80,97.85]),
    (6.0, 100, [174.58,102.13,125.86,101.31,189.83,195.16,182.36,120.89,181.04,4787.48]),
    (6.0, 10000, [81.32,131.31,101.63,186.18,96.11,145.63,111.58,642.23,124.95,109.45]),
    (6.0, 243618, [113.51,90.59,91.09,88.02,86.09,83.77,83.78,209.07,276.88,134.18]),
]


labels = [f"C={c}, D={d}" for c, d, _ in data]

# Stats
avg_times = [np.mean(runs) for _, _, runs in data]
medians = [np.median(runs) for _, _, runs in data]
q1 = [np.percentile(runs, 25) for _, _, runs in data]
q3 = [np.percentile(runs, 75) for _, _, runs in data]
x = np.arange(len(data))


# ---------- PLOT 1: "Boxplot" version (Average + Median + Quartiles) ----------
plt.figure(figsize=(12, 6))
# Moyennes en barres
plt.bar(x, avg_times, color="skyblue", label="Average")
# Médianes en points rouges
plt.scatter(x, medians, color="red", marker="o", label="Median", zorder=5)
# Quartiles en barres verticales
for i in range(len(data)):
    plt.vlines(x[i], q1[i], q3[i], color="black", linestyle="-", lw=2, label="Interquartile range" if i == 0 else "")
plt.xticks(x, labels, rotation=45, ha="right")
plt.ylabel("Execution time (s)")
plt.xlabel("Parameters (C, D)")
plt.title("Execution times: Average, Median, and Quartiles")
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
plt.title("Execution times: Average")
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
plt.title("Execution times: Median")
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
plt.title("Execution times: Quartiles (Q1 & Q3)")
plt.legend()
plt.tight_layout()
plt.savefig(f"{nomForme}_var_CandD_quartiles.png")
plt.close()


print("✅ All plots saved successfully!")