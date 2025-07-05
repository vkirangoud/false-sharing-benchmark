#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "thread_utils.h"
#include <fstream>

#define SIZE (1024 * 1025)
#define NUM_RUNS 10
#define OFFSET_BYTES (1 * sizeof(float)) // To force misalignment
#define CACHE_LINE_SIZE 64

// Function declarations
bool detect_false_sharing(const std::vector<std::pair<dim_t, dim_t>>& thread_ranges, 
                         const float* data_address, int num_threads);

// Get number of threads from OMP_NUM_THREADS env variable or default
int get_num_threads() {
    const char* env = std::getenv("OMP_NUM_THREADS");
    return env ? std::atoi(env) : 4; // Default to 4 threads
}

// Check if address is aligned to cache line
bool is_cache_aligned(const void* ptr) {
    return (reinterpret_cast<uintptr_t>(ptr) % CACHE_LINE_SIZE) == 0;
}
// Detect false sharing based on thread ranges and data address
bool find_false_sharing(const float* data_address, int num_threads) {
    if (num_threads < 2) return false;
    dim_t bf = CACHE_LINE_SIZE/sizeof(float);
    dim_t start, end;
    std::vector<std::pair<dim_t, dim_t>> thread_ranges;
    for (int i = 0; i < num_threads; ++i) {
        thread_block_partition(num_threads, SIZE, bf, i, false, &start, &end);
        thread_ranges.push_back({start, end});    
    }

    return detect_false_sharing(thread_ranges, data_address, num_threads);
}

// Detect false sharing based on thread ranges and data address
bool detect_false_sharing(const std::vector<std::pair<dim_t, dim_t>>& thread_ranges, 
                         const float* data_address, int num_threads) {
    if (thread_ranges.size() < 2) return false;
    
    // Calculate cache line boundaries
    uintptr_t base_addr = reinterpret_cast<uintptr_t>(data_address);
    
    // Check if any two threads access elements within the same cache line
    for (size_t i = 0; i < thread_ranges.size(); ++i) {
        for (size_t j = i + 1; j < thread_ranges.size(); ++j) {
            dim_t start1 = thread_ranges[i].first;
            dim_t end1 = thread_ranges[i].second;
            dim_t start2 = thread_ranges[j].first;
            dim_t end2 = thread_ranges[j].second;
            
            // Calculate cache line numbers for the ranges
            uintptr_t addr1_start = base_addr + start1 * sizeof(float);
            uintptr_t addr1_end = base_addr + (end1 - 1) * sizeof(float);
            uintptr_t addr2_start = base_addr + start2 * sizeof(float);
            uintptr_t addr2_end = base_addr + (end2 - 1) * sizeof(float);
            
            int cache_line1_start = addr1_start / CACHE_LINE_SIZE;
            int cache_line1_end = addr1_end / CACHE_LINE_SIZE;
            int cache_line2_start = addr2_start / CACHE_LINE_SIZE;
            int cache_line2_end = addr2_end / CACHE_LINE_SIZE;
            
            // Check for overlap in cache lines
            if (!(cache_line1_end < cache_line2_start || cache_line2_end < cache_line1_start)) {
                printf("⚠️  False sharing detected between threads %zu and %zu!\n", i, j);
                printf("   Thread %zu: cache lines %d-%d, Thread %zu: cache lines %d-%d\n", 
                       i, cache_line1_start, cache_line1_end, j, cache_line2_start, cache_line2_end);
                return true;
            }
        }
    }
    
    printf("✅ No false sharing detected - threads access separate cache lines\n");
    return false;
}

float* allocate_aligned_buffer(size_t count) {
    void* ptr = nullptr;
    if (posix_memalign(&ptr, CACHE_LINE_SIZE, count * sizeof(float))) {
        std::cerr << "Failed to allocate aligned memory\n";
        std::exit(1);
    }
    return reinterpret_cast<float*>(ptr);
}

float* allocate_misaligned_buffer(size_t count, size_t offset_bytes) {
    void* base = nullptr;
    size_t alignment = CACHE_LINE_SIZE;

    if (posix_memalign(&base, alignment, count * sizeof(float) + alignment)) {
        std::cerr << "Failed to allocate aligned memory\n";
        std::exit(1);
    }

    return reinterpret_cast<float*>(reinterpret_cast<char*>(base) + offset_bytes);
}

