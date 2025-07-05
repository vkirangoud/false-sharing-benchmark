#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

#include <cstddef>
#include <cstdint>
#include <omp.h>

// Type definitions (adjust as needed for your project)
typedef std::size_t dim_t;

// Partition a range among threads in blocks
void thread_block_partition
     (
       dim_t      n_way,
       dim_t      n,
       dim_t      bf,
       dim_t      work_id,
       bool       handle_edge_low,
       dim_t*     start,
       dim_t*     end
     );

#endif // THREAD_UTILS_H 