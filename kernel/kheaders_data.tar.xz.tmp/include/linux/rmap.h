/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_RMAP_H
#define _LINUX_RMAP_H


#include <linux/list.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/rwsem.h>
#include <linux/memcontrol.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/memremap.h>


struct anon_vma {
	struct anon_vma *root;		
	struct rw_semaphore rwsem;	
	
	atomic_t refcount;

	
	unsigned long num_children;
	
	unsigned long num_active_vmas;

	struct anon_vma *parent;	

	

	
	struct rb_root_cached rb_root;
};


struct anon_vma_chain {
	struct vm_area_struct *vma;
	struct anon_vma *anon_vma;
	struct list_head same_vma;   
	struct rb_node rb;			
	unsigned long rb_subtree_last;
#ifdef CONFIG_DEBUG_VM_RB
	unsigned long cached_vma_start, cached_vma_last;
#endif
};

enum ttu_flags {
	TTU_SPLIT_HUGE_PMD	= 0x4,	
	TTU_IGNORE_MLOCK	= 0x8,	
	TTU_SYNC		= 0x10,	
	TTU_IGNORE_HWPOISON	= 0x20,	
	TTU_BATCH_FLUSH		= 0x40,	
	TTU_RMAP_LOCKED		= 0x80,	
};

#ifdef CONFIG_MMU
static inline void get_anon_vma(struct anon_vma *anon_vma)
{
	atomic_inc(&anon_vma->refcount);
}

void __put_anon_vma(struct anon_vma *anon_vma);

static inline void put_anon_vma(struct anon_vma *anon_vma)
{
	if (atomic_dec_and_test(&anon_vma->refcount))
		__put_anon_vma(anon_vma);
}

static inline void anon_vma_lock_write(struct anon_vma *anon_vma)
{
	down_write(&anon_vma->root->rwsem);
}

static inline void anon_vma_unlock_write(struct anon_vma *anon_vma)
{
	up_write(&anon_vma->root->rwsem);
}

static inline void anon_vma_lock_read(struct anon_vma *anon_vma)
{
	down_read(&anon_vma->root->rwsem);
}

static inline int anon_vma_trylock_read(struct anon_vma *anon_vma)
{
	return down_read_trylock(&anon_vma->root->rwsem);
}

static inline void anon_vma_unlock_read(struct anon_vma *anon_vma)
{
	up_read(&anon_vma->root->rwsem);
}



void anon_vma_init(void);	
int  __anon_vma_prepare(struct vm_area_struct *);
void unlink_anon_vmas(struct vm_area_struct *);
int anon_vma_clone(struct vm_area_struct *, struct vm_area_struct *);
int anon_vma_fork(struct vm_area_struct *, struct vm_area_struct *);

static inline int anon_vma_prepare(struct vm_area_struct *vma)
{
	if (likely(vma->anon_vma))
		return 0;

	return __anon_vma_prepare(vma);
}

static inline void anon_vma_merge(struct vm_area_struct *vma,
				  struct vm_area_struct *next)
{
	VM_BUG_ON_VMA(vma->anon_vma != next->anon_vma, vma);
	unlink_anon_vmas(next);
}

struct anon_vma *page_get_anon_vma(struct page *page);


typedef int __bitwise rmap_t;


#define RMAP_NONE		((__force rmap_t)0)


#define RMAP_EXCLUSIVE		((__force rmap_t)BIT(0))


#define RMAP_COMPOUND		((__force rmap_t)BIT(1))


void page_move_anon_rmap(struct page *, struct vm_area_struct *);
void page_add_anon_rmap(struct page *, struct vm_area_struct *,
		unsigned long address, rmap_t flags);
void page_add_new_anon_rmap(struct page *, struct vm_area_struct *,
		unsigned long address);
void page_add_file_rmap(struct page *, struct vm_area_struct *,
		bool compound);
void page_remove_rmap(struct page *, struct vm_area_struct *,
		bool compound);

void hugepage_add_anon_rmap(struct page *, struct vm_area_struct *,
		unsigned long address, rmap_t flags);
void hugepage_add_new_anon_rmap(struct page *, struct vm_area_struct *,
		unsigned long address);

static inline void __page_dup_rmap(struct page *page, bool compound)
{
	atomic_inc(compound ? compound_mapcount_ptr(page) : &page->_mapcount);
}

static inline void page_dup_file_rmap(struct page *page, bool compound)
{
	__page_dup_rmap(page, compound);
}


