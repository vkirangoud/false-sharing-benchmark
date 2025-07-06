import matplotlib.pyplot as plt
import matplotlib.patches as patches

# Constants
CACHE_LINE_SIZE = 64  # bytes
DOUBLE_SIZE = 8       # bytes per double

def plot_false_sharing_doubles():
    fig, ax = plt.subplots(figsize=(12, 2))
    ax.set_xlim(0, 192)
    ax.set_ylim(0, 2)
    ax.set_xticks(range(0, 193, CACHE_LINE_SIZE))
    ax.set_yticks([])
    ax.set_xlabel("Memory Address Space (Bytes)")
    ax.set_title("False Sharing with double (8 bytes) – Cache Line Overlap")

    # Draw cache line boundaries
    for i in range(3):
        start = i * CACHE_LINE_SIZE
        ax.add_patch(patches.Rectangle((start, 0), CACHE_LINE_SIZE, 1,
                                       fill=False, linestyle='--', edgecolor='gray'))
        ax.text(start + CACHE_LINE_SIZE / 2, 1.05, f"Cache Line {i}",
                ha='center', va='bottom', fontsize=9)

    # Thread 0: accessing doubles 0–7 → bytes 0–56
    ax.add_patch(patches.Rectangle((0, 0.1), 64, 0.3, color='green', alpha=0.5,
                                   label="Thread 0: doubles 0–7 (bytes 0–56)"))

    # Thread 1: accessing doubles 7–15 → bytes 56–128
    ax.add_patch(patches.Rectangle((56, 0.6), 72, 0.3, color='red', alpha=0.5,
                                   label="Thread 1: doubles 7–15 (bytes 56–128)"))

    ax.legend(loc='upper right')
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    plot_false_sharing_doubles()
