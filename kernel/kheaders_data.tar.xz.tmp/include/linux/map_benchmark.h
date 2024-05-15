/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef _KERNEL_DMA_BENCHMARK_H
#define _KERNEL_DMA_BENCHMARK_H

#define DMA_MAP_BENCHMARK       _IOWR('d', 1, struct map_benchmark)
#define DMA_MAP_MAX_THREADS     1024
#define DMA_MAP_MAX_SECONDS     300
#define DMA_MAP_MAX_TRANS_DELAY (10 * NSEC_PER_MSEC)

#define DMA_MAP_BIDIRECTIONAL   0
#define DMA_MAP_TO_DEVICE       1
#define DMA_MAP_FROM_DEVICE     2

struct map_benchmark {
	__u64 avg_map_100ns; 
	__u64 map_stddev; 
	__u64 avg_unmap_100ns; 
	__u64 unmap_stddev;
	__u32 threads; 
	__u32 seconds; 
	__s32 node; 
	__u32 dma_bits; 
	__u32 dma_dir; 
	__u32 dma_trans_ns; 
	__u32 granule;  
};
#endif 
