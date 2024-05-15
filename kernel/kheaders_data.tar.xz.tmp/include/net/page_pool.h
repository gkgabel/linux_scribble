/* SPDX-License-Identifier: GPL-2.0
 *
 * page_pool.h
 *	Author:	Jesper Dangaard Brouer <netoptimizer@brouer.com>
 *	Copyright (C) 2016 Red Hat, Inc.
 */


#ifndef _NET_PAGE_POOL_H
#define _NET_PAGE_POOL_H

#include <linux/mm.h> 
#include <linux/ptr_ring.h>
#include <linux/dma-direction.h>

#define PP_FLAG_DMA_MAP		BIT(0) 
#define PP_FLAG_DMA_SYNC_DEV	BIT(1) 
#define PP_FLAG_PAGE_FRAG	BIT(2) 
#define PP_FLAG_ALL		(PP_FLAG_DMA_MAP |\
				 PP_FLAG_DMA_SYNC_DEV |\
				 PP_FLAG_PAGE_FRAG)


#define PP_ALLOC_CACHE_SIZE	128
#define PP_ALLOC_CACHE_REFILL	64
struct pp_alloc_cache {
	u32 count;
	struct page *cache[PP_ALLOC_CACHE_SIZE];
};

struct page_pool_params {
	unsigned int	flags;
	unsigned int	order;
	unsigned int	pool_size;
	int		nid;  
	struct device	*dev; 
	enum dma_data_direction dma_dir; 
	unsigned int	max_len; 
	unsigned int	offset;  
	void (*init_callback)(struct page *page, void *arg);
	void *init_arg;
};

#ifdef CONFIG_PAGE_POOL_STATS
struct page_pool_alloc_stats {
	u64 fast; 
	u64 slow; 
	u64 slow_high_order; 
	u64 empty; 
	u64 refill; 
	u64 waive;  
};

struct page_pool_recycle_stats {
	u64 cached;	
	u64 cache_full; 
	u64 ring;	
	u64 ring_full;	
	u64 released_refcnt; 
};


struct page_pool_stats {
	struct page_pool_alloc_stats alloc_stats;
	struct page_pool_recycle_stats recycle_stats;
};

int page_pool_ethtool_stats_get_count(void);
u8 *page_pool_ethtool_stats_get_strings(u8 *data);
u64 *page_pool_ethtool_stats_get(u64 *data, void *stats);


bool page_pool_get_stats(struct page_pool *pool,
			 struct page_pool_stats *stats);
#else

static inline int page_pool_ethtool_stats_get_count(void)
{
	return 0;
}

static inline u8 *page_pool_ethtool_stats_get_strings(u8 *data)
{
	return data;
}

static inline u64 *page_pool_ethtool_stats_get(u64 *data, void *stats)
{
	return data;
}

#endif

struct page_pool {
	struct page_pool_params p;

	struct delayed_work release_dw;
	void (*disconnect)(void *);
	unsigned long defer_start;
	unsigned long defer_warn;

	u32 pages_state_hold_cnt;
	unsigned int frag_offset;
	struct page *frag_page;
	long frag_users;

#ifdef CONFIG_PAGE_POOL_STATS
	
	struct page_pool_alloc_stats alloc_stats;
#endif
	u32 xdp_mem_id;

	
	struct pp_alloc_cache alloc ____cacheline_aligned_in_smp;

	
	struct ptr_ring ring;

#ifdef CONFIG_PAGE_POOL_STATS
	
	struct page_pool_recycle_stats __percpu *recycle_stats;
#endif
	atomic_t pages_state_release_cnt;

	
	refcount_t user_cnt;

	u64 destroy_cnt;
};

struct page *page_pool_alloc_pages(struct page_pool *pool, gfp_t gfp);

static inline struct page *page_pool_dev_alloc_pages(struct page_pool *pool)
{
	gfp_t gfp = (GFP_ATOMIC | __GFP_NOWARN);

	return page_pool_alloc_pages(pool, gfp);
}

struct page *page_pool_alloc_frag(struct page_pool *pool, unsigned int *offset,
				  unsigned int size, gfp_t gfp);

static inline struct page *page_pool_dev_alloc_frag(struct page_pool *pool,
						    unsigned int *offset,
						    unsigned int size)
{
	gfp_t gfp = (GFP_ATOMIC | __GFP_NOWARN);

	return page_pool_alloc_frag(pool, offset, size, gfp);
}


static
inline enum dma_data_direction page_pool_get_dma_dir(struct page_pool *pool)
{
	return pool->p.dma_dir;
}

