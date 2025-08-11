import matplotlib.pyplot as plt
import numpy as np

# Data extracted (C, D, AvgTime)
data = [
    (0.5, 100, 549.97),
    (0.5, 1000, 675.74),
    (0.5, 100000, 621.08),
    (0.5, 1000000, 2012.12),
    (1.42, 100, 505.38),
    (1.42, 1000, 363.19),
    (1.42, 100000, 807.80),
    (1.42, 1000000, 486.55),
    (3.0, 100, 579.99),
    (3.0, 1000, 583.07),
    (3.0, 100000, 368.59),
    (3.0, 1000000, 857.02),
    (6.0, 100, 496.81),
    (6.0, 1000, 786.73),
    (6.0, 100000, 530.25),
    (6.0, 1000000, 785.98),
]

# Prepare labels for X-axis: e.g. "C=0.5, D=100"
labels = [f"C={c}, D={int(d)}" for c, d, _ in data]
avg_times = [t for _, _, t in data]

# Figure size
plt.figure(figsize=(15, 7))

# Plot bars
bars = plt.bar(range(len(data)), avg_times, color='skyblue')

# Add value labels above bars
for bar in bars:
    yval = bar.get_height()
    plt.text(bar.get_x() + bar.get_width()/2, yval + 5, f"{yval:.1f}", ha='center', va='bottom', fontsize=8)

# X-axis parameters
plt.xticks(ticks=range(len(data)), labels=labels, rotation=45, ha='right')

# Labels and title
plt.ylabel('Average Time (s)')
plt.xlabel('Parameters (C, D)')
plt.title('Average Time (AvgTime) over 10 runs depending on parameters C and D')

plt.tight_layout()
plt.show()