void free_misaligned_buffer(float* misaligned_ptr, size_t offset_bytes) {
    void* base = reinterpret_cast<void*>(reinterpret_cast<char*>(misaligned_ptr) - offset_bytes);
    free(base);
}

// Blocked partitioning - each thread works on contiguous blocks
void vector_arithmetic_blocked(float* data, int num_threads) {
    omp_set_num_threads(num_threads);
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        dim_t start, end;
        thread_block_partition(num_threads, SIZE, CACHE_LINE_SIZE/sizeof(float), tid, false, &start, &end);
        
        for (dim_t i = start; i < end; ++i) {
            // Perform some arithmetic operations
            data[i] = std::sqrt(data[i]) + std::sin(data[i]) * std::cos(data[i]);
        }
    }
}

double benchmark_blocked(float* data, int num_threads) {
    double total = 0.0;
    for (int run = 0; run < NUM_RUNS; ++run) {
        auto start = std::chrono::high_resolution_clock::now();
        vector_arithmetic_blocked(data, num_threads);
        auto end = std::chrono::high_resolution_clock::now();
        total += std::chrono::duration<double>(end - start).count();
    }
    return total / NUM_RUNS;
}

int main(int argc, char** argv) {
    // Capture system information if not already captured
    if (system("test -f system_info.txt || ./capture_system_info.sh") != 0) {
        std::cerr << "Warning: Could not capture system information\n";
    }
    
    int num_threads = get_num_threads();
    size_t offset_bytes = 4;
    if (argc > 1) num_threads = std::atoi(argv[1]);
    if (argc > 2) offset_bytes = std::atoi(argv[2]);
    std::ofstream csv("benchmark_results.csv", std::ios::app);
    csv << "threads,offset,aligned_time,misaligned_time,speedup,aligned_false_sharing,misaligned_false_sharing\n";
    std::cout << "🧮 Vector Arithmetic Benchmark\n";
    std::cout << "🧵 Using " << num_threads << " threads (from OMP_NUM_THREADS)\n";
    std::cout << "📏 Vector size: " << SIZE << " elements\n";
    std::cout << "🔄 Operations: sqrt(x) + sin(x) * cos(x)\n\n";

    // Initialize data with some values
    std::vector<float> input_data(SIZE);
    for (int i = 0; i < SIZE; ++i) {
        input_data[i] = 1.0f + (i % 100) * 0.01f; // Values between 1.0 and 2.0
    }

    // Test aligned memory
    float* data_aligned = allocate_aligned_buffer(SIZE);
    std::memcpy(data_aligned, input_data.data(), SIZE * sizeof(float));
    std::cout << "Aligned data address: " << data_aligned 
              << " (aligned: " << (is_cache_aligned(data_aligned) ? "YES" : "NO") << ")\n";
    
    double time_aligned = benchmark_blocked(data_aligned, num_threads);
    std::cout << "✅ Aligned: " << time_aligned << " sec\n";
    bool aligned_false_sharing = find_false_sharing(data_aligned, num_threads);
    std::cout << "\n";

    // Test misaligned memory
    float* data_misaligned = allocate_misaligned_buffer(SIZE, offset_bytes); // variable offset
    std::memcpy(data_misaligned, input_data.data(), SIZE * sizeof(float));
    std::cout << "Misaligned data address: " << data_misaligned 
              << " (aligned: " << (is_cache_aligned(data_misaligned) ? "YES" : "NO") << ")\n";
    
    double time_misaligned = benchmark_blocked(data_misaligned, num_threads);
    std::cout << "⚠️  Misaligned: " << time_misaligned << " sec\n";
    bool misaligned_false_sharing = find_false_sharing(data_misaligned, num_threads);
    std::cout << "\n";

    // Summary
    std::cout << "📊 Performance Summary:\n";
    std::cout << "Aligned: " << time_aligned << "s\n";
    std::cout << "Misaligned: " << time_misaligned << "s\n";
    std::cout << "Speedup: " << time_misaligned / time_aligned << "x\n";

    csv << num_threads << "," << offset_bytes << "," << time_aligned << "," << time_misaligned << "," << (time_misaligned/time_aligned) << "," << aligned_false_sharing << "," << misaligned_false_sharing << "\n";
    csv.close();

    free(data_aligned);
    free_misaligned_buffer(data_misaligned, offset_bytes);
    return 0;
} 