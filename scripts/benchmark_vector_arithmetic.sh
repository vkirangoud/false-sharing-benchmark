#!/bin/bash

# Vector Arithmetic Benchmark Script
# Runs comprehensive benchmarks with different thread counts and offsets

BIN="../vector_arithmetic_benchmark"
CSV="../data/benchmark_results.csv"
PERF_ALIGNED="../data/perf_arithmetic_aligned.txt"
PERF_MISALIGNED="../data/perf_arithmetic_misaligned.txt"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}📋 $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

# Check if binary exists
if [ ! -f "$BIN" ]; then
    print_error "Binary $BIN not found. Building..."
    g++ -fopenmp -O3 -march=native -std=c++17 -o "$BIN" ../src/vector_arithmetic_benchmark.cpp ../src/thread_utils.cpp
    if [ $? -ne 0 ]; then
        print_error "Build failed!"
        exit 1
    fi
    print_success "Build successful!"
fi

# Capture system information
print_status "Capturing system information..."
./capture_system_info.sh

# Clear previous results
print_status "Clearing previous benchmark results..."
rm -f "$CSV"
rm -f "$PERF_ALIGNED" "$PERF_MISALIGNED"

# Define test configurations
THREAD_COUNTS=(2 4 8)
OFFSETS=(4 8 16 32 64)

print_status "Starting comprehensive vector arithmetic benchmark..."
echo "🧮 Vector Arithmetic Benchmark Suite"
echo "🧵 Thread counts: ${THREAD_COUNTS[*]}"
echo "📏 Offsets: ${OFFSETS[*]} bytes"
echo ""

# Run benchmarks
TOTAL_TESTS=$((${#THREAD_COUNTS[@]} * ${#OFFSETS[@]}))
CURRENT_TEST=0

for threads in "${THREAD_COUNTS[@]}"; do
    for offset in "${OFFSETS[@]}"; do
        CURRENT_TEST=$((CURRENT_TEST + 1))
        print_status "Test $CURRENT_TEST/$TOTAL_TESTS: $threads threads, ${offset}B offset"
        
        # Run the benchmark
        OUTPUT=$(./"$BIN" "$threads" "$offset" 2>&1)
        
        # Check if benchmark ran successfully
        if [ $? -eq 0 ]; then
            print_success "Completed: $threads threads, ${offset}B offset"
        else
            print_error "Failed: $threads threads, ${offset}B offset"
            echo "$OUTPUT"
        fi
    done
done

# Check if results were generated
if [ ! -f "$CSV" ]; then
    print_error "No results file generated!"
    exit 1
fi

print_success "Benchmark completed successfully!"
echo ""

# Display summary
print_status "Benchmark Summary:"
echo "📊 Results file: $CSV"
echo "📋 System info: system_info.txt"
echo ""

# Show sample results
print_status "Sample results (first few lines):"
head -10 "$CSV" | while IFS= read -r line; do
    if [[ $line == threads* ]]; then
        echo "  $line"
    elif [[ $line =~ ^[0-9] ]]; then
        echo "  $line"
    fi
done

# Generate performance analysis
print_status "Generating performance analysis..."

# Create a summary analysis
SUMMARY_FILE="../data/benchmark_summary.txt"
{
    echo "=== Vector Arithmetic Benchmark Summary ==="
    echo "Date: $(date)"
    echo "System: $(grep 'Distribution:' ../data/system_info.txt | cut -d':' -f2 | xargs)"
    echo "CPU: $(grep 'CPU Model:' ../data/system_info.txt | cut -d':' -f2 | xargs)"
    echo "Cache Line: $(grep 'Cache Line Size:' ../data/system_info.txt | cut -d':' -f2 | xargs)"
    echo ""
    echo "=== Performance Summary ==="
    
    # Calculate averages for each thread count
    for threads in "${THREAD_COUNTS[@]}"; do
        echo "Threads: $threads"
        echo "  Average aligned time: $(awk -F',' -v t="$threads" '$1==t {sum+=$3; count++} END {if(count>0) printf "%.6f", sum/count}' "$CSV")s"
        echo "  Average misaligned time: $(awk -F',' -v t="$threads" '$1==t {sum+=$4; count++} END {if(count>0) printf "%.6f", sum/count}' "$CSV")s"
        echo "  Average speedup: $(awk -F',' -v t="$threads" '$1==t {sum+=$5; count++} END {if(count>0) printf "%.3f", sum/count}' "$CSV")x"
        echo ""
    done
    
    echo "=== False Sharing Analysis ==="
    TOTAL_RUNS=$(grep -c '^[0-9]' "$CSV")
    ALIGNED_FS=$(awk -F',' '$6==1 {count++} END {print count}' "$CSV")
    MISALIGNED_FS=$(awk -F',' '$7==1 {count++} END {print count}' "$CSV")
    
    echo "Total benchmark runs: $TOTAL_RUNS"
    echo "Aligned memory with false sharing: $ALIGNED_FS"
    echo "Misaligned memory with false sharing: $MISALIGNED_FS"
    echo "False sharing rate (aligned): $(awk -v a="$ALIGNED_FS" -v t="$TOTAL_RUNS" 'BEGIN {if(t>0) printf "%.1f", a*100/t; else print "0"}')%"
    echo "False sharing rate (misaligned): $(awk -v m="$MISALIGNED_FS" -v t="$TOTAL_RUNS" 'BEGIN {if(t>0) printf "%.1f", m*100/t; else print "0"}')%"
    
} > "$SUMMARY_FILE"

print_success "Summary saved to: $SUMMARY_FILE"

# Display summary
echo ""
print_status "Performance Summary:"
cat "$SUMMARY_FILE"

# Generate plots if Python environment is available
if [ -d "../.venv" ] && [ -f "../plots/plot_vector_op_benchmark.py" ]; then
    print_status "Generating plots..."
    source ../.venv/bin/activate
    cd ../plots && python3 plot_vector_op_benchmark.py
    if [ $? -eq 0 ]; then
        print_success "Plots generated successfully!"
        echo "📈 Plot files:"
        ls -la ../plots/vec_benchmark_threads_*_with_false_sharing.png 2>/dev/null | awk '{print "  " $9}'
    else
        print_warning "Plot generation failed"
    fi
else
    print_warning "Python environment or plotting script not available"
fi

echo ""
print_success "Benchmark suite completed!"
echo "📁 Files generated:"
echo "  - $CSV (detailed results)"
echo "  - $SUMMARY_FILE (performance summary)"
echo "  - ../data/system_info.txt (system details)"
echo "  - ../plots/vec_benchmark_threads_*_with_false_sharing.png (plots)"

# Clean up temporary files
rm -f "$PERF_ALIGNED" "$PERF_MISALIGNED" 2>/dev/null 