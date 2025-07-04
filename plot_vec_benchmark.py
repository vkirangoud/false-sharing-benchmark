import matplotlib.pyplot as plt
import csv
import numpy as np

metrics = []
aligned = []
misaligned = []

with open('vec_timing.csv', 'r') as f:
    reader = csv.DictReader(f)
    rows = list(reader)
    for key in reader.fieldnames[1:]:
        metrics.append(key)
        aligned.append(float(rows[0][key]))
        misaligned.append(float(rows[1][key]))

def scale(values):
    return [v / 1e6 if v > 1e5 else v for v in values]

aligned_scaled = scale(aligned)
misaligned_scaled = scale(misaligned)

x = np.arange(len(metrics))
width = 0.35

plt.figure(figsize=(12, 6))
bars1 = plt.bar(x - width/2, aligned_scaled, width, label='✅ Aligned', color='green')
bars2 = plt.bar(x + width/2, misaligned_scaled, width, label='❌ Misaligned', color='red')

plt.ylabel('Count (Millions) or Seconds')
plt.title('False Sharing: Aligned vs Misaligned Performance')
plt.xticks(x, metrics, rotation=20)
plt.legend()
plt.grid(axis='y', linestyle='--', alpha=0.5)
plt.tight_layout()

for bars, raw_values in zip([bars1, bars2], [aligned, misaligned]):
    for bar, val in zip(bars, raw_values):
        height = bar.get_height()
        label = f'{val/1e6:.2f}M' if val > 1e5 else f'{val:.5f}'
        plt.text(bar.get_x() + bar.get_width()/2, height, label,
                 ha='center', va='bottom', fontsize=8)

plt.savefig("perf_plot.png", dpi=300)
plt.show()