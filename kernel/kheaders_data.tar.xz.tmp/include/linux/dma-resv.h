
#ifndef _LINUX_RESERVATION_H
#define _LINUX_RESERVATION_H

#include <linux/ww_mutex.h>
#include <linux/dma-fence.h>
#include <linux/slab.h>
#include <linux/seqlock.h>
#include <linux/rcupdate.h>

extern struct ww_class reservation_ww_class;

struct dma_resv_list;


enum dma_resv_usage {
	
	DMA_RESV_USAGE_KERNEL,

	
	DMA_RESV_USAGE_WRITE,

	
	DMA_RESV_USAGE_READ,

	
	DMA_RESV_USAGE_BOOKKEEP
};


static inline enum dma_resv_usage dma_resv_usage_rw(bool write)
{
	
	return write ? DMA_RESV_USAGE_READ : DMA_RESV_USAGE_WRITE;
}


struct dma_resv {
	
	struct ww_mutex lock;

	
	struct dma_resv_list __rcu *fences;
};


struct dma_resv_iter {
	
	struct dma_resv *obj;

	
	enum dma_resv_usage usage;

	
	struct dma_fence *fence;

	
	enum dma_resv_usage fence_usage;

	
	unsigned int index;

	
	struct dma_resv_list *fences;

	
	unsigned int num_fences;

	
	bool is_restarted;
};

struct dma_fence *dma_resv_iter_first_unlocked(struct dma_resv_iter *cursor);
struct dma_fence *dma_resv_iter_next_unlocked(struct dma_resv_iter *cursor);
struct dma_fence *dma_resv_iter_first(struct dma_resv_iter *cursor);
struct dma_fence *dma_resv_iter_next(struct dma_resv_iter *cursor);


static inline void dma_resv_iter_begin(struct dma_resv_iter *cursor,
				       struct dma_resv *obj,
				       enum dma_resv_usage usage)
{
	cursor->obj = obj;
	cursor->usage = usage;
	cursor->fence = NULL;
}


static inline void dma_resv_iter_end(struct dma_resv_iter *cursor)
{
	dma_fence_put(cursor->fence);
}


static inline enum dma_resv_usage
dma_resv_iter_usage(struct dma_resv_iter *cursor)
{
	return cursor->fence_usage;
}


static inline bool dma_resv_iter_is_restarted(struct dma_resv_iter *cursor)
{
	return cursor->is_restarted;
}


#define dma_resv_for_each_fence_unlocked(cursor, fence)			\
	for (fence = dma_resv_iter_first_unlocked(cursor);		\
	     fence; fence = dma_resv_iter_next_unlocked(cursor))


#define dma_resv_for_each_fence(cursor, obj, usage, fence)	\
	for (dma_resv_iter_begin(cursor, obj, usage),	\
	     fence = dma_resv_iter_first(cursor); fence;	\
	     fence = dma_resv_iter_next(cursor))

#define dma_resv_held(obj) lockdep_is_held(&(obj)->lock.base)
#define dma_resv_assert_held(obj) lockdep_assert_held(&(obj)->lock.base)

#ifdef CONFIG_DEBUG_MUTEXES
void dma_resv_reset_max_fences(struct dma_resv *obj);
#else
static inline void dma_resv_reset_max_fences(struct dma_resv *obj) {}
#endif


static inline int dma_resv_lock(struct dma_resv *obj,
				struct ww_acquire_ctx *ctx)
{
	return ww_mutex_lock(&obj->lock, ctx);
}


static inline int dma_resv_lock_interruptible(struct dma_resv *obj,
					      struct ww_acquire_ctx *ctx)
{
	return ww_mutex_lock_interruptible(&obj->lock, ctx);
}


static inline void dma_resv_lock_slow(struct dma_resv *obj,
				      struct ww_acquire_ctx *ctx)
{
	ww_mutex_lock_slow(&obj->lock, ctx);
}


static inline int dma_resv_lock_slow_interruptible(struct dma_resv *obj,
						   struct ww_acquire_ctx *ctx)
{
	return ww_mutex_lock_slow_interruptible(&obj->lock, ctx);
}


static inline bool __must_check dma_resv_trylock(struct dma_resv *obj)
{
	return ww_mutex_trylock(&obj->lock, NULL);
}


static inline bool dma_resv_is_locked(struct dma_resv *obj)
{
	return ww_mutex_is_locked(&obj->lock);
}


static inline struct ww_acquire_ctx *dma_resv_locking_ctx(struct dma_resv *obj)
{
	return READ_ONCE(obj->lock.ctx);
}


static inline void dma_resv_unlock(struct dma_resv *obj)
{
	dma_resv_reset_max_fences(obj);
	ww_mutex_unlock(&obj->lock);
}

void dma_resv_init(struct dma_resv *obj);
void dma_resv_fini(struct dma_resv *obj);
int dma_resv_reserve_fences(struct dma_resv *obj, unsigned int num_fences);
void dma_resv_add_fence(struct dma_resv *obj, struct dma_fence *fence,
			enum dma_resv_usage usage);
void dma_resv_replace_fences(struct dma_resv *obj, uint64_t context,
			     struct dma_fence *fence,
			     enum dma_resv_usage usage);
int dma_resv_get_fences(struct dma_resv *obj, enum dma_resv_usage usage,
			unsigned int *num_fences, struct dma_fence ***fences);
int dma_resv_get_singleton(struct dma_resv *obj, enum dma_resv_usage usage,
			   struct dma_fence **fence);
int dma_resv_copy_fences(struct dma_resv *dst, struct dma_resv *src);
long dma_resv_wait_timeout(struct dma_resv *obj, enum dma_resv_usage usage,
			   bool intr, unsigned long timeout);
bool dma_resv_test_signaled(struct dma_resv *obj, enum dma_resv_usage usage);
void dma_resv_describe(struct dma_resv *obj, struct seq_file *seq);

#endif 
