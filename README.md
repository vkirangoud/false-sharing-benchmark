# False Sharing Benchmark

This project demonstrates and analyzes false sharing effects in multi-threaded vector arithmetic operations. It compares performance between cache-line aligned and misaligned memory access patterns across different thread counts and memory offsets.

## Overview

The benchmark performs vector arithmetic operations (`sqrt(x) + sin(x) * cos(x)`) on large arrays using OpenMP parallelization. It measures:

- **Performance**: Execution time for aligned vs misaligned memory access
- **False Sharing**: Detection of cache line conflicts between threads
- **Scalability**: Performance across different thread counts (2, 4, 8 threads)
- **Memory Layout**: Impact of different offset values (4, 8, 16, 32, 64 bytes)

## Files

- `vector_arithmetic_benchmark.cpp` - Main benchmark program
- `thread_utils.h/cpp` - Thread partitioning utilities
- `plot_vector_op_benchmark.py` - Plotting script for results
- `benchmark_results.csv` - Generated benchmark data
- `vec_benchmark_threads_*_with_false_sharing.png` - Generated plots
- `capture_system_info.sh` - System information capture script
- `system_info.txt` - Captured system details
- `benchmark_vector_arithmetic.sh` - Comprehensive benchmark script
- `benchmark_summary.txt` - Performance analysis summary

## Prerequisites

### System Requirements
- Linux with GCC compiler
- OpenMP support
- Python 3 with matplotlib and pandas

### Install Dependencies

```bash
# Install Python virtual environment support
sudo apt install python3-venv

# Create and activate virtual environment
python3 -m venv .venv
source .venv/bin/activate

# Install Python packages
pip install pandas matplotlib
```

## Building the Benchmark

```bash
# Compile the benchmark
g++ -fopenmp -O3 -march=native -std=c++17 -o vector_arithmetic_benchmark vector_arithmetic_benchmark.cpp thread_utils.cpp
```

## Running the Benchmark

### System Information
The benchmark automatically captures system information including:
- Operating system and kernel version
- Compiler version and OpenMP support
- CPU model, cores, cache sizes
- Memory configuration
- NUMA topology (if available)
- Python environment details

### Single Run
```bash
# Run with default settings (4 threads, 4-byte offset)
./vector_arithmetic_benchmark

# Run with custom thread count and offset
./vector_arithmetic_benchmark <threads> <offset_bytes>
```

### Comprehensive Benchmark
```bash
# Run all combinations of thread counts and offsets
for threads in 2 4 8; do 
    for offset in 4 8 16 32 64; do 
        ./vector_arithmetic_benchmark $threads $offset
    done
done
```

### Automated Benchmark Suite
```bash
# Run the comprehensive benchmark script
./benchmark_vector_arithmetic.sh
```

This script automatically:
- Captures system information
- Runs all thread count and offset combinations
- Generates performance analysis
- Creates plots (if Python environment is available)
- Produces a summary report

### Manual System Information Capture
```bash
# Capture system information manually
./capture_system_info.sh

# View captured information
cat system_info.txt
```

This generates `benchmark_results.csv` with columns:
- `threads`: Number of OpenMP threads
- `offset`: Memory offset in bytes
- `aligned_time`: Execution time for aligned memory
- `misaligned_time`: Execution time for misaligned memory
- `speedup`: Performance ratio (misaligned/aligned)
- `aligned_false_sharing`: False sharing detected in aligned memory (0/1)
- `misaligned_false_sharing`: False sharing detected in misaligned memory (0/1)

### Output Files
- `benchmark_results.csv` - Detailed results for all configurations
- `benchmark_summary.txt` - Performance analysis and statistics
- `system_info.txt` - System configuration details
- `vec_benchmark_threads_*_with_false_sharing.png` - Performance plots

## Generating Plots

### With False Sharing Information
```bash
# Activate virtual environment
source .venv/bin/activate

# Generate plots with false sharing detection
python3 plot_vec_benchmark.py
```

This creates three plot files:
- `vec_benchmark_threads_2_with_false_sharing.png`
- `vec_benchmark_threads_4_with_false_sharing.png`
- `vec_benchmark_threads_8_with_false_sharing.png`

Each plot contains:
- **Top subplot**: Performance comparison with warning symbols (⚠️) for false sharing
- **Bottom subplot**: False sharing detection results (Yes/No)

## Understanding the Results

### False Sharing Detection
- **Aligned Memory**: Typically shows no false sharing (0) due to proper cache line alignment
- **Misaligned Memory**: Often shows false sharing (1) when threads access elements within the same cache line
- **64-byte Offset**: Misaligned memory becomes cache-line aligned again, eliminating false sharing

### Performance Patterns
- **Blocked Partitioning**: Each thread works on contiguous memory blocks
- **Cache Line Size**: 64 bytes (16 float elements)
- **Memory Access**: Sequential access within each thread's assigned range

### Expected Observations
1. **Aligned Memory**: Consistent performance, no false sharing
2. **Misaligned Memory**: Variable performance depending on offset
3. **64-byte Offset**: Similar performance to aligned memory (no false sharing)
4. **Thread Scaling**: Performance varies with thread count due to overhead and cache effects

## Customization

### Modify Thread Counts
Edit the loop in the comprehensive benchmark:
```bash
for threads in 2 4 8 16; do  # Add more thread counts
```

### Modify Offsets
Edit the offset values:
```bash
for offset in 4 8 16 32 64 128; do  # Add more offsets
```

### Change Vector Size
Modify `SIZE` in `vector_arithmetic_benchmark.cpp`:
```cpp
#define SIZE (1024 * 1025)  // Change to desired size
```

### Modify Operations
Change the arithmetic operations in `vector_arithmetic_blocked()`:
```cpp
data[i] = std::sqrt(data[i]) + std::sin(data[i]) * std::cos(data[i]);
```

## Troubleshooting

### Compilation Issues
- Ensure OpenMP is available: `gcc -fopenmp -dM -E - < /dev/null | grep -i open`
- Check C++17 support: `gcc --version`

### Python Plotting Issues
- Ensure virtual environment is activated: `source .venv/bin/activate`
- Check package installation: `pip list | grep -E "(pandas|matplotlib)"`

### Performance Variations
- Results may vary due to system load, CPU frequency scaling, and cache state
- Run multiple times for consistent results
- Consider using `taskset` to pin processes to specific CPU cores

## Analysis

The benchmark helps understand:
- **Memory Layout Impact**: How alignment affects multi-threaded performance
- **False Sharing Effects**: Performance degradation from cache line conflicts
- **Thread Scaling**: How performance scales with thread count
- **Cache Behavior**: Cache line utilization and conflicts

This information is valuable for optimizing multi-threaded applications and understanding memory access patterns in parallel computing.

# 🧪 False Sharing Benchmark: Vector Addition

This project demonstrates the impact of false sharing using OpenMP and perf on vector addition.

## Run the Benchmark

```bash
chmod +x benchmark_vector_add.sh
./benchmark_vector_add.sh
```

## Plot Results

```bash
pip install matplotlib
python plot_vec_benchmark.py
```

## Output

- `vec_timing.csv`: Contains timing and perf data
- `perf_plot.png`: Bar chart comparing aligned and misaligned results