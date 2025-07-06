#!/bin/bash

# System Information Capture Script
# Captures details about the system where benchmarks are run

OUTPUT_FILE="../data/system_info.txt"

echo "=== System Information for False Sharing Benchmark ===" > "$OUTPUT_FILE"
echo "Capture Date: $(date)" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Operating System Information
echo "=== Operating System ===" >> "$OUTPUT_FILE"
echo "OS: $(uname -s)" >> "$OUTPUT_FILE"
echo "Kernel: $(uname -r)" >> "$OUTPUT_FILE"
echo "Architecture: $(uname -m)" >> "$OUTPUT_FILE"
echo "Hostname: $(hostname)" >> "$OUTPUT_FILE"
if [ -f /etc/os-release ]; then
    echo "Distribution: $(grep PRETTY_NAME /etc/os-release | cut -d'"' -f2)" >> "$OUTPUT_FILE"
fi
echo "" >> "$OUTPUT_FILE"

# Compiler Information
echo "=== Compiler Information ===" >> "$OUTPUT_FILE"
echo "GCC Version: $(gcc --version | head -n1)" >> "$OUTPUT_FILE"
echo "G++ Version: $(g++ --version | head -n1)" >> "$OUTPUT_FILE"
echo "OpenMP Support: $(gcc -fopenmp -dM -E - < /dev/null | grep -i open | wc -l) flags found" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# CPU Information
echo "=== CPU Information ===" >> "$OUTPUT_FILE"
if [ -f /proc/cpuinfo ]; then
    echo "CPU Model: $(grep 'model name' /proc/cpuinfo | head -n1 | cut -d':' -f2 | sed 's/^[ \t]*//')" >> "$OUTPUT_FILE"
    echo "CPU Cores: $(grep -c 'processor' /proc/cpuinfo)" >> "$OUTPUT_FILE"
    echo "CPU Threads: $(nproc)" >> "$OUTPUT_FILE"
    echo "CPU Frequency: $(grep 'cpu MHz' /proc/cpuinfo | head -n1 | cut -d':' -f2 | sed 's/^[ \t]*//') MHz" >> "$OUTPUT_FILE"
    echo "Cache Line Size: $(getconf LEVEL1_DCACHE_LINESIZE) bytes" >> "$OUTPUT_FILE"
    echo "L1 Cache Size: $(getconf LEVEL1_DCACHE_SIZE) bytes" >> "$OUTPUT_FILE"
    echo "L2 Cache Size: $(getconf LEVEL2_CACHE_SIZE) bytes" >> "$OUTPUT_FILE"
    echo "L3 Cache Size: $(getconf LEVEL3_CACHE_SIZE) bytes" >> "$OUTPUT_FILE"
fi
echo "" >> "$OUTPUT_FILE"

# Memory Information
echo "=== Memory Information ===" >> "$OUTPUT_FILE"
if [ -f /proc/meminfo ]; then
    echo "Total Memory: $(grep MemTotal /proc/meminfo | awk '{print $2 " " $3}')" >> "$OUTPUT_FILE"
    echo "Available Memory: $(grep MemAvailable /proc/meminfo | awk '{print $2 " " $3}')" >> "$OUTPUT_FILE"
fi
echo "" >> "$OUTPUT_FILE"

# NUMA Information
echo "=== NUMA Information ===" >> "$OUTPUT_FILE"
if command -v numactl &> /dev/null; then
    echo "NUMA Nodes: $(numactl --hardware | grep 'available:' | cut -d' ' -f2)" >> "$OUTPUT_FILE"
    echo "NUMA Details:" >> "$OUTPUT_FILE"
    numactl --hardware >> "$OUTPUT_FILE" 2>/dev/null || echo "NUMA details not available" >> "$OUTPUT_FILE"
else
    echo "numactl not available" >> "$OUTPUT_FILE"
fi
echo "" >> "$OUTPUT_FILE"

# Python Environment
echo "=== Python Environment ===" >> "$OUTPUT_FILE"
if command -v python3 &> /dev/null; then
    echo "Python Version: $(python3 --version)" >> "$OUTPUT_FILE"
    echo "Python Location: $(which python3)" >> "$OUTPUT_FILE"
fi
if [ -d ".venv" ]; then
    echo "Virtual Environment: .venv (exists)" >> "$OUTPUT_FILE"
    if [ -f ".venv/bin/activate" ]; then
        echo "Virtual Environment Status: Active" >> "$OUTPUT_FILE"
    fi
fi
echo "" >> "$OUTPUT_FILE"

# Performance Monitoring Tools
echo "=== Performance Tools ===" >> "$OUTPUT_FILE"
if command -v perf &> /dev/null; then
    echo "perf: Available ($(perf --version | head -n1))" >> "$OUTPUT_FILE"
else
    echo "perf: Not available" >> "$OUTPUT_FILE"
fi
if command -v lscpu &> /dev/null; then
    echo "lscpu: Available" >> "$OUTPUT_FILE"
else
    echo "lscpu: Not available" >> "$OUTPUT_FILE"
fi
echo "" >> "$OUTPUT_FILE"

# System Load
echo "=== Current System Load ===" >> "$OUTPUT_FILE"
echo "Load Average: $(uptime | awk -F'load average:' '{print $2}')" >> "$OUTPUT_FILE"
echo "Uptime: $(uptime -p)" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# CPU Governor and Frequency Scaling
echo "=== CPU Power Management ===" >> "$OUTPUT_FILE"
if [ -f /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor ]; then
    echo "CPU Governor: $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor)" >> "$OUTPUT_FILE"
    echo "Current Frequency: $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq) kHz" >> "$OUTPUT_FILE"
    echo "Max Frequency: $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq) kHz" >> "$OUTPUT_FILE"
    echo "Min Frequency: $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq) kHz" >> "$OUTPUT_FILE"
else
    echo "CPU frequency scaling information not available" >> "$OUTPUT_FILE"
fi
echo "" >> "$OUTPUT_FILE"

# Compiler Flags and Optimization
echo "=== Compiler Configuration ===" >> "$OUTPUT_FILE"
echo "Default C++ Standard: $(g++ -dM -E - < /dev/null | grep __cplusplus | cut -d' ' -f3)" >> "$OUTPUT_FILE"
echo "OpenMP Version: $(gcc -fopenmp -dM -E - < /dev/null | grep _OPENMP | cut -d' ' -f3)" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

echo "System information captured in: $OUTPUT_FILE"
echo "=== End of System Information ===" >> "$OUTPUT_FILE" 