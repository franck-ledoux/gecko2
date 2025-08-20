import matplotlib.pyplot as plt
import numpy as np

# Data extracted (C, D, AvgTime)
data = [
    (0.5, 10, 58.67),
    (0.5, 100, 85.31),
    (0.5, 10000, 89.46),
    (0.5, 324818, 87.93),
    (1.42, 10, 112.81),
    (1.42, 100, 62.44),
    (1.42, 10000, 66.05),
    (1.42, 324818, 56.98),
    (3.0, 10, 84.54),
    (3.0, 100, 94.24),
    (3.0, 10000, 95.12),
    (3.0, 324818, 63.31),
    (6.0, 10, 82.32),
    (6.0, 100, 63.71),
    (6.0, 10000, 67.35),
    (6.0, 324818, 94.23),
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