/* SPDX-License-Identifier: GPL-2.0 */
#ifndef LINUX_MM_INLINE_H
#define LINUX_MM_INLINE_H

#include <linux/atomic.h>
#include <linux/huge_mm.h>
#include <linux/swap.h>
#include <linux/string.h>
#include <linux/userfaultfd_k.h>
#include <linux/swapops.h>


static inline int folio_is_file_lru(struct folio *folio)
{
	return !folio_test_swapbacked(folio);
}

static inline int page_is_file_lru(struct page *page)
{
	return folio_is_file_lru(page_folio(page));
}

static __always_inline void update_lru_size(struct lruvec *lruvec,
				enum lru_list lru, enum zone_type zid,
				long nr_pages)
{
	struct pglist_data *pgdat = lruvec_pgdat(lruvec);

	__mod_lruvec_state(lruvec, NR_LRU_BASE + lru, nr_pages);
	__mod_zone_page_state(&pgdat->node_zones[zid],
				NR_ZONE_LRU_BASE + lru, nr_pages);
#ifdef CONFIG_MEMCG
	mem_cgroup_update_lru_size(lruvec, lru, zid, nr_pages);
#endif
}


static __always_inline void __folio_clear_lru_flags(struct folio *folio)
{
	VM_BUG_ON_FOLIO(!folio_test_lru(folio), folio);

	__folio_clear_lru(folio);

	
	if (folio_test_active(folio) && folio_test_unevictable(folio))
		return;

	__folio_clear_active(folio);
	__folio_clear_unevictable(folio);
}

static __always_inline void __clear_page_lru_flags(struct page *page)
{
	__folio_clear_lru_flags(page_folio(page));
}


static __always_inline enum lru_list folio_lru_list(struct folio *folio)
{
	enum lru_list lru;

	VM_BUG_ON_FOLIO(folio_test_active(folio) && folio_test_unevictable(folio), folio);

	if (folio_test_unevictable(folio))
		return LRU_UNEVICTABLE;

	lru = folio_is_file_lru(folio) ? LRU_INACTIVE_FILE : LRU_INACTIVE_ANON;
	if (folio_test_active(folio))
		lru += LRU_ACTIVE;

	return lru;
}

static __always_inline
void lruvec_add_folio(struct lruvec *lruvec, struct folio *folio)
{
	enum lru_list lru = folio_lru_list(folio);

	update_lru_size(lruvec, lru, folio_zonenum(folio),
			folio_nr_pages(folio));
	if (lru != LRU_UNEVICTABLE)
		list_add(&folio->lru, &lruvec->lists[lru]);
}

static __always_inline void add_page_to_lru_list(struct page *page,
				struct lruvec *lruvec)
{
	lruvec_add_folio(lruvec, page_folio(page));
}

static __always_inline
void lruvec_add_folio_tail(struct lruvec *lruvec, struct folio *folio)
{
	enum lru_list lru = folio_lru_list(folio);

	update_lru_size(lruvec, lru, folio_zonenum(folio),
			folio_nr_pages(folio));
	
	list_add_tail(&folio->lru, &lruvec->lists[lru]);
}

static __always_inline void add_page_to_lru_list_tail(struct page *page,
				struct lruvec *lruvec)
{
	lruvec_add_folio_tail(lruvec, page_folio(page));
}

static __always_inline
void lruvec_del_folio(struct lruvec *lruvec, struct folio *folio)
{
	enum lru_list lru = folio_lru_list(folio);

	if (lru != LRU_UNEVICTABLE)
		list_del(&folio->lru);
	update_lru_size(lruvec, lru, folio_zonenum(folio),
			-folio_nr_pages(folio));
}

static __always_inline void del_page_from_lru_list(struct page *page,
				struct lruvec *lruvec)
{
	lruvec_del_folio(lruvec, page_folio(page));
}

#ifdef CONFIG_ANON_VMA_NAME

