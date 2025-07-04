#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <cstdlib>
#include <cstring>

#define SIZE (1024 * 1024)
#define NUM_RUNS 10
#define OFFSET_BYTES 37 // To force misalignment

// Get number of threads from OMP_NUM_THREADS env variable or default
int get_num_threads() {
    const char* env = std::getenv("OMP_NUM_THREADS");
    return env ? std::atoi(env) : 4; // Default to 4 threads
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
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < SIZE; ++i) {
        C[i] = A[i] + B[i];
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

int main() {
    int num_threads = get_num_threads();
    std::cout << "🧵 Using " << num_threads << " threads (from OMP_NUM_THREADS)\n";

    std::vector<float> A(SIZE, 1.0f);
    std::vector<float> B(SIZE, 2.0f);

    std::vector<float> C_aligned(SIZE, 0.0f);
    double time_aligned = benchmark(A.data(), B.data(), C_aligned.data(), num_threads);
    std::cout << "✅ Aligned C:   Avg execution time = " << time_aligned << " sec\n";

    float* C_misaligned = allocate_misaligned_buffer(SIZE, OFFSET_BYTES);
    double time_misaligned = benchmark(A.data(), B.data(), C_misaligned, num_threads);
    std::cout << "⚠️  Misaligned C: Avg execution time = " << time_misaligned << " sec\n";

    free_misaligned_buffer(C_misaligned, OFFSET_BYTES);
    return 0;
}
