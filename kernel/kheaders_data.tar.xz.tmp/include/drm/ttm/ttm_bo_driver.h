

#ifndef _TTM_BO_DRIVER_H_
#define _TTM_BO_DRIVER_H_

#include <drm/drm_mm.h>
#include <drm/drm_vma_manager.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/dma-resv.h>

#include <drm/ttm/ttm_device.h>

#include "ttm_bo_api.h"
#include "ttm_kmap_iter.h"
#include "ttm_placement.h"
#include "ttm_tt.h"
#include "ttm_pool.h"




int ttm_bo_mem_space(struct ttm_buffer_object *bo,
		     struct ttm_placement *placement,
		     struct ttm_resource **mem,
		     struct ttm_operation_ctx *ctx);


void ttm_bo_unmap_virtual(struct ttm_buffer_object *bo);


static inline int ttm_bo_reserve(struct ttm_buffer_object *bo,
				 bool interruptible, bool no_wait,
				 struct ww_acquire_ctx *ticket)
{
	int ret = 0;

	if (no_wait) {
		bool success;
		if (WARN_ON(ticket))
			return -EBUSY;

		success = dma_resv_trylock(bo->base.resv);
		return success ? 0 : -EBUSY;
	}

	if (interruptible)
		ret = dma_resv_lock_interruptible(bo->base.resv, ticket);
	else
		ret = dma_resv_lock(bo->base.resv, ticket);
	if (ret == -EINTR)
		return -ERESTARTSYS;
	return ret;
}


static inline int ttm_bo_reserve_slowpath(struct ttm_buffer_object *bo,
					  bool interruptible,
					  struct ww_acquire_ctx *ticket)
{
	if (interruptible) {
		int ret = dma_resv_lock_slow_interruptible(bo->base.resv,
							   ticket);
		if (ret == -EINTR)
			ret = -ERESTARTSYS;
		return ret;
	}
	dma_resv_lock_slow(bo->base.resv, ticket);
	return 0;
}

static inline void
ttm_bo_move_to_lru_tail_unlocked(struct ttm_buffer_object *bo)
{
	spin_lock(&bo->bdev->lru_lock);
	ttm_bo_move_to_lru_tail(bo);
	spin_unlock(&bo->bdev->lru_lock);
}

static inline void ttm_bo_assign_mem(struct ttm_buffer_object *bo,
				     struct ttm_resource *new_mem)
{
	WARN_ON(bo->resource);
	bo->resource = new_mem;
}


static inline void ttm_bo_move_null(struct ttm_buffer_object *bo,
				    struct ttm_resource *new_mem)
{
	ttm_resource_free(bo, &bo->resource);
	ttm_bo_assign_mem(bo, new_mem);
}


static inline void ttm_bo_unreserve(struct ttm_buffer_object *bo)
{
	ttm_bo_move_to_lru_tail_unlocked(bo);
	dma_resv_unlock(bo->base.resv);
}


int ttm_mem_io_reserve(struct ttm_device *bdev,
		       struct ttm_resource *mem);
void ttm_mem_io_free(struct ttm_device *bdev,
		     struct ttm_resource *mem);



int ttm_bo_move_memcpy(struct ttm_buffer_object *bo,
		       struct ttm_operation_ctx *ctx,
		       struct ttm_resource *new_mem);


int ttm_bo_move_accel_cleanup(struct ttm_buffer_object *bo,
			      struct dma_fence *fence, bool evict,
			      bool pipeline,
			      struct ttm_resource *new_mem);


void ttm_bo_move_sync_cleanup(struct ttm_buffer_object *bo,
			      struct ttm_resource *new_mem);


int ttm_bo_pipeline_gutting(struct ttm_buffer_object *bo);


pgprot_t ttm_io_prot(struct ttm_buffer_object *bo, struct ttm_resource *res,
		     pgprot_t tmp);


int ttm_bo_tt_bind(struct ttm_buffer_object *bo, struct ttm_resource *mem);


void ttm_bo_tt_destroy(struct ttm_buffer_object *bo);

void ttm_move_memcpy(bool clear,
		     u32 num_pages,
		     struct ttm_kmap_iter *dst_iter,
		     struct ttm_kmap_iter *src_iter);

struct ttm_kmap_iter *
ttm_kmap_iter_iomap_init(struct ttm_kmap_iter_iomap *iter_io,
			 struct io_mapping *iomap,
			 struct sg_table *st,
			 resource_size_t start);
#endif
