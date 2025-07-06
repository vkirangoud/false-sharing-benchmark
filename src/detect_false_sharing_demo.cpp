#include <iostream>
#include <vector>
#include <utility>
#include <cstdint>
#include <cstdlib>
#include <cstdio>

constexpr int CACHE_LINE_SIZE = 64; // typical x86 cache line size

using dim_t = std::size_t;

// Generate index ranges for each thread
std::vector<std::pair<dim_t, dim_t>> generate_thread_ranges(dim_t total_size, int num_threads) {
    std::vector<std::pair<dim_t, dim_t>> ranges(num_threads);
    dim_t chunk = total_size / num_threads;
    dim_t leftover = total_size % num_threads;

    dim_t start = 0;
    for (int i = 0; i < num_threads; ++i) {
        dim_t end = start + chunk + (i < leftover ? 1 : 0);
        ranges[i] = {start, end};
        start = end;
    }
    return ranges;
}

// Detect false sharing by checking cache line overlap
bool detect_false_sharing(const std::vector<std::pair<dim_t, dim_t>>& thread_ranges,
                          const float* data_address, int num_threads) {
    if (thread_ranges.size() < 2) return false;

    uintptr_t base_addr = reinterpret_cast<uintptr_t>(data_address);

    for (size_t i = 0; i < thread_ranges.size(); ++i) {
        for (size_t j = i + 1; j < thread_ranges.size(); ++j) {
            dim_t start1 = thread_ranges[i].first;
            dim_t end1   = thread_ranges[i].second;
            dim_t start2 = thread_ranges[j].first;
            dim_t end2   = thread_ranges[j].second;

            uintptr_t addr1_start = base_addr + start1 * sizeof(float);
            uintptr_t addr1_end   = base_addr + (end1 - 1) * sizeof(float);
            uintptr_t addr2_start = base_addr + start2 * sizeof(float);
            uintptr_t addr2_end   = base_addr + (end2 - 1) * sizeof(float);

            int cl1_start = addr1_start / CACHE_LINE_SIZE;
            int cl1_end   = addr1_end   / CACHE_LINE_SIZE;
            int cl2_start = addr2_start / CACHE_LINE_SIZE;
            int cl2_end   = addr2_end   / CACHE_LINE_SIZE;

            bool overlap = !(cl1_end < cl2_start || cl2_end < cl1_start);
            if (overlap) {
                printf("⚠️  False sharing detected between threads %zu and %zu!\n", i, j);
                printf("    Thread %zu: cache lines %d-%d\n", i, cl1_start, cl1_end);
                printf("    Thread %zu: cache lines %d-%d\n", j, cl2_start, cl2_end);
                return true;
            }
        }
    }

    printf("✅ No false sharing detected - threads access separate cache lines\n");
    return false;
}

int main() {
    const int num_threads = 4;
    const int total_elements = 256;

    // Allocate buffer
    float* buffer = new float[total_elements];

    // Simulate a misaligned pointer to force false sharing
    float* misaligned = reinterpret_cast<float*>(reinterpret_cast<char*>(buffer) + 32);

    std::cout << "🔍 Checking false sharing with " << num_threads << " threads on "
              << total_elements << " float elements\n";

    // Generate thread index ranges
    auto thread_ranges = generate_thread_ranges(total_elements, num_threads);

    // Run detection
    detect_false_sharing(thread_ranges, misaligned, num_threads);

    delete[] buffer;
    return 0;
}
// g++ -O2 -std=c++17 detect_false_sharing_demo.cpp -o detect_false_sharing_demo
// ./detect_false_sharing_demo
