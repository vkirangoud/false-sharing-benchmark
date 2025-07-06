#include "thread_utils.h"


void thread_block_partition
     (
       dim_t      n_way,
       dim_t      n,
       dim_t      bf,
       dim_t      work_id,
       bool       handle_edge_low,
       dim_t*     start,
       dim_t*     end
     )
{

	if ( n_way == 1 ) { *start = 0; *end = n; return; }

	dim_t      all_start  = 0;
	dim_t      all_end    = n;

	dim_t      size       = all_end - all_start;

	dim_t      n_bf_whole = size / bf;
	dim_t      n_bf_left  = size % bf;

	dim_t      n_bf_lo    = n_bf_whole / n_way;
	dim_t      n_bf_hi    = n_bf_whole / n_way;

	// In this function, we partition the space between all_start and
	// all_end into n_way partitions, each a multiple of block_factor
	// with the exception of the one partition that receives the
	// "edge" case (if applicable).
	//
	// Here are examples of various thread partitionings, in units of
	// the block_factor, when n_way = 4. (A '+' indicates the thread
	// that receives the leftover edge case (ie: n_bf_left extra
	// rows/columns in its sub-range).
	//                                        (all_start ... all_end)
	// n_bf_whole  _left  hel  n_th_lo  _hi   thr0  thr1  thr2  thr3
	//         12     =0    f        0    4      3     3     3     3
	//         12     >0    f        0    4      3     3     3     3+
	//         13     >0    f        1    3      4     3     3     3+
	//         14     >0    f        2    2      4     4     3     3+
	//         15     >0    f        3    1      4     4     4     3+
	//         15     =0    f        3    1      4     4     4     3
	//
	//         12     =0    t        4    0      3     3     3     3
	//         12     >0    t        4    0      3+    3     3     3
	//         13     >0    t        3    1      3+    3     3     4
	//         14     >0    t        2    2      3+    3     4     4
	//         15     >0    t        1    3      3+    4     4     4
	//         15     =0    t        1    3      3     4     4     4

	// As indicated by the table above, load is balanced as equally
	// as possible, even in the presence of an edge case.

	// First, we must differentiate between cases where the leftover
	// "edge" case (n_bf_left) should be allocated to a thread partition
	// at the low end of the index range or the high end.

	if ( handle_edge_low == false )
	{
		// Notice that if all threads receive the same number of
		// block_factors, those threads are considered "high" and
		// the "low" thread group is empty.
		dim_t n_th_lo = n_bf_whole % n_way;
		//dim_t n_th_hi = n_way - n_th_lo;

		// If some partitions must have more block_factors than others
		// assign the slightly larger partitions to lower index threads.
		if ( n_th_lo != 0 ) n_bf_lo += 1;

		// Compute the actual widths (in units of rows/columns) of
		// individual threads in the low and high groups.
		dim_t size_lo = n_bf_lo * bf;
		dim_t size_hi = n_bf_hi * bf;

		// Precompute the starting indices of the low and high groups.
		dim_t lo_start = all_start;
		dim_t hi_start = all_start + n_th_lo * size_lo;

		// Compute the start and end of individual threads' ranges
		// as a function of their work_ids and also the group to which
		// they belong (low or high).
		if ( work_id < n_th_lo )
		{
			*start = lo_start + (work_id  ) * size_lo;
			*end   = lo_start + (work_id+1) * size_lo;
		}
		else // if ( n_th_lo <= work_id )
		{
			*start = hi_start + (work_id-n_th_lo  ) * size_hi;
			*end   = hi_start + (work_id-n_th_lo+1) * size_hi;

			// Since the edge case is being allocated to the high
			// end of the index range, we have to advance the last
			// thread's end.
			if ( work_id == n_way - 1 ) *end += n_bf_left;
		}
	}
	else // if ( handle_edge_low == TRUE )
	{
		// Notice that if all threads receive the same number of
		// block_factors, those threads are considered "low" and
		// the "high" thread group is empty.
		dim_t n_th_hi = n_bf_whole % n_way;
		dim_t n_th_lo = n_way - n_th_hi;

		// If some partitions must have more block_factors than others
		// assign the slightly larger partitions to higher index threads.
		if ( n_th_hi != 0 ) n_bf_hi += 1;

		// Compute the actual widths (in units of rows/columns) of
		// individual threads in the low and high groups.
		dim_t size_lo = n_bf_lo * bf;
		dim_t size_hi = n_bf_hi * bf;

		// Precompute the starting indices of the low and high groups.
		dim_t lo_start = all_start;
		dim_t hi_start = all_start + n_th_lo * size_lo
		                           + n_bf_left;

		// Compute the start and end of individual threads' ranges
		// as a function of their work_ids and also the group to which
		// they belong (low or high).
		if ( work_id < n_th_lo )
		{
			*start = lo_start + (work_id  ) * size_lo;
			*end   = lo_start + (work_id+1) * size_lo;

			// Since the edge case is being allocated to the low
			// end of the index range, we have to advance the
			// starts/ends accordingly.
			if ( work_id == 0 )   *end   += n_bf_left;
			else                { *start += n_bf_left;
			                      *end   += n_bf_left; }
		}
		else // if ( n_th_lo <= work_id )
		{
			*start = hi_start + (work_id-n_th_lo  ) * size_hi;
			*end   = hi_start + (work_id-n_th_lo+1) * size_hi;
		}
	}
} 