bool page_pool_return_skb_page(struct page *page);

struct page_pool *page_pool_create(const struct page_pool_params *params);

struct xdp_mem_info;

#ifdef CONFIG_PAGE_POOL
void page_pool_destroy(struct page_pool *pool);
void page_pool_use_xdp_mem(struct page_pool *pool, void (*disconnect)(void *),
			   struct xdp_mem_info *mem);
void page_pool_release_page(struct page_pool *pool, struct page *page);
void page_pool_put_page_bulk(struct page_pool *pool, void **data,
			     int count);
#else
static inline void page_pool_destroy(struct page_pool *pool)
{
}

static inline void page_pool_use_xdp_mem(struct page_pool *pool,
					 void (*disconnect)(void *),
					 struct xdp_mem_info *mem)
{
}
static inline void page_pool_release_page(struct page_pool *pool,
					  struct page *page)
{
}

static inline void page_pool_put_page_bulk(struct page_pool *pool, void **data,
					   int count)
{
}
#endif

void page_pool_put_defragged_page(struct page_pool *pool, struct page *page,
				  unsigned int dma_sync_size,
				  bool allow_direct);

static inline void page_pool_fragment_page(struct page *page, long nr)
{
	atomic_long_set(&page->pp_frag_count, nr);
}

static inline long page_pool_defrag_page(struct page *page, long nr)
{
	long ret;

	
	if (atomic_long_read(&page->pp_frag_count) == nr)
		return 0;

	ret = atomic_long_sub_return(nr, &page->pp_frag_count);
	WARN_ON(ret < 0);
	return ret;
}

static inline bool page_pool_is_last_frag(struct page_pool *pool,
					  struct page *page)
{
	
	return !(pool->p.flags & PP_FLAG_PAGE_FRAG) ||
	       (page_pool_defrag_page(page, 1) == 0);
}

static inline void page_pool_put_page(struct page_pool *pool,
				      struct page *page,
				      unsigned int dma_sync_size,
				      bool allow_direct)
{
	
#ifdef CONFIG_PAGE_POOL
	if (!page_pool_is_last_frag(pool, page))
		return;

	page_pool_put_defragged_page(pool, page, dma_sync_size, allow_direct);
#endif
}


static inline void page_pool_put_full_page(struct page_pool *pool,
					   struct page *page, bool allow_direct)
{
	page_pool_put_page(pool, page, -1, allow_direct);
}


static inline void page_pool_recycle_direct(struct page_pool *pool,
					    struct page *page)
{
	page_pool_put_full_page(pool, page, true);
}

#define PAGE_POOL_DMA_USE_PP_FRAG_COUNT	\
		(sizeof(dma_addr_t) > sizeof(unsigned long))

static inline dma_addr_t page_pool_get_dma_addr(struct page *page)
{
	dma_addr_t ret = page->dma_addr;

	if (PAGE_POOL_DMA_USE_PP_FRAG_COUNT)
		ret |= (dma_addr_t)page->dma_addr_upper << 16 << 16;

	return ret;
}

static inline void page_pool_set_dma_addr(struct page *page, dma_addr_t addr)
{
	page->dma_addr = addr;
	if (PAGE_POOL_DMA_USE_PP_FRAG_COUNT)
		page->dma_addr_upper = upper_32_bits(addr);
}

static inline bool is_page_pool_compiled_in(void)
{
#ifdef CONFIG_PAGE_POOL
	return true;
#else
	return false;
#endif
}

static inline bool page_pool_put(struct page_pool *pool)
{
	return refcount_dec_and_test(&pool->user_cnt);
}


void page_pool_update_nid(struct page_pool *pool, int new_nid);
static inline void page_pool_nid_changed(struct page_pool *pool, int new_nid)
{
	if (unlikely(pool->p.nid != new_nid))
		page_pool_update_nid(pool, new_nid);
}

static inline void page_pool_ring_lock(struct page_pool *pool)
	__acquires(&pool->ring.producer_lock)
{
	if (in_serving_softirq())
		spin_lock(&pool->ring.producer_lock);
	else
		spin_lock_bh(&pool->ring.producer_lock);
}

static inline void page_pool_ring_unlock(struct page_pool *pool)
	__releases(&pool->ring.producer_lock)
{
	if (in_serving_softirq())
		spin_unlock(&pool->ring.producer_lock);
	else
		spin_unlock_bh(&pool->ring.producer_lock);
}

#endif 
