#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include "thread_utils.h"

#define SIZE (1024 * 1024)
#define NUM_RUNS 10
#define OFFSET_BYTES (1 * sizeof(float)) // To force misalignment
#define CACHE_LINE_SIZE 64

// Get number of threads from OMP_NUM_THREADS env variable or default
int get_num_threads() {
    const char* env = std::getenv("OMP_NUM_THREADS");
    return env ? std::atoi(env) : 4; // Default to 4 threads
}

// Check if address is aligned to cache line
bool is_cache_aligned(const void* ptr) {
    return (reinterpret_cast<uintptr_t>(ptr) % CACHE_LINE_SIZE) == 0;
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
    size_t alignment = 64;

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

void vector_add(const float* A, const float* B, float* C, int num_threads) {
    omp_set_num_threads(num_threads);
    int block_size = CACHE_LINE_SIZE/sizeof(float); // Set to 1 to maximize false sharing
    #pragma omp parallel
    {
        dim_t start, end;
        dim_t work_id = omp_get_thread_num();
        thread_block_partition(omp_get_num_threads(), SIZE, block_size, work_id, false, &start, &end);
        for (dim_t i = start; i < end; ++i) {
            C[i] = A[i] + B[i];
        }
    }
}

void vector_add_interleaved(const float* A, const float* B, float* C, int num_threads) {
    omp_set_num_threads(num_threads);
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        // Each thread processes every Nth element (N = number of threads)
        // This causes false sharing as adjacent elements are written by different threads
        for (int i = tid; i < SIZE; i += num_threads) {
            C[i] = A[i] + B[i];
        }
    }
}

double benchmark(const float* A, const float* B, float* C, int num_threads) {
    double total = 0.0;
    for (int run = 0; run < NUM_RUNS; ++run) {
        auto start = std::chrono::high_resolution_clock::now();
        vector_add(A, B, C, num_threads);
        auto end = std::chrono::high_resolution_clock::now();
        total += std::chrono::duration<double>(end - start).count();
    }
    return total / NUM_RUNS;
}

double benchmark_interleaved(const float* A, const float* B, float* C, int num_threads) {
    double total = 0.0;
    for (int run = 0; run < NUM_RUNS; ++run) {
        auto start = std::chrono::high_resolution_clock::now();
        vector_add_interleaved(A, B, C, num_threads);
        auto end = std::chrono::high_resolution_clock::now();
        total += std::chrono::duration<double>(end - start).count();
    }
    return total / NUM_RUNS;
}

int main() {
    int num_threads = get_num_threads();
    std::cout << "🧵 Using " << num_threads << " threads (from OMP_NUM_THREADS)\n";

    std::vector<float> A(SIZE, 1.0f);
    std::vector<float> B(SIZE, 2.0f);

    float* C_aligned = allocate_aligned_buffer(SIZE);
    std::memset(C_aligned, 0, SIZE * sizeof(float));
    std::cout << "Aligned C address: " << C_aligned 
              << " (aligned: " << (is_cache_aligned(C_aligned) ? "YES" : "NO") << ")\n";
    double time_aligned = benchmark(A.data(), B.data(), C_aligned, num_threads);
    std::cout << "✅ Aligned C (blocked):   Avg execution time = " << time_aligned << " sec\n";
#if 0
    float* C_aligned_interleaved = allocate_aligned_buffer(SIZE);
    std::memset(C_aligned_interleaved, 0, SIZE * sizeof(float));
    double time_aligned_interleaved = benchmark_interleaved(A.data(), B.data(), C_aligned_interleaved, num_threads);
    std::cout << "✅ Aligned C (interleaved):   Avg execution time = " << time_aligned_interleaved << " sec\n";
#endif

    float* C_misaligned = allocate_misaligned_buffer(SIZE, OFFSET_BYTES);
    std::memset(C_misaligned, 0, SIZE * sizeof(float));
    std::cout << "Misaligned C address: " << C_misaligned 
              << " (aligned: " << (is_cache_aligned(C_misaligned) ? "YES" : "NO") << ")\n";
    double time_misaligned = benchmark(A.data(), B.data(), C_misaligned, num_threads);
    std::cout << "⚠️  Misaligned C (blocked): Avg execution time = " << time_misaligned << " sec\n";

#if 0
    float* C_misaligned_interleaved = allocate_misaligned_buffer(SIZE, OFFSET_BYTES);
    std::memset(C_misaligned_interleaved, 0, SIZE * sizeof(float));
    double time_misaligned_interleaved = benchmark_interleaved(A.data(), B.data(), C_misaligned_interleaved, num_threads);
    std::cout << "⚠️  Misaligned C (interleaved): Avg execution time = " << time_misaligned_interleaved << " sec\n";
#endif
    free(C_aligned);
   // free(C_aligned_interleaved);
    free_misaligned_buffer(C_misaligned, OFFSET_BYTES);
   // free_misaligned_buffer(C_misaligned_interleaved, OFFSET_BYTES);
    return 0;
}
