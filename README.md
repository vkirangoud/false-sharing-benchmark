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