extern struct anon_vma_name *anon_vma_name(struct vm_area_struct *vma);
extern struct anon_vma_name *anon_vma_name_alloc(const char *name);
extern void anon_vma_name_free(struct kref *kref);


static inline void anon_vma_name_get(struct anon_vma_name *anon_name)
{
	if (anon_name)
		kref_get(&anon_name->kref);
}

static inline void anon_vma_name_put(struct anon_vma_name *anon_name)
{
	if (anon_name)
		kref_put(&anon_name->kref, anon_vma_name_free);
}

static inline
struct anon_vma_name *anon_vma_name_reuse(struct anon_vma_name *anon_name)
{
	
	if (kref_read(&anon_name->kref) < REFCOUNT_MAX) {
		anon_vma_name_get(anon_name);
		return anon_name;

	}
	return anon_vma_name_alloc(anon_name->name);
}

static inline void dup_anon_vma_name(struct vm_area_struct *orig_vma,
				     struct vm_area_struct *new_vma)
{
	struct anon_vma_name *anon_name = anon_vma_name(orig_vma);

	if (anon_name)
		new_vma->anon_name = anon_vma_name_reuse(anon_name);
}

static inline void free_anon_vma_name(struct vm_area_struct *vma)
{
	
	if (!vma->vm_file)
		anon_vma_name_put(vma->anon_name);
}

static inline bool anon_vma_name_eq(struct anon_vma_name *anon_name1,
				    struct anon_vma_name *anon_name2)
{
	if (anon_name1 == anon_name2)
		return true;

	return anon_name1 && anon_name2 &&
		!strcmp(anon_name1->name, anon_name2->name);
}

#else 
static inline struct anon_vma_name *anon_vma_name(struct vm_area_struct *vma)
{
	return NULL;
}

static inline struct anon_vma_name *anon_vma_name_alloc(const char *name)
{
	return NULL;
}

static inline void anon_vma_name_get(struct anon_vma_name *anon_name) {}
static inline void anon_vma_name_put(struct anon_vma_name *anon_name) {}
static inline void dup_anon_vma_name(struct vm_area_struct *orig_vma,
				     struct vm_area_struct *new_vma) {}
static inline void free_anon_vma_name(struct vm_area_struct *vma) {}

static inline bool anon_vma_name_eq(struct anon_vma_name *anon_name1,
				    struct anon_vma_name *anon_name2)
{
	return true;
}

#endif  

static inline void init_tlb_flush_pending(struct mm_struct *mm)
{
	atomic_set(&mm->tlb_flush_pending, 0);
}

static inline void inc_tlb_flush_pending(struct mm_struct *mm)
{
	atomic_inc(&mm->tlb_flush_pending);
	
}

static inline void dec_tlb_flush_pending(struct mm_struct *mm)
{
	
	atomic_dec(&mm->tlb_flush_pending);
}

static inline bool mm_tlb_flush_pending(struct mm_struct *mm)
{
	
	return atomic_read(&mm->tlb_flush_pending);
}

static inline bool mm_tlb_flush_nested(struct mm_struct *mm)
{
	
	return atomic_read(&mm->tlb_flush_pending) > 1;
}


static inline void
pte_install_uffd_wp_if_needed(struct vm_area_struct *vma, unsigned long addr,
			      pte_t *pte, pte_t pteval)
{
#ifdef CONFIG_PTE_MARKER_UFFD_WP
	bool arm_uffd_pte = false;

	
	WARN_ON_ONCE(!pte_none(*pte));

	if (vma_is_anonymous(vma) || !userfaultfd_wp(vma))
		return;

	
	if (unlikely(pte_present(pteval) && pte_uffd_wp(pteval)))
		arm_uffd_pte = true;

	
	if (unlikely(pte_swp_uffd_wp_any(pteval)))
		arm_uffd_pte = true;

	if (unlikely(arm_uffd_pte))
		set_pte_at(vma->vm_mm, addr, pte,
			   make_pte_marker(PTE_MARKER_UFFD_WP));
#endif
}

#endif
