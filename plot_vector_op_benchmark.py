import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# Read the CSV file
csv_file = 'benchmark_results.csv'
df = pd.read_csv(csv_file)

# Remove repeated headers if present
if df.columns[0] == 'threads':
    df = df[df['threads'] != 'threads']
    df['threads'] = df['threads'].astype(int)
    df['offset'] = df['offset'].astype(int)
    df['aligned_time'] = df['aligned_time'].astype(float)
    df['misaligned_time'] = df['misaligned_time'].astype(float)
    df['speedup'] = df['speedup'].astype(float)
    df['aligned_false_sharing'] = df['aligned_false_sharing'].astype(int)
    df['misaligned_false_sharing'] = df['misaligned_false_sharing'].astype(int)

# Get unique thread counts
thread_counts = sorted(df['threads'].unique())

for threads in thread_counts:
    subset = df[df['threads'] == threads]
    offsets = subset['offset'].astype(int)
    aligned = subset['aligned_time'].astype(float)
    misaligned = subset['misaligned_time'].astype(float)
    aligned_fs = subset['aligned_false_sharing'].astype(int)
    misaligned_fs = subset['misaligned_false_sharing'].astype(int)

    x = np.arange(len(offsets))
    width = 0.35

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10))
    
    # Performance plot
    bars1 = ax1.bar(x - width/2, aligned, width, label='Aligned', color='green', alpha=0.7)
    bars2 = ax1.bar(x + width/2, misaligned, width, label='Misaligned', color='red', alpha=0.7)
    
    # Add false sharing indicators
    for i, (a_fs, m_fs) in enumerate(zip(aligned_fs.values, misaligned_fs.values)):
        if a_fs:
            ax1.text(x[i] - width/2, aligned.iloc[i] + 0.0001, '⚠️', ha='center', va='bottom', fontsize=12)
        if m_fs:
            ax1.text(x[i] + width/2, misaligned.iloc[i] + 0.0001, '⚠️', ha='center', va='bottom', fontsize=12)

    ax1.set_ylabel('Time (seconds)')
    ax1.set_title(f'Vector Arithmetic Performance (Threads={threads})')
    ax1.set_xticks(x)
    ax1.set_xticklabels(offsets)
    ax1.legend()
    ax1.grid(axis='y', linestyle='--', alpha=0.5)

    # Add value labels on bars
    for bars, values in zip([bars1, bars2], [aligned, misaligned]):
        for bar, val in zip(bars, values):
            height = bar.get_height()
            ax1.text(bar.get_x() + bar.get_width()/2, height, f'{val:.5f}',
                     ha='center', va='bottom', fontsize=8)

    # False sharing plot
    ax2.bar(x - width/2, aligned_fs, width, label='Aligned', color='green', alpha=0.7)
    ax2.bar(x + width/2, misaligned_fs, width, label='Misaligned', color='red', alpha=0.7)
    ax2.set_ylabel('False Sharing (0=No, 1=Yes)')
    ax2.set_xlabel('Offset (bytes)')
    ax2.set_title(f'False Sharing Detection (Threads={threads})')
    ax2.set_xticks(x)
    ax2.set_xticklabels(offsets)
    ax2.set_ylim(0, 1.2)
    ax2.legend()
    ax2.grid(axis='y', linestyle='--', alpha=0.5)

    # Add labels for false sharing
    for i, (a_fs, m_fs) in enumerate(zip(aligned_fs.values, misaligned_fs.values)):
        if a_fs:
            ax2.text(x[i] - width/2, a_fs + 0.05, 'Yes', ha='center', va='bottom', fontsize=10)
        else:
            ax2.text(x[i] - width/2, a_fs + 0.05, 'No', ha='center', va='bottom', fontsize=10)
        if m_fs:
            ax2.text(x[i] + width/2, m_fs + 0.05, 'Yes', ha='center', va='bottom', fontsize=10)
        else:
            ax2.text(x[i] + width/2, m_fs + 0.05, 'No', ha='center', va='bottom', fontsize=10)

    plt.tight_layout()
    plt.savefig(f'vec_benchmark_threads_{threads}_with_false_sharing.png', dpi=300)
    plt.show()