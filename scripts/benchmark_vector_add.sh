#!/bin/bash

SRC="../src/compare_vector_add_alignment.cpp"
BIN="../compare_vec"
CSV="../data/vec_timing.csv"
PERF_ALIGNED="../data/perf_aligned.txt"
PERF_MISALIGNED="../data/perf_misaligned.txt"

# Capture system information
echo "📋 Capturing system information..."
./capture_system_info.sh

echo "🔧 Building $SRC..."
g++ -O3 -fopenmp -std=c++17 "$SRC" ../src/thread_utils.cpp -o "$BIN" || { echo "❌ Build failed"; exit 1; }

echo "🚀 Running $BIN..."
OUTPUT=$(./$BIN)

TIME_ALIGNED=$(echo "$OUTPUT" | grep "Aligned" | awk '{print $NF}')
TIME_MISALIGNED=$(echo "$OUTPUT" | grep "Misaligned" | awk '{print $NF}')

echo "📊 Measuring perf stats (aligned)..."
perf stat -e cache-misses,cache-references,cycles,instructions -o "$PERF_ALIGNED" -- ./$BIN > /dev/null

echo "📊 Measuring perf stats (misaligned)..."
perf stat -e cache-misses,cache-references,cycles,instructions -o "$PERF_MISALIGNED" -- ./$BIN > /dev/null

get_metric() {
    grep "$1" "$2" | awk '{gsub(",", "", $1); print $1}'
}

MISS_ALIGNED=$(get_metric "cache-misses" "$PERF_ALIGNED")
MISS_MISALIGNED=$(get_metric "cache-misses" "$PERF_MISALIGNED")
REFS_ALIGNED=$(get_metric "cache-references" "$PERF_ALIGNED")
REFS_MISALIGNED=$(get_metric "cache-references" "$PERF_MISALIGNED")
CYCLES_ALIGNED=$(get_metric "cycles" "$PERF_ALIGNED")
CYCLES_MISALIGNED=$(get_metric "cycles" "$PERF_MISALIGNED")
INST_ALIGNED=$(get_metric "instructions" "$PERF_ALIGNED")
INST_MISALIGNED=$(get_metric "instructions" "$PERF_MISALIGNED")

echo "version,exec_time_sec,cache_misses,cache_references,cycles,instructions" > "$CSV"
echo "aligned,$TIME_ALIGNED,$MISS_ALIGNED,$REFS_ALIGNED,$CYCLES_ALIGNED,$INST_ALIGNED" >> "$CSV"
echo "misaligned,$TIME_MISALIGNED,$MISS_MISALIGNED,$REFS_MISALIGNED,$CYCLES_MISALIGNED,$INST_MISALIGNED" >> "$CSV"

echo "✅ Benchmark complete. Data written to $CSV"
rm -f "$PERF_ALIGNED" "$PERF_MISALIGNED"