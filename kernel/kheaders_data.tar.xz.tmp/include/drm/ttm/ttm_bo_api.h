


#ifndef _TTM_BO_API_H_
#define _TTM_BO_API_H_

#include <drm/drm_gem.h>
#include <drm/drm_vma_manager.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/bitmap.h>
#include <linux/dma-resv.h>

#include "ttm_resource.h"

struct ttm_global;

struct ttm_device;

struct iosys_map;

struct drm_mm_node;

struct ttm_placement;

struct ttm_place;



enum ttm_bo_type {
	ttm_bo_type_device,
	ttm_bo_type_kernel,
	ttm_bo_type_sg
};

struct ttm_tt;



struct ttm_buffer_object {
	struct drm_gem_object base;

	

	struct ttm_device *bdev;
	enum ttm_bo_type type;
	uint32_t page_alignment;
	void (*destroy) (struct ttm_buffer_object *);

	
	struct kref kref;

	

	struct ttm_resource *resource;
	struct ttm_tt *ttm;
	bool deleted;
	struct ttm_lru_bulk_move *bulk_move;

	

	struct list_head ddestroy;

	

	unsigned priority;
	unsigned pin_count;

	

	struct sg_table *sg;
};



#define TTM_BO_MAP_IOMEM_MASK 0x80
struct ttm_bo_kmap_obj {
	void *virtual;
	struct page *page;
	enum {
		ttm_bo_map_iomap        = 1 | TTM_BO_MAP_IOMEM_MASK,
		ttm_bo_map_vmap         = 2,
		ttm_bo_map_kmap         = 3,
		ttm_bo_map_premapped    = 4 | TTM_BO_MAP_IOMEM_MASK,
	} bo_kmap_type;
	struct ttm_buffer_object *bo;
};


struct ttm_operation_ctx {
	bool interruptible;
	bool no_wait_gpu;
	bool gfp_retry_mayfail;
	bool allow_res_evict;
	bool force_alloc;
	struct dma_resv *resv;
	uint64_t bytes_moved;
};


static inline void ttm_bo_get(struct ttm_buffer_object *bo)
{
	kref_get(&bo->kref);
}


static inline __must_check struct ttm_buffer_object *
ttm_bo_get_unless_zero(struct ttm_buffer_object *bo)
{
	if (!kref_get_unless_zero(&bo->kref))
		return NULL;
	return bo;
}


int ttm_bo_wait(struct ttm_buffer_object *bo, bool interruptible, bool no_wait);

static inline int ttm_bo_wait_ctx(struct ttm_buffer_object *bo, struct ttm_operation_ctx *ctx)
{
	return ttm_bo_wait(bo, ctx->interruptible, ctx->no_wait_gpu);
}


int ttm_bo_validate(struct ttm_buffer_object *bo,
		    struct ttm_placement *placement,
		    struct ttm_operation_ctx *ctx);


void ttm_bo_put(struct ttm_buffer_object *bo);

void ttm_bo_move_to_lru_tail(struct ttm_buffer_object *bo);
void ttm_bo_set_bulk_move(struct ttm_buffer_object *bo,
			  struct ttm_lru_bulk_move *bulk);


int ttm_bo_lock_delayed_workqueue(struct ttm_device *bdev);


void ttm_bo_unlock_delayed_workqueue(struct ttm_device *bdev, int resched);


bool ttm_bo_eviction_valuable(struct ttm_buffer_object *bo,
			      const struct ttm_place *place);



int ttm_bo_init_reserved(struct ttm_device *bdev,
			 struct ttm_buffer_object *bo,
			 size_t size, enum ttm_bo_type type,
			 struct ttm_placement *placement,
			 uint32_t page_alignment,
			 struct ttm_operation_ctx *ctx,
			 struct sg_table *sg, struct dma_resv *resv,
			 void (*destroy) (struct ttm_buffer_object *));


int ttm_bo_init(struct ttm_device *bdev, struct ttm_buffer_object *bo,
		size_t size, enum ttm_bo_type type,
		struct ttm_placement *placement,
		uint32_t page_alignment, bool interrubtible,
		struct sg_table *sg, struct dma_resv *resv,
		void (*destroy) (struct ttm_buffer_object *));


static inline void *ttm_kmap_obj_virtual(struct ttm_bo_kmap_obj *map,
					 bool *is_iomem)
{
	*is_iomem = !!(map->bo_kmap_type & TTM_BO_MAP_IOMEM_MASK);
	return map->virtual;
}


int ttm_bo_kmap(struct ttm_buffer_object *bo, unsigned long start_page,
		unsigned long num_pages, struct ttm_bo_kmap_obj *map);


void ttm_bo_kunmap(struct ttm_bo_kmap_obj *map);


int ttm_bo_vmap(struct ttm_buffer_object *bo, struct iosys_map *map);


void ttm_bo_vunmap(struct ttm_buffer_object *bo, struct iosys_map *map);


int ttm_bo_mmap_obj(struct vm_area_struct *vma, struct ttm_buffer_object *bo);


ssize_t ttm_bo_io(struct ttm_device *bdev, struct file *filp,
		  const char __user *wbuf, char __user *rbuf,
		  size_t count, loff_t *f_pos, bool write);

int ttm_bo_swapout(struct ttm_buffer_object *bo, struct ttm_operation_ctx *ctx,
		   gfp_t gfp_flags);

void ttm_bo_pin(struct ttm_buffer_object *bo);
void ttm_bo_unpin(struct ttm_buffer_object *bo);

int ttm_mem_evict_first(struct ttm_device *bdev,
			struct ttm_resource_manager *man,
			const struct ttm_place *place,
			struct ttm_operation_ctx *ctx,
			struct ww_acquire_ctx *ticket);


#define TTM_BO_VM_NUM_PREFAULT 16

vm_fault_t ttm_bo_vm_reserve(struct ttm_buffer_object *bo,
			     struct vm_fault *vmf);

vm_fault_t ttm_bo_vm_fault_reserved(struct vm_fault *vmf,
				    pgprot_t prot,
				    pgoff_t num_prefault);

vm_fault_t ttm_bo_vm_fault(struct vm_fault *vmf);

void ttm_bo_vm_open(struct vm_area_struct *vma);

void ttm_bo_vm_close(struct vm_area_struct *vma);

int ttm_bo_vm_access(struct vm_area_struct *vma, unsigned long addr,
		     void *buf, int len, int write);
bool ttm_bo_delayed_delete(struct ttm_device *bdev, bool remove_all);

vm_fault_t ttm_bo_vm_dummy_page(struct vm_fault *vmf, pgprot_t prot);

#endif