static inline int page_try_dup_anon_rmap(struct page *page, bool compound,
					 struct vm_area_struct *vma)
{
	VM_BUG_ON_PAGE(!PageAnon(page), page);

	
	if (!PageAnonExclusive(page))
		goto dup;

	
	if (likely(!is_device_private_page(page) &&
	    unlikely(page_needs_cow_for_dma(vma, page))))
		return -EBUSY;

	ClearPageAnonExclusive(page);
	
dup:
	__page_dup_rmap(page, compound);
	return 0;
}


static inline int page_try_share_anon_rmap(struct page *page)
{
	VM_BUG_ON_PAGE(!PageAnon(page) || !PageAnonExclusive(page), page);

	
	if (likely(!is_device_private_page(page) &&
	    unlikely(page_maybe_dma_pinned(page))))
		return -EBUSY;

	ClearPageAnonExclusive(page);
	return 0;
}


int folio_referenced(struct folio *, int is_locked,
			struct mem_cgroup *memcg, unsigned long *vm_flags);

void try_to_migrate(struct folio *folio, enum ttu_flags flags);
void try_to_unmap(struct folio *, enum ttu_flags flags);

int make_device_exclusive_range(struct mm_struct *mm, unsigned long start,
				unsigned long end, struct page **pages,
				void *arg);


#define PVMW_SYNC		(1 << 0)

#define PVMW_MIGRATION		(1 << 1)

struct page_vma_mapped_walk {
	unsigned long pfn;
	unsigned long nr_pages;
	pgoff_t pgoff;
	struct vm_area_struct *vma;
	unsigned long address;
	pmd_t *pmd;
	pte_t *pte;
	spinlock_t *ptl;
	unsigned int flags;
};

#define DEFINE_PAGE_VMA_WALK(name, _page, _vma, _address, _flags)	\
	struct page_vma_mapped_walk name = {				\
		.pfn = page_to_pfn(_page),				\
		.nr_pages = compound_nr(_page),				\
		.pgoff = page_to_pgoff(_page),				\
		.vma = _vma,						\
		.address = _address,					\
		.flags = _flags,					\
	}

#define DEFINE_FOLIO_VMA_WALK(name, _folio, _vma, _address, _flags)	\
	struct page_vma_mapped_walk name = {				\
		.pfn = folio_pfn(_folio),				\
		.nr_pages = folio_nr_pages(_folio),			\
		.pgoff = folio_pgoff(_folio),				\
		.vma = _vma,						\
		.address = _address,					\
		.flags = _flags,					\
	}

static inline void page_vma_mapped_walk_done(struct page_vma_mapped_walk *pvmw)
{
	
	if (pvmw->pte && !is_vm_hugetlb_page(pvmw->vma))
		pte_unmap(pvmw->pte);
	if (pvmw->ptl)
		spin_unlock(pvmw->ptl);
}

bool page_vma_mapped_walk(struct page_vma_mapped_walk *pvmw);


unsigned long page_address_in_vma(struct page *, struct vm_area_struct *);


int folio_mkclean(struct folio *);

int pfn_mkclean_range(unsigned long pfn, unsigned long nr_pages, pgoff_t pgoff,
		      struct vm_area_struct *vma);

void remove_migration_ptes(struct folio *src, struct folio *dst, bool locked);

int page_mapped_in_vma(struct page *page, struct vm_area_struct *vma);


struct rmap_walk_control {
	void *arg;
	bool try_lock;
	bool contended;
	
	bool (*rmap_one)(struct folio *folio, struct vm_area_struct *vma,
					unsigned long addr, void *arg);
	int (*done)(struct folio *folio);
	struct anon_vma *(*anon_lock)(struct folio *folio,
				      struct rmap_walk_control *rwc);
	bool (*invalid_vma)(struct vm_area_struct *vma, void *arg);
};

void rmap_walk(struct folio *folio, struct rmap_walk_control *rwc);
void rmap_walk_locked(struct folio *folio, struct rmap_walk_control *rwc);


struct anon_vma *folio_lock_anon_vma_read(struct folio *folio,
					  struct rmap_walk_control *rwc);
void page_unlock_anon_vma_read(struct anon_vma *anon_vma);

#else	

#define anon_vma_init()		do {} while (0)
#define anon_vma_prepare(vma)	(0)
#define anon_vma_link(vma)	do {} while (0)

static inline int folio_referenced(struct folio *folio, int is_locked,
				  struct mem_cgroup *memcg,
				  unsigned long *vm_flags)
{
	*vm_flags = 0;
	return 0;
}

static inline void try_to_unmap(struct folio *folio, enum ttu_flags flags)
{
}

static inline int folio_mkclean(struct folio *folio)
{
	return 0;
}
#endif	

static inline int page_mkclean(struct page *page)
{
	return folio_mkclean(page_folio(page));
}
#endif	
