import matplotlib.pyplot as plt
import matplotlib.patches as patches
import argparse

CACHE_LINE_SIZE = 64  # bytes
DOUBLE_SIZE = 8       # bytes per double

def draw_cache_lines(ax, max_bytes):
    for i in range(0, max_bytes + CACHE_LINE_SIZE, CACHE_LINE_SIZE):
        ax.add_patch(patches.Rectangle((i, 0), CACHE_LINE_SIZE, 1,
                                       fill=False, linestyle='--', edgecolor='gray'))
        ax.text(i + CACHE_LINE_SIZE / 2, 1.05, f"CL{i//CACHE_LINE_SIZE}",
                ha='center', va='bottom', fontsize=9)

def plot_thread_ranges_double(ranges, filename="false_sharing_double_demo.png"):
    fig, ax = plt.subplots(figsize=(12, 2))
    
    max_byte = max(end * DOUBLE_SIZE for _, end in ranges)
    ax.set_xlim(0, max_byte + CACHE_LINE_SIZE)
    ax.set_ylim(0, 2)
    ax.set_yticks([])
    ax.set_xticks(range(0, max_byte + CACHE_LINE_SIZE, CACHE_LINE_SIZE))
    ax.set_xlabel("Memory Address Space (Bytes)")
    ax.set_title("Cache Line Overlap with double (8 bytes) – Custom Thread Ranges")

    draw_cache_lines(ax, max_byte)

    colors = ['green', 'red', 'blue', 'orange', 'purple', 'brown', 'cyan']
    for i, (start, end) in enumerate(ranges):
        color = colors[i % len(colors)]
        start_byte = start * DOUBLE_SIZE
        end_byte = end * DOUBLE_SIZE
        ax.add_patch(patches.Rectangle((start_byte, 0.1 + 0.5*i), end_byte - start_byte, 0.3,
                                       color=color, alpha=0.5, label=f"Thread {i}: [{start}, {end - 1}]"))

    ax.legend(loc='upper right')
    plt.tight_layout()
    plt.savefig(filename)
    print(f"✅ Plot saved as '{filename}'")
    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Visualize false sharing with double arrays.")
    parser.add_argument('--range', nargs=2, type=int, action='append',
                        metavar=('START', 'END'), required=True,
                        help='Specify a thread range as START END (in double elements)')
    parser.add_argument('--output', type=str, default='false_sharing_double_demo.png',
                        help='Filename to save the plot as (default: false_sharing_double_demo.png)')
    args = parser.parse_args()

    thread_ranges = [(start, end) for start, end in args.range]
    plot_thread_ranges_double(thread_ranges, filename=args.output)

# python false_sharing_double_demo.py --range 0 8 --range 7 16 --range 15 24 --range 23 32 --range 31 40 --range 39 48 --range 47 56 --range 55 64 --range 63 72 --range 71 80 --range 79 88 --range 87 96 --range 95 104 --range 103 112 --range 111 120 --range 119 128 --range 127 136 --range 135 144 --range 143 152 --range 151 160 --range 159 168 --range 167 176 --range 175 184 --range 183 192
