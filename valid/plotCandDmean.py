import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np

# Données extraites des summaries
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

# Séparer les colonnes
C_vals = np.array([x[0] for x in data])
D_vals = np.array([x[1] for x in data])
AvgTime = np.array([x[2] for x in data])

fig = plt.figure(figsize=(10,7))
ax = fig.add_subplot(111, projection='3d')

# On peut utiliser une échelle logarithmique pour D
ax.scatter(C_vals, D_vals, AvgTime, c=AvgTime, cmap='viridis', s=60)
ax.set_xlabel('Paramètre C')
ax.set_ylabel('Paramètre D (log scale)')
ax.set_zlabel('AvgTime (s)')
ax.set_yscale('log')

plt.title('Moyenne des temps (AvgTime) en fonction des paramètres C et D')
plt.show()