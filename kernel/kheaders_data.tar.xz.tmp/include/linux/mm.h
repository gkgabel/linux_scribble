/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MM_H
#define _LINUX_MM_H

#include <linux/errno.h>
#include <linux/mmdebug.h>
#include <linux/gfp.h>
#include <linux/bug.h>
#include <linux/list.h>
#include <linux/mmzone.h>
#include <linux/rbtree.h>
#include <linux/atomic.h>
#include <linux/debug_locks.h>
#include <linux/mm_types.h>
#include <linux/mmap_lock.h>
#include <linux/range.h>
#include <linux/pfn.h>
#include <linux/percpu-refcount.h>
#include <linux/bit_spinlock.h>
#include <linux/shrinker.h>
#include <linux/resource.h>
#include <linux/page_ext.h>
#include <linux/err.h>
#include <linux/page-flags.h>
#include <linux/page_ref.h>
#include <linux/overflow.h>
#include <linux/sizes.h>
#include <linux/sched.h>
#include <linux/pgtable.h>
#include <linux/kasan.h>
#include <linux/memremap.h>

#include <linux/page_owner.h>
#include <linux/page_ext.h>

struct mempolicy;
struct anon_vma;
struct anon_vma_chain;
struct user_struct;
struct pt_regs;

extern int sysctl_page_lock_unfairness;

void init_mm_internals(void);

#ifndef CONFIG_NUMA		
extern unsigned long max_mapnr;

static inline void set_max_mapnr(unsigned long limit)
{
	max_mapnr = limit;
}
#else
static inline void set_max_mapnr(unsigned long limit) { }
#endif

extern atomic_long_t _totalram_pages;
static inline unsigned long totalram_pages(void)
{
	return (unsigned long)atomic_long_read(&_totalram_pages);
}

static inline void totalram_pages_inc(void)
{
	atomic_long_inc(&_totalram_pages);
}

static inline void totalram_pages_dec(void)
{
	atomic_long_dec(&_totalram_pages);
}

static inline void totalram_pages_add(long count)
{
	atomic_long_add(count, &_totalram_pages);
}

extern void * high_memory;
extern int page_cluster;

#ifdef CONFIG_SYSCTL
extern int sysctl_legacy_va_layout;
#else
#define sysctl_legacy_va_layout 0
#endif

#ifdef CONFIG_HAVE_ARCH_MMAP_RND_BITS
extern const int mmap_rnd_bits_min;
extern const int mmap_rnd_bits_max;
extern int mmap_rnd_bits __read_mostly;
#endif
#ifdef CONFIG_HAVE_ARCH_MMAP_RND_COMPAT_BITS
extern const int mmap_rnd_compat_bits_min;
extern const int mmap_rnd_compat_bits_max;
extern int mmap_rnd_compat_bits __read_mostly;
#endif

#include <asm/page.h>
#include <asm/processor.h>


#ifndef untagged_addr
#define untagged_addr(addr) (addr)
#endif

#ifndef __pa_symbol
#define __pa_symbol(x)  __pa(RELOC_HIDE((unsigned long)(x), 0))
#endif

#ifndef page_to_virt
#define page_to_virt(x)	__va(PFN_PHYS(page_to_pfn(x)))
#endif

#ifndef lm_alias
#define lm_alias(x)	__va(__pa_symbol(x))
#endif


#ifndef mm_forbids_zeropage
#define mm_forbids_zeropage(X)	(0)
#endif


#if BITS_PER_LONG == 64

#define	mm_zero_struct_page(pp) __mm_zero_struct_page(pp)
static inline void __mm_zero_struct_page(struct page *page)
{
	unsigned long *_pp = (void *)page;

	 
	BUILD_BUG_ON(sizeof(struct page) & 7);
	BUILD_BUG_ON(sizeof(struct page) < 56);
	BUILD_BUG_ON(sizeof(struct page) > 80);

	switch (sizeof(struct page)) {
	case 80:
		_pp[9] = 0;
		fallthrough;
	case 72:
		_pp[8] = 0;
		fallthrough;
	case 64:
		_pp[7] = 0;
		fallthrough;
	case 56:
		_pp[6] = 0;
		_pp[5] = 0;
		_pp[4] = 0;
		_pp[3] = 0;
		_pp[2] = 0;
		_pp[1] = 0;
		_pp[0] = 0;
	}
}
#else
#define mm_zero_struct_page(pp)  ((void)memset((pp), 0, sizeof(struct page)))
#endif


#define MAPCOUNT_ELF_CORE_MARGIN	(5)
#define DEFAULT_MAX_MAP_COUNT	(USHRT_MAX - MAPCOUNT_ELF_CORE_MARGIN)

extern int sysctl_max_map_count;

extern unsigned long sysctl_user_reserve_kbytes;
extern unsigned long sysctl_admin_reserve_kbytes;

extern int sysctl_overcommit_memory;
extern int sysctl_overcommit_ratio;
extern unsigned long sysctl_overcommit_kbytes;

int overcommit_ratio_handler(struct ctl_table *, int, void *, size_t *,
		loff_t *);
int overcommit_kbytes_handler(struct ctl_table *, int, void *, size_t *,
		loff_t *);
int overcommit_policy_handler(struct ctl_table *, int, void *, size_t *,
		loff_t *);

#if defined(CONFIG_SPARSEMEM) && !defined(CONFIG_SPARSEMEM_VMEMMAP)
#define nth_page(page,n) pfn_to_page(page_to_pfn((page)) + (n))
#define folio_page_idx(folio, p)	(page_to_pfn(p) - folio_pfn(folio))
#else
#define nth_page(page,n) ((page) + (n))
#define folio_page_idx(folio, p)	((p) - &(folio)->page)
#endif


#define PAGE_ALIGN(addr) ALIGN(addr, PAGE_SIZE)


#define PAGE_ALIGN_DOWN(addr) ALIGN_DOWN(addr, PAGE_SIZE)


#define PAGE_ALIGNED(addr)	IS_ALIGNED((unsigned long)(addr), PAGE_SIZE)

#define lru_to_page(head) (list_entry((head)->prev, struct page, lru))
static inline struct folio *lru_to_folio(struct list_head *head)
{
	return list_entry((head)->prev, struct folio, lru);
}

void setup_initial_init_mm(void *start_code, void *end_code,
			   void *end_data, void *brk);



struct vm_area_struct *vm_area_alloc(struct mm_struct *);
struct vm_area_struct *vm_area_dup(struct vm_area_struct *);
void vm_area_free(struct vm_area_struct *);

#ifndef CONFIG_MMU
extern struct rb_root nommu_region_tree;
extern struct rw_semaphore nommu_region_sem;

extern unsigned int kobjsize(const void *objp);
#endif


#define VM_NONE		0x00000000

#define VM_READ		0x00000001	
#define VM_WRITE	0x00000002
#define VM_EXEC		0x00000004
#define VM_SHARED	0x00000008


#define VM_MAYREAD	0x00000010	
#define VM_MAYWRITE	0x00000020
#define VM_MAYEXEC	0x00000040
#define VM_MAYSHARE	0x00000080

#define VM_GROWSDOWN	0x00000100	
#define VM_UFFD_MISSING	0x00000200	
#define VM_PFNMAP	0x00000400	
#define VM_UFFD_WP	0x00001000	

#define VM_LOCKED	0x00002000
#define VM_IO           0x00004000	

					
#define VM_SEQ_READ	0x00008000	
#define VM_RAND_READ	0x00010000	

#define VM_DONTCOPY	0x00020000      
#define VM_DONTEXPAND	0x00040000	
#define VM_LOCKONFAULT	0x00080000	
#define VM_ACCOUNT	0x00100000	
#define VM_NORESERVE	0x00200000	
#define VM_HUGETLB	0x00400000	
#define VM_SYNC		0x00800000	
#define VM_ARCH_1	0x01000000	
#define VM_WIPEONFORK	0x02000000	
#define VM_DONTDUMP	0x04000000	

#ifdef CONFIG_MEM_SOFT_DIRTY
# define VM_SOFTDIRTY	0x08000000	
#else
# define VM_SOFTDIRTY	0
#endif

#define VM_MIXEDMAP	0x10000000	
#define VM_HUGEPAGE	0x20000000	
#define VM_NOHUGEPAGE	0x40000000	
#define VM_MERGEABLE	0x80000000	

#ifdef CONFIG_ARCH_USES_HIGH_VMA_FLAGS
#define VM_HIGH_ARCH_BIT_0	32	
#define VM_HIGH_ARCH_BIT_1	33	
#define VM_HIGH_ARCH_BIT_2	34	
#define VM_HIGH_ARCH_BIT_3	35	
#define VM_HIGH_ARCH_BIT_4	36	
#define VM_HIGH_ARCH_0	BIT(VM_HIGH_ARCH_BIT_0)
#define VM_HIGH_ARCH_1	BIT(VM_HIGH_ARCH_BIT_1)
#define VM_HIGH_ARCH_2	BIT(VM_HIGH_ARCH_BIT_2)
#define VM_HIGH_ARCH_3	BIT(VM_HIGH_ARCH_BIT_3)
#define VM_HIGH_ARCH_4	BIT(VM_HIGH_ARCH_BIT_4)
#endif 

#ifdef CONFIG_ARCH_HAS_PKEYS
# define VM_PKEY_SHIFT	VM_HIGH_ARCH_BIT_0
# define VM_PKEY_BIT0	VM_HIGH_ARCH_0	
# define VM_PKEY_BIT1	VM_HIGH_ARCH_1	
# define VM_PKEY_BIT2	VM_HIGH_ARCH_2
# define VM_PKEY_BIT3	VM_HIGH_ARCH_3
#ifdef CONFIG_PPC
# define VM_PKEY_BIT4  VM_HIGH_ARCH_4
#else
# define VM_PKEY_BIT4  0
#endif
#endif 

#if defined(CONFIG_X86)
# define VM_PAT		VM_ARCH_1	
#elif defined(CONFIG_PPC)
# define VM_SAO		VM_ARCH_1	
#elif defined(CONFIG_PARISC)
# define VM_GROWSUP	VM_ARCH_1
#elif defined(CONFIG_IA64)
# define VM_GROWSUP	VM_ARCH_1
#elif defined(CONFIG_SPARC64)
# define VM_SPARC_ADI	VM_ARCH_1	
# define VM_ARCH_CLEAR	VM_SPARC_ADI
#elif defined(CONFIG_ARM64)
# define VM_ARM64_BTI	VM_ARCH_1	
# define VM_ARCH_CLEAR	VM_ARM64_BTI
#elif !defined(CONFIG_MMU)
# define VM_MAPPED_COPY	VM_ARCH_1	
#endif

#if defined(CONFIG_ARM64_MTE)
# define VM_MTE		VM_HIGH_ARCH_0	
# define VM_MTE_ALLOWED	VM_HIGH_ARCH_1	
#else
# define VM_MTE		VM_NONE
# define VM_MTE_ALLOWED	VM_NONE
#endif

#ifndef VM_GROWSUP
# define VM_GROWSUP	VM_NONE
#endif

#ifdef CONFIG_HAVE_ARCH_USERFAULTFD_MINOR
# define VM_UFFD_MINOR_BIT	37
# define VM_UFFD_MINOR		BIT(VM_UFFD_MINOR_BIT)	
#else 
# define VM_UFFD_MINOR		VM_NONE
#endif 


#define VM_STACK_INCOMPLETE_SETUP	(VM_RAND_READ | VM_SEQ_READ)

#define TASK_EXEC ((current->personality & READ_IMPLIES_EXEC) ? VM_EXEC : 0)


#define VM_DATA_FLAGS_TSK_EXEC	(VM_READ | VM_WRITE | TASK_EXEC | \
				 VM_MAYREAD | VM_MAYWRITE | VM_MAYEXEC)
#define VM_DATA_FLAGS_NON_EXEC	(VM_READ | VM_WRITE | VM_MAYREAD | \
				 VM_MAYWRITE | VM_MAYEXEC)
#define VM_DATA_FLAGS_EXEC	(VM_READ | VM_WRITE | VM_EXEC | \
				 VM_MAYREAD | VM_MAYWRITE | VM_MAYEXEC)

#ifndef VM_DATA_DEFAULT_FLAGS		
#define VM_DATA_DEFAULT_FLAGS  VM_DATA_FLAGS_EXEC
#endif

#ifndef VM_STACK_DEFAULT_FLAGS		
#define VM_STACK_DEFAULT_FLAGS VM_DATA_DEFAULT_FLAGS
#endif

#ifdef CONFIG_STACK_GROWSUP
#define VM_STACK	VM_GROWSUP
#else
#define VM_STACK	VM_GROWSDOWN
#endif

#define VM_STACK_FLAGS	(VM_STACK | VM_STACK_DEFAULT_FLAGS | VM_ACCOUNT)


#define VM_ACCESS_FLAGS (VM_READ | VM_WRITE | VM_EXEC)



#define VM_SPECIAL (VM_IO | VM_DONTEXPAND | VM_PFNMAP | VM_MIXEDMAP)


#define VM_NO_KHUGEPAGED (VM_SPECIAL | VM_HUGETLB)


#define VM_INIT_DEF_MASK	VM_NOHUGEPAGE


#define VM_LOCKED_CLEAR_MASK	(~(VM_LOCKED | VM_LOCKONFAULT))


#ifndef VM_ARCH_CLEAR
# define VM_ARCH_CLEAR	VM_NONE
#endif
#define VM_FLAGS_CLEAR	(ARCH_VM_PKEY_FLAGS | VM_ARCH_CLEAR)




#define FAULT_FLAG_DEFAULT  (FAULT_FLAG_ALLOW_RETRY | \
			     FAULT_FLAG_KILLABLE | \
			     FAULT_FLAG_INTERRUPTIBLE)


static inline bool fault_flag_allow_retry_first(enum fault_flag flags)
{
	return (flags & FAULT_FLAG_ALLOW_RETRY) &&
	    (!(flags & FAULT_FLAG_TRIED));
}

#define FAULT_FLAG_TRACE \
	{ FAULT_FLAG_WRITE,		"WRITE" }, \
	{ FAULT_FLAG_MKWRITE,		"MKWRITE" }, \
	{ FAULT_FLAG_ALLOW_RETRY,	"ALLOW_RETRY" }, \
	{ FAULT_FLAG_RETRY_NOWAIT,	"RETRY_NOWAIT" }, \
	{ FAULT_FLAG_KILLABLE,		"KILLABLE" }, \
	{ FAULT_FLAG_TRIED,		"TRIED" }, \
	{ FAULT_FLAG_USER,		"USER" }, \
	{ FAULT_FLAG_REMOTE,		"REMOTE" }, \
	{ FAULT_FLAG_INSTRUCTION,	"INSTRUCTION" }, \
	{ FAULT_FLAG_INTERRUPTIBLE,	"INTERRUPTIBLE" }


struct vm_fault {
	const struct {
		struct vm_area_struct *vma;	
		gfp_t gfp_mask;			
		pgoff_t pgoff;			
		unsigned long address;		
		unsigned long real_address;	
	};
	enum fault_flag flags;		
	pmd_t *pmd;			
	pud_t *pud;			
	union {
		pte_t orig_pte;		
		pmd_t orig_pmd;		
	};

	struct page *cow_page;		
	struct page *page;		
	
	pte_t *pte;			
	spinlock_t *ptl;		
	pgtable_t prealloc_pte;		
};


enum page_entry_size {
	PE_SIZE_PTE = 0,
	PE_SIZE_PMD,
	PE_SIZE_PUD,
};


struct vm_operations_struct {
	void (*open)(struct vm_area_struct * area);
	
	void (*close)(struct vm_area_struct * area);
	
	int (*may_split)(struct vm_area_struct *area, unsigned long addr);
	int (*mremap)(struct vm_area_struct *area);
	
	int (*mprotect)(struct vm_area_struct *vma, unsigned long start,
			unsigned long end, unsigned long newflags);
	vm_fault_t (*fault)(struct vm_fault *vmf);
	vm_fault_t (*huge_fault)(struct vm_fault *vmf,
			enum page_entry_size pe_size);
	vm_fault_t (*map_pages)(struct vm_fault *vmf,
			pgoff_t start_pgoff, pgoff_t end_pgoff);
	unsigned long (*pagesize)(struct vm_area_struct * area);

	
	vm_fault_t (*page_mkwrite)(struct vm_fault *vmf);

	
	vm_fault_t (*pfn_mkwrite)(struct vm_fault *vmf);

	
	int (*access)(struct vm_area_struct *vma, unsigned long addr,
		      void *buf, int len, int write);

	
	const char *(*name)(struct vm_area_struct *vma);

#ifdef CONFIG_NUMA
	
	int (*set_policy)(struct vm_area_struct *vma, struct mempolicy *new);

	
	struct mempolicy *(*get_policy)(struct vm_area_struct *vma,
					unsigned long addr);
#endif
	
	struct page *(*find_special_page)(struct vm_area_struct *vma,
					  unsigned long addr);
};

static inline void vma_init(struct vm_area_struct *vma, struct mm_struct *mm)
{
	static const struct vm_operations_struct dummy_vm_ops = {};

	memset(vma, 0, sizeof(*vma));
	vma->vm_mm = mm;
	vma->vm_ops = &dummy_vm_ops;
	INIT_LIST_HEAD(&vma->anon_vma_chain);
}

static inline void vma_set_anonymous(struct vm_area_struct *vma)
{
	vma->vm_ops = NULL;
}

static inline bool vma_is_anonymous(struct vm_area_struct *vma)
{
	return !vma->vm_ops;
}

static inline bool vma_is_temporary_stack(struct vm_area_struct *vma)
{
	int maybe_stack = vma->vm_flags & (VM_GROWSDOWN | VM_GROWSUP);

	if (!maybe_stack)
		return false;

	if ((vma->vm_flags & VM_STACK_INCOMPLETE_SETUP) ==
						VM_STACK_INCOMPLETE_SETUP)
		return true;

	return false;
}

static inline bool vma_is_foreign(struct vm_area_struct *vma)
{
	if (!current->mm)
		return true;

	if (current->mm != vma->vm_mm)
		return true;

	return false;
}

static inline bool vma_is_accessible(struct vm_area_struct *vma)
{
	return vma->vm_flags & VM_ACCESS_FLAGS;
}

#ifdef CONFIG_SHMEM

bool vma_is_shmem(struct vm_area_struct *vma);
#else
static inline bool vma_is_shmem(struct vm_area_struct *vma) { return false; }
#endif

int vma_is_stack_for_current(struct vm_area_struct *vma);


#define TLB_FLUSH_VMA(mm,flags) { .vm_mm = (mm), .vm_flags = (flags) }

struct mmu_gather;
struct inode;

static inline unsigned int compound_order(struct page *page)
{
	if (!PageHead(page))
		return 0;
	return page[1].compound_order;
}


static inline unsigned int folio_order(struct folio *folio)
{
	return compound_order(&folio->page);
}

#include <linux/huge_mm.h>




static inline int put_page_testzero(struct page *page)
{
	VM_BUG_ON_PAGE(page_ref_count(page) == 0, page);
	return page_ref_dec_and_test(page);
}

static inline int folio_put_testzero(struct folio *folio)
{
	return put_page_testzero(&folio->page);
}


static inline bool get_page_unless_zero(struct page *page)
{
	return page_ref_add_unless(page, 1, 0);
}

extern int page_is_ram(unsigned long pfn);

enum {
	REGION_INTERSECTS,
	REGION_DISJOINT,
	REGION_MIXED,
};

int region_intersects(resource_size_t offset, size_t size, unsigned long flags,
		      unsigned long desc);


struct page *vmalloc_to_page(const void *addr);
unsigned long vmalloc_to_pfn(const void *addr);



#ifndef is_ioremap_addr
#define is_ioremap_addr(x) is_vmalloc_addr(x)
#endif

#ifdef CONFIG_MMU
extern bool is_vmalloc_addr(const void *x);
extern int is_vmalloc_or_module_addr(const void *x);
#else
static inline bool is_vmalloc_addr(const void *x)
{
	return false;
}
static inline int is_vmalloc_or_module_addr(const void *x)
{
	return 0;
}
#endif


static inline int folio_entire_mapcount(struct folio *folio)
{
	VM_BUG_ON_FOLIO(!folio_test_large(folio), folio);
	return atomic_read(folio_mapcount_ptr(folio)) + 1;
}


static inline int compound_mapcount(struct page *page)
{
	return folio_entire_mapcount(page_folio(page));
}


static inline void page_mapcount_reset(struct page *page)
{
	atomic_set(&(page)->_mapcount, -1);
}

int __page_mapcount(struct page *page);


static inline int page_mapcount(struct page *page)
{
	if (unlikely(PageCompound(page)))
		return __page_mapcount(page);
	return atomic_read(&page->_mapcount) + 1;
}

int folio_mapcount(struct folio *folio);

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
static inline int total_mapcount(struct page *page)
{
	return folio_mapcount(page_folio(page));
}

#else
static inline int total_mapcount(struct page *page)
{
	return page_mapcount(page);
}
#endif

static inline struct page *virt_to_head_page(const void *x)
{
	struct page *page = virt_to_page(x);

	return compound_head(page);
}

static inline struct folio *virt_to_folio(const void *x)
{
	struct page *page = virt_to_page(x);

	return page_folio(page);
}

void __folio_put(struct folio *folio);

void put_pages_list(struct list_head *pages);

void split_page(struct page *page, unsigned int order);
void folio_copy(struct folio *dst, struct folio *src);

unsigned long nr_free_buffer_pages(void);


typedef void compound_page_dtor(struct page *);


enum compound_dtor_id {
	NULL_COMPOUND_DTOR,
	COMPOUND_PAGE_DTOR,
#ifdef CONFIG_HUGETLB_PAGE
	HUGETLB_PAGE_DTOR,
#endif
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	TRANSHUGE_PAGE_DTOR,
#endif
	NR_COMPOUND_DTORS,
};
extern compound_page_dtor * const compound_page_dtors[NR_COMPOUND_DTORS];

static inline void set_compound_page_dtor(struct page *page,
		enum compound_dtor_id compound_dtor)
{
	VM_BUG_ON_PAGE(compound_dtor >= NR_COMPOUND_DTORS, page);
	page[1].compound_dtor = compound_dtor;
}

void destroy_large_folio(struct folio *folio);

static inline int head_compound_pincount(struct page *head)
{
	return atomic_read(compound_pincount_ptr(head));
}

static inline void set_compound_order(struct page *page, unsigned int order)
{
	page[1].compound_order = order;
#ifdef CONFIG_64BIT
	page[1].compound_nr = 1U << order;
#endif
}


static inline unsigned long compound_nr(struct page *page)
{
	if (!PageHead(page))
		return 1;
#ifdef CONFIG_64BIT
	return page[1].compound_nr;
#else
	return 1UL << compound_order(page);
#endif
}


static inline unsigned long page_size(struct page *page)
{
	return PAGE_SIZE << compound_order(page);
}


static inline unsigned int page_shift(struct page *page)
{
	return PAGE_SHIFT + compound_order(page);
}


static inline unsigned int thp_order(struct page *page)
{
	VM_BUG_ON_PGFLAGS(PageTail(page), page);
	return compound_order(page);
}


static inline int thp_nr_pages(struct page *page)
{
	VM_BUG_ON_PGFLAGS(PageTail(page), page);
	return compound_nr(page);
}


static inline unsigned long thp_size(struct page *page)
{
	return PAGE_SIZE << thp_order(page);
}

void free_compound_page(struct page *page);

#ifdef CONFIG_MMU

static inline pte_t maybe_mkwrite(pte_t pte, struct vm_area_struct *vma)
{
	if (likely(vma->vm_flags & VM_WRITE))
		pte = pte_mkwrite(pte);
	return pte;
}

vm_fault_t do_set_pmd(struct vm_fault *vmf, struct page *page);
void do_set_pte(struct vm_fault *vmf, struct page *page, unsigned long addr);

vm_fault_t finish_fault(struct vm_fault *vmf);
vm_fault_t finish_mkwrite_fault(struct vm_fault *vmf);
#endif



#if defined(CONFIG_ZONE_DEVICE) && defined(CONFIG_FS_DAX)
DECLARE_STATIC_KEY_FALSE(devmap_managed_key);

bool __put_devmap_managed_page_refs(struct page *page, int refs);
static inline bool put_devmap_managed_page_refs(struct page *page, int refs)
{
	if (!static_branch_unlikely(&devmap_managed_key))
		return false;
	if (!is_zone_device_page(page))
		return false;
	return __put_devmap_managed_page_refs(page, refs);
}
#else 
static inline bool put_devmap_managed_page_refs(struct page *page, int refs)
{
	return false;
}
#endif 

static inline bool put_devmap_managed_page(struct page *page)
{
	return put_devmap_managed_page_refs(page, 1);
}


#define folio_ref_zero_or_close_to_overflow(folio) \
	((unsigned int) folio_ref_count(folio) + 127u <= 127u)


static inline void folio_get(struct folio *folio)
{
	VM_BUG_ON_FOLIO(folio_ref_zero_or_close_to_overflow(folio), folio);
	folio_ref_inc(folio);
}

static inline void get_page(struct page *page)
{
	folio_get(page_folio(page));
}

bool __must_check try_grab_page(struct page *page, unsigned int flags);

static inline __must_check bool try_get_page(struct page *page)
{
	page = compound_head(page);
	if (WARN_ON_ONCE(page_ref_count(page) <= 0))
		return false;
	page_ref_inc(page);
	return true;
}


static inline void folio_put(struct folio *folio)
{
	if (folio_put_testzero(folio))
		__folio_put(folio);
}


static inline void folio_put_refs(struct folio *folio, int refs)
{
	if (folio_ref_sub_and_test(folio, refs))
		__folio_put(folio);
}

void release_pages(struct page **pages, int nr);


static inline void folios_put(struct folio **folios, unsigned int nr)
{
	release_pages((struct page **)folios, nr);
}

static inline void put_page(struct page *page)
{
	struct folio *folio = page_folio(page);
	
	struct page_ext *page_ext;
    struct page_owner *pg_owner;
	page_ext = lookup_page_ext(page);
    if(page_ext != NULL)
	{	pg_owner = (void *)page_ext + page_owner_ops.offset;
        if(pg_owner->flag_gup>0)
			pg_owner->flag_gup--;
	}
	
	
	if (put_devmap_managed_page(&folio->page))
		return;
	folio_put(folio);
}


#define GUP_PIN_COUNTING_BIAS (1U << 10)

void unpin_user_page(struct page *page);
void unpin_user_pages_dirty_lock(struct page **pages, unsigned long npages,
				 bool make_dirty);
void unpin_user_page_range_dirty_lock(struct page *page, unsigned long npages,
				      bool make_dirty);
void unpin_user_pages(struct page **pages, unsigned long npages);

static inline bool is_cow_mapping(vm_flags_t flags)
{
	return (flags & (VM_SHARED | VM_MAYWRITE)) == VM_MAYWRITE;
}

#if defined(CONFIG_SPARSEMEM) && !defined(CONFIG_SPARSEMEM_VMEMMAP)
#define SECTION_IN_PAGE_FLAGS
#endif


static inline int page_zone_id(struct page *page)
{
	return (page->flags >> ZONEID_PGSHIFT) & ZONEID_MASK;
}

#ifdef NODE_NOT_IN_PAGE_FLAGS
extern int page_to_nid(const struct page *page);
#else
static inline int page_to_nid(const struct page *page)
{
	struct page *p = (struct page *)page;

	return (PF_POISONED_CHECK(p)->flags >> NODES_PGSHIFT) & NODES_MASK;
}
#endif

static inline int folio_nid(const struct folio *folio)
{
	return page_to_nid(&folio->page);
}

#ifdef CONFIG_NUMA_BALANCING
static inline int cpu_pid_to_cpupid(int cpu, int pid)
{
	return ((cpu & LAST__CPU_MASK) << LAST__PID_SHIFT) | (pid & LAST__PID_MASK);
}

static inline int cpupid_to_pid(int cpupid)
{
	return cpupid & LAST__PID_MASK;
}

static inline int cpupid_to_cpu(int cpupid)
{
	return (cpupid >> LAST__PID_SHIFT) & LAST__CPU_MASK;
}

static inline int cpupid_to_nid(int cpupid)
{
	return cpu_to_node(cpupid_to_cpu(cpupid));
}

static inline bool cpupid_pid_unset(int cpupid)
{
	return cpupid_to_pid(cpupid) == (-1 & LAST__PID_MASK);
}

static inline bool cpupid_cpu_unset(int cpupid)
{
	return cpupid_to_cpu(cpupid) == (-1 & LAST__CPU_MASK);
}

static inline bool __cpupid_match_pid(pid_t task_pid, int cpupid)
{
	return (task_pid & LAST__PID_MASK) == cpupid_to_pid(cpupid);
}

#define cpupid_match_pid(task, cpupid) __cpupid_match_pid(task->pid, cpupid)
#ifdef LAST_CPUPID_NOT_IN_PAGE_FLAGS
static inline int page_cpupid_xchg_last(struct page *page, int cpupid)
{
	return xchg(&page->_last_cpupid, cpupid & LAST_CPUPID_MASK);
}

static inline int page_cpupid_last(struct page *page)
{
	return page->_last_cpupid;
}
static inline void page_cpupid_reset_last(struct page *page)
{
	page->_last_cpupid = -1 & LAST_CPUPID_MASK;
}
#else
static inline int page_cpupid_last(struct page *page)
{
	return (page->flags >> LAST_CPUPID_PGSHIFT) & LAST_CPUPID_MASK;
}

extern int page_cpupid_xchg_last(struct page *page, int cpupid);

static inline void page_cpupid_reset_last(struct page *page)
{
	page->flags |= LAST_CPUPID_MASK << LAST_CPUPID_PGSHIFT;
}
#endif 
#else 
static inline int page_cpupid_xchg_last(struct page *page, int cpupid)
{
	return page_to_nid(page); 
}

static inline int page_cpupid_last(struct page *page)
{
	return page_to_nid(page); 
}

static inline int cpupid_to_nid(int cpupid)
{
	return -1;
}

static inline int cpupid_to_pid(int cpupid)
{
	return -1;
}

static inline int cpupid_to_cpu(int cpupid)
{
	return -1;
}

static inline int cpu_pid_to_cpupid(int nid, int pid)
{
	return -1;
}

static inline bool cpupid_pid_unset(int cpupid)
{
	return true;
}

static inline void page_cpupid_reset_last(struct page *page)
{
}

static inline bool cpupid_match_pid(struct task_struct *task, int cpupid)
{
	return false;
}
#endif 

#if defined(CONFIG_KASAN_SW_TAGS) || defined(CONFIG_KASAN_HW_TAGS)



static inline u8 page_kasan_tag(const struct page *page)
{
	u8 tag = 0xff;

	if (kasan_enabled()) {
		tag = (page->flags >> KASAN_TAG_PGSHIFT) & KASAN_TAG_MASK;
		tag ^= 0xff;
	}

	return tag;
}

static inline void page_kasan_tag_set(struct page *page, u8 tag)
{
	unsigned long old_flags, flags;

	if (!kasan_enabled())
		return;

	tag ^= 0xff;
	old_flags = READ_ONCE(page->flags);
	do {
		flags = old_flags;
		flags &= ~(KASAN_TAG_MASK << KASAN_TAG_PGSHIFT);
		flags |= (tag & KASAN_TAG_MASK) << KASAN_TAG_PGSHIFT;
	} while (unlikely(!try_cmpxchg(&page->flags, &old_flags, flags)));
}

static inline void page_kasan_tag_reset(struct page *page)
{
	if (kasan_enabled())
		page_kasan_tag_set(page, 0xff);
}

#else 

static inline u8 page_kasan_tag(const struct page *page)
{
	return 0xff;
}

static inline void page_kasan_tag_set(struct page *page, u8 tag) { }
static inline void page_kasan_tag_reset(struct page *page) { }

#endif 

static inline struct zone *page_zone(const struct page *page)
{
	return &NODE_DATA(page_to_nid(page))->node_zones[page_zonenum(page)];
}

static inline pg_data_t *page_pgdat(const struct page *page)
{
	return NODE_DATA(page_to_nid(page));
}

static inline struct zone *folio_zone(const struct folio *folio)
{
	return page_zone(&folio->page);
}

static inline pg_data_t *folio_pgdat(const struct folio *folio)
{
	return page_pgdat(&folio->page);
}

#ifdef SECTION_IN_PAGE_FLAGS
static inline void set_page_section(struct page *page, unsigned long section)
{
	page->flags &= ~(SECTIONS_MASK << SECTIONS_PGSHIFT);
	page->flags |= (section & SECTIONS_MASK) << SECTIONS_PGSHIFT;
}

static inline unsigned long page_to_section(const struct page *page)
{
	return (page->flags >> SECTIONS_PGSHIFT) & SECTIONS_MASK;
}
#endif


static inline unsigned long folio_pfn(struct folio *folio)
{
	return page_to_pfn(&folio->page);
}

static inline atomic_t *folio_pincount_ptr(struct folio *folio)
{
	return &folio_page(folio, 1)->compound_pincount;
}


static inline bool folio_maybe_dma_pinned(struct folio *folio)
{
	if (folio_test_large(folio))
		return atomic_read(folio_pincount_ptr(folio)) > 0;

	
	return ((unsigned int)folio_ref_count(folio)) >=
		GUP_PIN_COUNTING_BIAS;
}

static inline bool page_maybe_dma_pinned(struct page *page)
{
	return folio_maybe_dma_pinned(page_folio(page));
}


static inline bool page_needs_cow_for_dma(struct vm_area_struct *vma,
					  struct page *page)
{
	VM_BUG_ON(!(raw_read_seqcount(&vma->vm_mm->write_protect_seq) & 1));

	if (!test_bit(MMF_HAS_PINNED, &vma->vm_mm->flags))
		return false;

	return page_maybe_dma_pinned(page);
}


#ifdef CONFIG_MIGRATION
static inline bool is_longterm_pinnable_page(struct page *page)
{
#ifdef CONFIG_CMA
	int mt = get_pageblock_migratetype(page);

	if (mt == MIGRATE_CMA || mt == MIGRATE_ISOLATE)
		return false;
#endif
	
	if (is_zero_pfn(page_to_pfn(page)))
		return true;

	
	if (is_device_coherent_page(page))
		return false;

	
	return !is_zone_movable_page(page);
}
#else
static inline bool is_longterm_pinnable_page(struct page *page)
{
	return true;
}
#endif

static inline bool folio_is_longterm_pinnable(struct folio *folio)
{
	return is_longterm_pinnable_page(&folio->page);
}

static inline void set_page_zone(struct page *page, enum zone_type zone)
{
	page->flags &= ~(ZONES_MASK << ZONES_PGSHIFT);
	page->flags |= (zone & ZONES_MASK) << ZONES_PGSHIFT;
}

static inline void set_page_node(struct page *page, unsigned long node)
{
	page->flags &= ~(NODES_MASK << NODES_PGSHIFT);
	page->flags |= (node & NODES_MASK) << NODES_PGSHIFT;
}

static inline void set_page_links(struct page *page, enum zone_type zone,
	unsigned long node, unsigned long pfn)
{
	set_page_zone(page, zone);
	set_page_node(page, node);
#ifdef SECTION_IN_PAGE_FLAGS
	set_page_section(page, pfn_to_section_nr(pfn));
#endif
}


static inline long folio_nr_pages(struct folio *folio)
{
	return compound_nr(&folio->page);
}


static inline struct folio *folio_next(struct folio *folio)
{
	return (struct folio *)folio_page(folio, folio_nr_pages(folio));
}


static inline unsigned int folio_shift(struct folio *folio)
{
	return PAGE_SHIFT + folio_order(folio);
}


static inline size_t folio_size(struct folio *folio)
{
	return PAGE_SIZE << folio_order(folio);
}

#ifndef HAVE_ARCH_MAKE_PAGE_ACCESSIBLE
static inline int arch_make_page_accessible(struct page *page)
{
	return 0;
}
#endif

#ifndef HAVE_ARCH_MAKE_FOLIO_ACCESSIBLE
static inline int arch_make_folio_accessible(struct folio *folio)
{
	int ret;
	long i, nr = folio_nr_pages(folio);

	for (i = 0; i < nr; i++) {
		ret = arch_make_page_accessible(folio_page(folio, i));
		if (ret)
			break;
	}

	return ret;
}
#endif


#include <linux/vmstat.h>

static __always_inline void *lowmem_page_address(const struct page *page)
{
	return page_to_virt(page);
}

#if defined(CONFIG_HIGHMEM) && !defined(WANT_PAGE_VIRTUAL)
#define HASHED_PAGE_VIRTUAL
#endif

#if defined(WANT_PAGE_VIRTUAL)
static inline void *page_address(const struct page *page)
{
	return page->virtual;
}
static inline void set_page_address(struct page *page, void *address)
{
	page->virtual = address;
}
#define page_address_init()  do { } while(0)
#endif

#if defined(HASHED_PAGE_VIRTUAL)
void *page_address(const struct page *page);
void set_page_address(struct page *page, void *virtual);
void page_address_init(void);
#endif

#if !defined(HASHED_PAGE_VIRTUAL) && !defined(WANT_PAGE_VIRTUAL)
#define page_address(page) lowmem_page_address(page)
#define set_page_address(page, address)  do { } while(0)
#define page_address_init()  do { } while(0)
#endif

static inline void *folio_address(const struct folio *folio)
{
	return page_address(&folio->page);
}

extern void *page_rmapping(struct page *page);
extern pgoff_t __page_file_index(struct page *page);


static inline pgoff_t page_index(struct page *page)
{
	if (unlikely(PageSwapCache(page)))
		return __page_file_index(page);
	return page->index;
}

bool page_mapped(struct page *page);
bool folio_mapped(struct folio *folio);


static inline bool page_is_pfmemalloc(const struct page *page)
{
	
	return (uintptr_t)page->lru.next & BIT(1);
}


static inline void set_page_pfmemalloc(struct page *page)
{
	page->lru.next = (void *)BIT(1);
}

static inline void clear_page_pfmemalloc(struct page *page)
{
	page->lru.next = NULL;
}


extern void pagefault_out_of_memory(void);

#define offset_in_page(p)	((unsigned long)(p) & ~PAGE_MASK)
#define offset_in_thp(page, p)	((unsigned long)(p) & (thp_size(page) - 1))
#define offset_in_folio(folio, p) ((unsigned long)(p) & (folio_size(folio) - 1))


#define SHOW_MEM_FILTER_NODES		(0x0001u)	

extern void show_free_areas(unsigned int flags, nodemask_t *nodemask);


struct zap_details {
	struct folio *single_folio;	
	bool even_cows;			
	zap_flags_t zap_flags;		
};


#define  ZAP_FLAG_DROP_MARKER        ((__force zap_flags_t) BIT(0))

#define  ZAP_FLAG_UNMAP              ((__force zap_flags_t) BIT(1))

#ifdef CONFIG_MMU
extern bool can_do_mlock(void);
#else
static inline bool can_do_mlock(void) { return false; }
#endif
extern int user_shm_lock(size_t, struct ucounts *);
extern void user_shm_unlock(size_t, struct ucounts *);

struct page *vm_normal_page(struct vm_area_struct *vma, unsigned long addr,
			     pte_t pte);
struct page *vm_normal_page_pmd(struct vm_area_struct *vma, unsigned long addr,
				pmd_t pmd);

void zap_vma_ptes(struct vm_area_struct *vma, unsigned long address,
		  unsigned long size);
void zap_page_range(struct vm_area_struct *vma, unsigned long address,
		    unsigned long size);
void unmap_vmas(struct mmu_gather *tlb, struct vm_area_struct *start_vma,
		unsigned long start, unsigned long end);
void zap_page_range_single(struct vm_area_struct *vma, unsigned long address,
			   unsigned long size, struct zap_details *details);

struct mmu_notifier_range;

void free_pgd_range(struct mmu_gather *tlb, unsigned long addr,
		unsigned long end, unsigned long floor, unsigned long ceiling);
int
copy_page_range(struct vm_area_struct *dst_vma, struct vm_area_struct *src_vma);
int follow_pte(struct mm_struct *mm, unsigned long address,
	       pte_t **ptepp, spinlock_t **ptlp);
int follow_pfn(struct vm_area_struct *vma, unsigned long address,
	unsigned long *pfn);
int follow_phys(struct vm_area_struct *vma, unsigned long address,
		unsigned int flags, unsigned long *prot, resource_size_t *phys);
int generic_access_phys(struct vm_area_struct *vma, unsigned long addr,
			void *buf, int len, int write);

extern void truncate_pagecache(struct inode *inode, loff_t new);
extern void truncate_setsize(struct inode *inode, loff_t newsize);
void pagecache_isize_extended(struct inode *inode, loff_t from, loff_t to);
void truncate_pagecache_range(struct inode *inode, loff_t offset, loff_t end);
int generic_error_remove_page(struct address_space *mapping, struct page *page);

#ifdef CONFIG_MMU
extern vm_fault_t handle_mm_fault(struct vm_area_struct *vma,
				  unsigned long address, unsigned int flags,
				  struct pt_regs *regs);
extern int fixup_user_fault(struct mm_struct *mm,
			    unsigned long address, unsigned int fault_flags,
			    bool *unlocked);
void unmap_mapping_pages(struct address_space *mapping,
		pgoff_t start, pgoff_t nr, bool even_cows);
void unmap_mapping_range(struct address_space *mapping,
		loff_t const holebegin, loff_t const holelen, int even_cows);
#else
static inline vm_fault_t handle_mm_fault(struct vm_area_struct *vma,
					 unsigned long address, unsigned int flags,
					 struct pt_regs *regs)
{
	
	BUG();
	return VM_FAULT_SIGBUS;
}
static inline int fixup_user_fault(struct mm_struct *mm, unsigned long address,
		unsigned int fault_flags, bool *unlocked)
{
	
	BUG();
	return -EFAULT;
}
static inline void unmap_mapping_pages(struct address_space *mapping,
		pgoff_t start, pgoff_t nr, bool even_cows) { }
static inline void unmap_mapping_range(struct address_space *mapping,
		loff_t const holebegin, loff_t const holelen, int even_cows) { }
#endif

static inline void unmap_shared_mapping_range(struct address_space *mapping,
		loff_t const holebegin, loff_t const holelen)
{
	unmap_mapping_range(mapping, holebegin, holelen, 0);
}

extern int access_process_vm(struct task_struct *tsk, unsigned long addr,
		void *buf, int len, unsigned int gup_flags);
extern int access_remote_vm(struct mm_struct *mm, unsigned long addr,
		void *buf, int len, unsigned int gup_flags);
extern int __access_remote_vm(struct mm_struct *mm, unsigned long addr,
			      void *buf, int len, unsigned int gup_flags);

long get_user_pages_remote(struct mm_struct *mm,
			    unsigned long start, unsigned long nr_pages,
			    unsigned int gup_flags, struct page **pages,
			    struct vm_area_struct **vmas, int *locked);
long pin_user_pages_remote(struct mm_struct *mm,
			   unsigned long start, unsigned long nr_pages,
			   unsigned int gup_flags, struct page **pages,
			   struct vm_area_struct **vmas, int *locked);
long get_user_pages(unsigned long start, unsigned long nr_pages,
			    unsigned int gup_flags, struct page **pages,
			    struct vm_area_struct **vmas);
long pin_user_pages(unsigned long start, unsigned long nr_pages,
		    unsigned int gup_flags, struct page **pages,
		    struct vm_area_struct **vmas);
long get_user_pages_unlocked(unsigned long start, unsigned long nr_pages,
		    struct page **pages, unsigned int gup_flags);
long pin_user_pages_unlocked(unsigned long start, unsigned long nr_pages,
		    struct page **pages, unsigned int gup_flags);

int get_user_pages_fast(unsigned long start, int nr_pages,
			unsigned int gup_flags, struct page **pages);
int pin_user_pages_fast(unsigned long start, int nr_pages,
			unsigned int gup_flags, struct page **pages);

int account_locked_vm(struct mm_struct *mm, unsigned long pages, bool inc);
int __account_locked_vm(struct mm_struct *mm, unsigned long pages, bool inc,
			struct task_struct *task, bool bypass_rlim);

struct kvec;
int get_kernel_pages(const struct kvec *iov, int nr_pages, int write,
			struct page **pages);
struct page *get_dump_page(unsigned long addr);

bool folio_mark_dirty(struct folio *folio);
bool set_page_dirty(struct page *page);
int set_page_dirty_lock(struct page *page);

int get_cmdline(struct task_struct *task, char *buffer, int buflen);

extern unsigned long move_page_tables(struct vm_area_struct *vma,
		unsigned long old_addr, struct vm_area_struct *new_vma,
		unsigned long new_addr, unsigned long len,
		bool need_rmap_locks);



#define  MM_CP_TRY_CHANGE_WRITABLE	   (1UL << 0)

#define  MM_CP_PROT_NUMA                   (1UL << 1)

#define  MM_CP_UFFD_WP                     (1UL << 2) 
#define  MM_CP_UFFD_WP_RESOLVE             (1UL << 3) 
#define  MM_CP_UFFD_WP_ALL                 (MM_CP_UFFD_WP | \
					    MM_CP_UFFD_WP_RESOLVE)

extern unsigned long change_protection(struct mmu_gather *tlb,
			      struct vm_area_struct *vma, unsigned long start,
			      unsigned long end, pgprot_t newprot,
			      unsigned long cp_flags);
extern int mprotect_fixup(struct mmu_gather *tlb, struct vm_area_struct *vma,
			  struct vm_area_struct **pprev, unsigned long start,
			  unsigned long end, unsigned long newflags);


int get_user_pages_fast_only(unsigned long start, int nr_pages,
			     unsigned int gup_flags, struct page **pages);
int pin_user_pages_fast_only(unsigned long start, int nr_pages,
			     unsigned int gup_flags, struct page **pages);

static inline bool get_user_page_fast_only(unsigned long addr,
			unsigned int gup_flags, struct page **pagep)
{
	return get_user_pages_fast_only(addr, 1, gup_flags, pagep) == 1;
}

static inline unsigned long get_mm_counter(struct mm_struct *mm, int member)
{
	long val = atomic_long_read(&mm->rss_stat.count[member]);

#ifdef SPLIT_RSS_COUNTING
	
	if (val < 0)
		val = 0;
#endif
	return (unsigned long)val;
}

void mm_trace_rss_stat(struct mm_struct *mm, int member, long count);

static inline void add_mm_counter(struct mm_struct *mm, int member, long value)
{
	long count = atomic_long_add_return(value, &mm->rss_stat.count[member]);

	mm_trace_rss_stat(mm, member, count);
}

static inline void inc_mm_counter(struct mm_struct *mm, int member)
{
	long count = atomic_long_inc_return(&mm->rss_stat.count[member]);

	mm_trace_rss_stat(mm, member, count);
}

static inline void dec_mm_counter(struct mm_struct *mm, int member)
{
	long count = atomic_long_dec_return(&mm->rss_stat.count[member]);

	mm_trace_rss_stat(mm, member, count);
}


static inline int mm_counter_file(struct page *page)
{
	if (PageSwapBacked(page))
		return MM_SHMEMPAGES;
	return MM_FILEPAGES;
}

static inline int mm_counter(struct page *page)
{
	if (PageAnon(page))
		return MM_ANONPAGES;
	return mm_counter_file(page);
}

static inline unsigned long get_mm_rss(struct mm_struct *mm)
{
	return get_mm_counter(mm, MM_FILEPAGES) +
		get_mm_counter(mm, MM_ANONPAGES) +
		get_mm_counter(mm, MM_SHMEMPAGES);
}

static inline unsigned long get_mm_hiwater_rss(struct mm_struct *mm)
{
	return max(mm->hiwater_rss, get_mm_rss(mm));
}

static inline unsigned long get_mm_hiwater_vm(struct mm_struct *mm)
{
	return max(mm->hiwater_vm, mm->total_vm);
}

static inline void update_hiwater_rss(struct mm_struct *mm)
{
	unsigned long _rss = get_mm_rss(mm);

	if ((mm)->hiwater_rss < _rss)
		(mm)->hiwater_rss = _rss;
}

static inline void update_hiwater_vm(struct mm_struct *mm)
{
	if (mm->hiwater_vm < mm->total_vm)
		mm->hiwater_vm = mm->total_vm;
}

static inline void reset_mm_hiwater_rss(struct mm_struct *mm)
{
	mm->hiwater_rss = get_mm_rss(mm);
}

static inline void setmax_mm_hiwater_rss(unsigned long *maxrss,
					 struct mm_struct *mm)
{
	unsigned long hiwater_rss = get_mm_hiwater_rss(mm);

	if (*maxrss < hiwater_rss)
		*maxrss = hiwater_rss;
}

#if defined(SPLIT_RSS_COUNTING)
void sync_mm_rss(struct mm_struct *mm);
#else
static inline void sync_mm_rss(struct mm_struct *mm)
{
}
#endif

#ifndef CONFIG_ARCH_HAS_PTE_SPECIAL
static inline int pte_special(pte_t pte)
{
	return 0;
}

static inline pte_t pte_mkspecial(pte_t pte)
{
	return pte;
}
#endif

#ifndef CONFIG_ARCH_HAS_PTE_DEVMAP
static inline int pte_devmap(pte_t pte)
{
	return 0;
}
#endif

int vma_wants_writenotify(struct vm_area_struct *vma, pgprot_t vm_page_prot);

extern pte_t *__get_locked_pte(struct mm_struct *mm, unsigned long addr,
			       spinlock_t **ptl);
static inline pte_t *get_locked_pte(struct mm_struct *mm, unsigned long addr,
				    spinlock_t **ptl)
{
	pte_t *ptep;
	__cond_lock(*ptl, ptep = __get_locked_pte(mm, addr, ptl));
	return ptep;
}

#ifdef __PAGETABLE_P4D_FOLDED
static inline int __p4d_alloc(struct mm_struct *mm, pgd_t *pgd,
						unsigned long address)
{
	return 0;
}
#else
int __p4d_alloc(struct mm_struct *mm, pgd_t *pgd, unsigned long address);
#endif

#if defined(__PAGETABLE_PUD_FOLDED) || !defined(CONFIG_MMU)
static inline int __pud_alloc(struct mm_struct *mm, p4d_t *p4d,
						unsigned long address)
{
	return 0;
}
static inline void mm_inc_nr_puds(struct mm_struct *mm) {}
static inline void mm_dec_nr_puds(struct mm_struct *mm) {}

#else
int __pud_alloc(struct mm_struct *mm, p4d_t *p4d, unsigned long address);

static inline void mm_inc_nr_puds(struct mm_struct *mm)
{
	if (mm_pud_folded(mm))
		return;
	atomic_long_add(PTRS_PER_PUD * sizeof(pud_t), &mm->pgtables_bytes);
}

static inline void mm_dec_nr_puds(struct mm_struct *mm)
{
	if (mm_pud_folded(mm))
		return;
	atomic_long_sub(PTRS_PER_PUD * sizeof(pud_t), &mm->pgtables_bytes);
}
#endif

#if defined(__PAGETABLE_PMD_FOLDED) || !defined(CONFIG_MMU)
static inline int __pmd_alloc(struct mm_struct *mm, pud_t *pud,
						unsigned long address)
{
	return 0;
}

static inline void mm_inc_nr_pmds(struct mm_struct *mm) {}
static inline void mm_dec_nr_pmds(struct mm_struct *mm) {}

#else
int __pmd_alloc(struct mm_struct *mm, pud_t *pud, unsigned long address);

static inline void mm_inc_nr_pmds(struct mm_struct *mm)
{
	if (mm_pmd_folded(mm))
		return;
	atomic_long_add(PTRS_PER_PMD * sizeof(pmd_t), &mm->pgtables_bytes);
}

static inline void mm_dec_nr_pmds(struct mm_struct *mm)
{
	if (mm_pmd_folded(mm))
		return;
	atomic_long_sub(PTRS_PER_PMD * sizeof(pmd_t), &mm->pgtables_bytes);
}
#endif

#ifdef CONFIG_MMU
static inline void mm_pgtables_bytes_init(struct mm_struct *mm)
{
	atomic_long_set(&mm->pgtables_bytes, 0);
}

static inline unsigned long mm_pgtables_bytes(const struct mm_struct *mm)
{
	return atomic_long_read(&mm->pgtables_bytes);
}

static inline void mm_inc_nr_ptes(struct mm_struct *mm)
{
	atomic_long_add(PTRS_PER_PTE * sizeof(pte_t), &mm->pgtables_bytes);
}

static inline void mm_dec_nr_ptes(struct mm_struct *mm)
{
	atomic_long_sub(PTRS_PER_PTE * sizeof(pte_t), &mm->pgtables_bytes);
}
#else

static inline void mm_pgtables_bytes_init(struct mm_struct *mm) {}
static inline unsigned long mm_pgtables_bytes(const struct mm_struct *mm)
{
	return 0;
}

static inline void mm_inc_nr_ptes(struct mm_struct *mm) {}
static inline void mm_dec_nr_ptes(struct mm_struct *mm) {}
#endif

int __pte_alloc(struct mm_struct *mm, pmd_t *pmd);
int __pte_alloc_kernel(pmd_t *pmd);

#if defined(CONFIG_MMU)

static inline p4d_t *p4d_alloc(struct mm_struct *mm, pgd_t *pgd,
		unsigned long address)
{
	return (unlikely(pgd_none(*pgd)) && __p4d_alloc(mm, pgd, address)) ?
		NULL : p4d_offset(pgd, address);
}

static inline pud_t *pud_alloc(struct mm_struct *mm, p4d_t *p4d,
		unsigned long address)
{
	return (unlikely(p4d_none(*p4d)) && __pud_alloc(mm, p4d, address)) ?
		NULL : pud_offset(p4d, address);
}

static inline pmd_t *pmd_alloc(struct mm_struct *mm, pud_t *pud, unsigned long address)
{
	return (unlikely(pud_none(*pud)) && __pmd_alloc(mm, pud, address))?
		NULL: pmd_offset(pud, address);
}
#endif 

#if USE_SPLIT_PTE_PTLOCKS
#if ALLOC_SPLIT_PTLOCKS
void __init ptlock_cache_init(void);
extern bool ptlock_alloc(struct page *page);
extern void ptlock_free(struct page *page);

static inline spinlock_t *ptlock_ptr(struct page *page)
{
	return page->ptl;
}
#else 
static inline void ptlock_cache_init(void)
{
}

static inline bool ptlock_alloc(struct page *page)
{
	return true;
}

static inline void ptlock_free(struct page *page)
{
}

static inline spinlock_t *ptlock_ptr(struct page *page)
{
	return &page->ptl;
}
#endif 

static inline spinlock_t *pte_lockptr(struct mm_struct *mm, pmd_t *pmd)
{
	return ptlock_ptr(pmd_page(*pmd));
}

static inline bool ptlock_init(struct page *page)
{
	
	VM_BUG_ON_PAGE(*(unsigned long *)&page->ptl, page);
	if (!ptlock_alloc(page))
		return false;
	spin_lock_init(ptlock_ptr(page));
	return true;
}

#else	

static inline spinlock_t *pte_lockptr(struct mm_struct *mm, pmd_t *pmd)
{
	return &mm->page_table_lock;
}
static inline void ptlock_cache_init(void) {}
static inline bool ptlock_init(struct page *page) { return true; }
static inline void ptlock_free(struct page *page) {}
#endif 

static inline void pgtable_init(void)
{
	ptlock_cache_init();
	pgtable_cache_init();
}

static inline bool pgtable_pte_page_ctor(struct page *page)
{
	if (!ptlock_init(page))
		return false;
	__SetPageTable(page);
	inc_lruvec_page_state(page, NR_PAGETABLE);
	return true;
}

static inline void pgtable_pte_page_dtor(struct page *page)
{
	ptlock_free(page);
	__ClearPageTable(page);
	dec_lruvec_page_state(page, NR_PAGETABLE);
}

#define pte_offset_map_lock(mm, pmd, address, ptlp)	\
({							\
	spinlock_t *__ptl = pte_lockptr(mm, pmd);	\
	pte_t *__pte = pte_offset_map(pmd, address);	\
	*(ptlp) = __ptl;				\
	spin_lock(__ptl);				\
	__pte;						\
})

#define pte_unmap_unlock(pte, ptl)	do {		\
	spin_unlock(ptl);				\
	pte_unmap(pte);					\
} while (0)

#define pte_alloc(mm, pmd) (unlikely(pmd_none(*(pmd))) && __pte_alloc(mm, pmd))

#define pte_alloc_map(mm, pmd, address)			\
	(pte_alloc(mm, pmd) ? NULL : pte_offset_map(pmd, address))

#define pte_alloc_map_lock(mm, pmd, address, ptlp)	\
	(pte_alloc(mm, pmd) ?			\
		 NULL : pte_offset_map_lock(mm, pmd, address, ptlp))

#define pte_alloc_kernel(pmd, address)			\
	((unlikely(pmd_none(*(pmd))) && __pte_alloc_kernel(pmd))? \
		NULL: pte_offset_kernel(pmd, address))

#if USE_SPLIT_PMD_PTLOCKS

static struct page *pmd_to_page(pmd_t *pmd)
{
	unsigned long mask = ~(PTRS_PER_PMD * sizeof(pmd_t) - 1);
	return virt_to_page((void *)((unsigned long) pmd & mask));
}

static inline spinlock_t *pmd_lockptr(struct mm_struct *mm, pmd_t *pmd)
{
	return ptlock_ptr(pmd_to_page(pmd));
}

static inline bool pmd_ptlock_init(struct page *page)
{
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	page->pmd_huge_pte = NULL;
#endif
	return ptlock_init(page);
}

static inline void pmd_ptlock_free(struct page *page)
{
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	VM_BUG_ON_PAGE(page->pmd_huge_pte, page);
#endif
	ptlock_free(page);
}

#define pmd_huge_pte(mm, pmd) (pmd_to_page(pmd)->pmd_huge_pte)

#else

static inline spinlock_t *pmd_lockptr(struct mm_struct *mm, pmd_t *pmd)
{
	return &mm->page_table_lock;
}

static inline bool pmd_ptlock_init(struct page *page) { return true; }
static inline void pmd_ptlock_free(struct page *page) {}

#define pmd_huge_pte(mm, pmd) ((mm)->pmd_huge_pte)

#endif

static inline spinlock_t *pmd_lock(struct mm_struct *mm, pmd_t *pmd)
{
	spinlock_t *ptl = pmd_lockptr(mm, pmd);
	spin_lock(ptl);
	return ptl;
}

static inline bool pgtable_pmd_page_ctor(struct page *page)
{
	if (!pmd_ptlock_init(page))
		return false;
	__SetPageTable(page);
	inc_lruvec_page_state(page, NR_PAGETABLE);
	return true;
}

static inline void pgtable_pmd_page_dtor(struct page *page)
{
	pmd_ptlock_free(page);
	__ClearPageTable(page);
	dec_lruvec_page_state(page, NR_PAGETABLE);
}


static inline spinlock_t *pud_lockptr(struct mm_struct *mm, pud_t *pud)
{
	return &mm->page_table_lock;
}

static inline spinlock_t *pud_lock(struct mm_struct *mm, pud_t *pud)
{
	spinlock_t *ptl = pud_lockptr(mm, pud);

	spin_lock(ptl);
	return ptl;
}

extern void __init pagecache_init(void);
extern void free_initmem(void);


extern unsigned long free_reserved_area(void *start, void *end,
					int poison, const char *s);

extern void adjust_managed_page_count(struct page *page, long count);
extern void mem_init_print_info(void);

extern void reserve_bootmem_region(phys_addr_t start, phys_addr_t end);


static inline void free_reserved_page(struct page *page)
{
	ClearPageReserved(page);
	init_page_count(page);
	__free_page(page);
	adjust_managed_page_count(page, 1);
}
#define free_highmem_page(page) free_reserved_page(page)

static inline void mark_page_reserved(struct page *page)
{
	SetPageReserved(page);
	adjust_managed_page_count(page, -1);
}


static inline unsigned long free_initmem_default(int poison)
{
	extern char __init_begin[], __init_end[];

	return free_reserved_area(&__init_begin, &__init_end,
				  poison, "unused kernel image (initmem)");
}

static inline unsigned long get_num_physpages(void)
{
	int nid;
	unsigned long phys_pages = 0;

	for_each_online_node(nid)
		phys_pages += node_present_pages(nid);

	return phys_pages;
}


void free_area_init(unsigned long *max_zone_pfn);
unsigned long node_map_pfn_alignment(void);
unsigned long __absent_pages_in_range(int nid, unsigned long start_pfn,
						unsigned long end_pfn);
extern unsigned long absent_pages_in_range(unsigned long start_pfn,
						unsigned long end_pfn);
extern void get_pfn_range_for_nid(unsigned int nid,
			unsigned long *start_pfn, unsigned long *end_pfn);
extern unsigned long find_min_pfn_with_active_regions(void);

#ifndef CONFIG_NUMA
static inline int early_pfn_to_nid(unsigned long pfn)
{
	return 0;
}
#else

extern int __meminit early_pfn_to_nid(unsigned long pfn);
#endif

extern void set_dma_reserve(unsigned long new_dma_reserve);
extern void memmap_init_range(unsigned long, int, unsigned long,
		unsigned long, unsigned long, enum meminit_context,
		struct vmem_altmap *, int migratetype);
extern void setup_per_zone_wmarks(void);
extern void calculate_min_free_kbytes(void);
extern int __meminit init_per_zone_wmark_min(void);
extern void mem_init(void);
extern void __init mmap_init(void);
extern void show_mem(unsigned int flags, nodemask_t *nodemask);
extern long si_mem_available(void);
extern void si_meminfo(struct sysinfo * val);
extern void si_meminfo_node(struct sysinfo *val, int nid);
#ifdef __HAVE_ARCH_RESERVED_KERNEL_PAGES
extern unsigned long arch_reserved_kernel_pages(void);
#endif

extern __printf(3, 4)
void warn_alloc(gfp_t gfp_mask, nodemask_t *nodemask, const char *fmt, ...);

extern void setup_per_cpu_pageset(void);


extern int min_free_kbytes;
extern int watermark_boost_factor;
extern int watermark_scale_factor;
extern bool arch_has_descending_max_zone_pfns(void);


extern atomic_long_t mmap_pages_allocated;
extern int nommu_shrink_inode_mappings(struct inode *, size_t, size_t);


void vma_interval_tree_insert(struct vm_area_struct *node,
			      struct rb_root_cached *root);
void vma_interval_tree_insert_after(struct vm_area_struct *node,
				    struct vm_area_struct *prev,
				    struct rb_root_cached *root);
void vma_interval_tree_remove(struct vm_area_struct *node,
			      struct rb_root_cached *root);
struct vm_area_struct *vma_interval_tree_iter_first(struct rb_root_cached *root,
				unsigned long start, unsigned long last);
struct vm_area_struct *vma_interval_tree_iter_next(struct vm_area_struct *node,
				unsigned long start, unsigned long last);

#define vma_interval_tree_foreach(vma, root, start, last)		\
	for (vma = vma_interval_tree_iter_first(root, start, last);	\
	     vma; vma = vma_interval_tree_iter_next(vma, start, last))

void anon_vma_interval_tree_insert(struct anon_vma_chain *node,
				   struct rb_root_cached *root);
void anon_vma_interval_tree_remove(struct anon_vma_chain *node,
				   struct rb_root_cached *root);
struct anon_vma_chain *
anon_vma_interval_tree_iter_first(struct rb_root_cached *root,
				  unsigned long start, unsigned long last);
struct anon_vma_chain *anon_vma_interval_tree_iter_next(
	struct anon_vma_chain *node, unsigned long start, unsigned long last);
#ifdef CONFIG_DEBUG_VM_RB
void anon_vma_interval_tree_verify(struct anon_vma_chain *node);
#endif

#define anon_vma_interval_tree_foreach(avc, root, start, last)		 \
	for (avc = anon_vma_interval_tree_iter_first(root, start, last); \
	     avc; avc = anon_vma_interval_tree_iter_next(avc, start, last))


extern int __vm_enough_memory(struct mm_struct *mm, long pages, int cap_sys_admin);
extern int __vma_adjust(struct vm_area_struct *vma, unsigned long start,
	unsigned long end, pgoff_t pgoff, struct vm_area_struct *insert,
	struct vm_area_struct *expand);
static inline int vma_adjust(struct vm_area_struct *vma, unsigned long start,
	unsigned long end, pgoff_t pgoff, struct vm_area_struct *insert)
{
	return __vma_adjust(vma, start, end, pgoff, insert, NULL);
}
extern struct vm_area_struct *vma_merge(struct mm_struct *,
	struct vm_area_struct *prev, unsigned long addr, unsigned long end,
	unsigned long vm_flags, struct anon_vma *, struct file *, pgoff_t,
	struct mempolicy *, struct vm_userfaultfd_ctx, struct anon_vma_name *);
extern struct anon_vma *find_mergeable_anon_vma(struct vm_area_struct *);
extern int __split_vma(struct mm_struct *, struct vm_area_struct *,
	unsigned long addr, int new_below);
extern int split_vma(struct mm_struct *, struct vm_area_struct *,
	unsigned long addr, int new_below);
extern int insert_vm_struct(struct mm_struct *, struct vm_area_struct *);
extern void __vma_link_rb(struct mm_struct *, struct vm_area_struct *,
	struct rb_node **, struct rb_node *);
extern void unlink_file_vma(struct vm_area_struct *);
extern struct vm_area_struct *copy_vma(struct vm_area_struct **,
	unsigned long addr, unsigned long len, pgoff_t pgoff,
	bool *need_rmap_locks);
extern void exit_mmap(struct mm_struct *);

static inline int check_data_rlimit(unsigned long rlim,
				    unsigned long new,
				    unsigned long start,
				    unsigned long end_data,
				    unsigned long start_data)
{
	if (rlim < RLIM_INFINITY) {
		if (((new - start) + (end_data - start_data)) > rlim)
			return -ENOSPC;
	}

	return 0;
}

extern int mm_take_all_locks(struct mm_struct *mm);
extern void mm_drop_all_locks(struct mm_struct *mm);

extern int set_mm_exe_file(struct mm_struct *mm, struct file *new_exe_file);
extern int replace_mm_exe_file(struct mm_struct *mm, struct file *new_exe_file);
extern struct file *get_mm_exe_file(struct mm_struct *mm);
extern struct file *get_task_exe_file(struct task_struct *task);

extern bool may_expand_vm(struct mm_struct *, vm_flags_t, unsigned long npages);
extern void vm_stat_account(struct mm_struct *, vm_flags_t, long npages);

extern bool vma_is_special_mapping(const struct vm_area_struct *vma,
				   const struct vm_special_mapping *sm);
extern struct vm_area_struct *_install_special_mapping(struct mm_struct *mm,
				   unsigned long addr, unsigned long len,
				   unsigned long flags,
				   const struct vm_special_mapping *spec);

extern int install_special_mapping(struct mm_struct *mm,
				   unsigned long addr, unsigned long len,
				   unsigned long flags, struct page **pages);

unsigned long randomize_stack_top(unsigned long stack_top);
unsigned long randomize_page(unsigned long start, unsigned long range);

extern unsigned long get_unmapped_area(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);

extern unsigned long mmap_region(struct file *file, unsigned long addr,
	unsigned long len, vm_flags_t vm_flags, unsigned long pgoff,
	struct list_head *uf);
extern unsigned long do_mmap(struct file *file, unsigned long addr,
	unsigned long len, unsigned long prot, unsigned long flags,
	unsigned long pgoff, unsigned long *populate, struct list_head *uf);
extern int __do_munmap(struct mm_struct *, unsigned long, size_t,
		       struct list_head *uf, bool downgrade);
extern int do_munmap(struct mm_struct *, unsigned long, size_t,
		     struct list_head *uf);
extern int do_madvise(struct mm_struct *mm, unsigned long start, size_t len_in, int behavior);

#ifdef CONFIG_MMU
extern int __mm_populate(unsigned long addr, unsigned long len,
			 int ignore_errors);
static inline void mm_populate(unsigned long addr, unsigned long len)
{
	
	(void) __mm_populate(addr, len, 1);
}
#else
static inline void mm_populate(unsigned long addr, unsigned long len) {}
#endif


extern int __must_check vm_brk(unsigned long, unsigned long);
extern int __must_check vm_brk_flags(unsigned long, unsigned long, unsigned long);
extern int vm_munmap(unsigned long, size_t);
extern unsigned long __must_check vm_mmap(struct file *, unsigned long,
        unsigned long, unsigned long,
        unsigned long, unsigned long);

struct vm_unmapped_area_info {
#define VM_UNMAPPED_AREA_TOPDOWN 1
	unsigned long flags;
	unsigned long length;
	unsigned long low_limit;
	unsigned long high_limit;
	unsigned long align_mask;
	unsigned long align_offset;
};

extern unsigned long vm_unmapped_area(struct vm_unmapped_area_info *info);


extern void truncate_inode_pages(struct address_space *, loff_t);
extern void truncate_inode_pages_range(struct address_space *,
				       loff_t lstart, loff_t lend);
extern void truncate_inode_pages_final(struct address_space *);


extern vm_fault_t filemap_fault(struct vm_fault *vmf);
extern vm_fault_t filemap_map_pages(struct vm_fault *vmf,
		pgoff_t start_pgoff, pgoff_t end_pgoff);
extern vm_fault_t filemap_page_mkwrite(struct vm_fault *vmf);

extern unsigned long stack_guard_gap;

extern int expand_stack(struct vm_area_struct *vma, unsigned long address);


extern int expand_downwards(struct vm_area_struct *vma,
		unsigned long address);
#if VM_GROWSUP
extern int expand_upwards(struct vm_area_struct *vma, unsigned long address);
#else
  #define expand_upwards(vma, address) (0)
#endif


extern struct vm_area_struct * find_vma(struct mm_struct * mm, unsigned long addr);
extern struct vm_area_struct * find_vma_prev(struct mm_struct * mm, unsigned long addr,
					     struct vm_area_struct **pprev);


static inline
struct vm_area_struct *find_vma_intersection(struct mm_struct *mm,
					     unsigned long start_addr,
					     unsigned long end_addr)
{
	struct vm_area_struct *vma = find_vma(mm, start_addr);

	if (vma && end_addr <= vma->vm_start)
		vma = NULL;
	return vma;
}


static inline
struct vm_area_struct *vma_lookup(struct mm_struct *mm, unsigned long addr)
{
	struct vm_area_struct *vma = find_vma(mm, addr);

	if (vma && addr < vma->vm_start)
		vma = NULL;

	return vma;
}

static inline unsigned long vm_start_gap(struct vm_area_struct *vma)
{
	unsigned long vm_start = vma->vm_start;

	if (vma->vm_flags & VM_GROWSDOWN) {
		vm_start -= stack_guard_gap;
		if (vm_start > vma->vm_start)
			vm_start = 0;
	}
	return vm_start;
}

static inline unsigned long vm_end_gap(struct vm_area_struct *vma)
{
	unsigned long vm_end = vma->vm_end;

	if (vma->vm_flags & VM_GROWSUP) {
		vm_end += stack_guard_gap;
		if (vm_end < vma->vm_end)
			vm_end = -PAGE_SIZE;
	}
	return vm_end;
}

static inline unsigned long vma_pages(struct vm_area_struct *vma)
{
	return (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;
}


static inline struct vm_area_struct *find_exact_vma(struct mm_struct *mm,
				unsigned long vm_start, unsigned long vm_end)
{
	struct vm_area_struct *vma = find_vma(mm, vm_start);

	if (vma && (vma->vm_start != vm_start || vma->vm_end != vm_end))
		vma = NULL;

	return vma;
}

static inline bool range_in_vma(struct vm_area_struct *vma,
				unsigned long start, unsigned long end)
{
	return (vma && vma->vm_start <= start && end <= vma->vm_end);
}

#ifdef CONFIG_MMU
pgprot_t vm_get_page_prot(unsigned long vm_flags);
void vma_set_page_prot(struct vm_area_struct *vma);
#else
static inline pgprot_t vm_get_page_prot(unsigned long vm_flags)
{
	return __pgprot(0);
}
static inline void vma_set_page_prot(struct vm_area_struct *vma)
{
	vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
}
#endif

void vma_set_file(struct vm_area_struct *vma, struct file *file);

#ifdef CONFIG_NUMA_BALANCING
unsigned long change_prot_numa(struct vm_area_struct *vma,
			unsigned long start, unsigned long end);
#endif

struct vm_area_struct *find_extend_vma(struct mm_struct *, unsigned long addr);
int remap_pfn_range(struct vm_area_struct *, unsigned long addr,
			unsigned long pfn, unsigned long size, pgprot_t);
int remap_pfn_range_notrack(struct vm_area_struct *vma, unsigned long addr,
		unsigned long pfn, unsigned long size, pgprot_t prot);
int vm_insert_page(struct vm_area_struct *, unsigned long addr, struct page *);
int vm_insert_pages(struct vm_area_struct *vma, unsigned long addr,
			struct page **pages, unsigned long *num);
int vm_map_pages(struct vm_area_struct *vma, struct page **pages,
				unsigned long num);
int vm_map_pages_zero(struct vm_area_struct *vma, struct page **pages,
				unsigned long num);
vm_fault_t vmf_insert_pfn(struct vm_area_struct *vma, unsigned long addr,
			unsigned long pfn);
vm_fault_t vmf_insert_pfn_prot(struct vm_area_struct *vma, unsigned long addr,
			unsigned long pfn, pgprot_t pgprot);
vm_fault_t vmf_insert_mixed(struct vm_area_struct *vma, unsigned long addr,
			pfn_t pfn);
vm_fault_t vmf_insert_mixed_prot(struct vm_area_struct *vma, unsigned long addr,
			pfn_t pfn, pgprot_t pgprot);
vm_fault_t vmf_insert_mixed_mkwrite(struct vm_area_struct *vma,
		unsigned long addr, pfn_t pfn);
int vm_iomap_memory(struct vm_area_struct *vma, phys_addr_t start, unsigned long len);

static inline vm_fault_t vmf_insert_page(struct vm_area_struct *vma,
				unsigned long addr, struct page *page)
{
	int err = vm_insert_page(vma, addr, page);

	if (err == -ENOMEM)
		return VM_FAULT_OOM;
	if (err < 0 && err != -EBUSY)
		return VM_FAULT_SIGBUS;

	return VM_FAULT_NOPAGE;
}

#ifndef io_remap_pfn_range
static inline int io_remap_pfn_range(struct vm_area_struct *vma,
				     unsigned long addr, unsigned long pfn,
				     unsigned long size, pgprot_t prot)
{
	return remap_pfn_range(vma, addr, pfn, size, pgprot_decrypted(prot));
}
#endif

static inline vm_fault_t vmf_error(int err)
{
	if (err == -ENOMEM)
		return VM_FAULT_OOM;
	return VM_FAULT_SIGBUS;
}

struct page *follow_page(struct vm_area_struct *vma, unsigned long address,
			 unsigned int foll_flags);

#define FOLL_WRITE	0x01	
#define FOLL_TOUCH	0x02	
#define FOLL_GET	0x04	
#define FOLL_DUMP	0x08	
#define FOLL_FORCE	0x10	
#define FOLL_NOWAIT	0x20	
#define FOLL_NOFAULT	0x80	
#define FOLL_HWPOISON	0x100	
#define FOLL_NUMA	0x200	
#define FOLL_MIGRATION	0x400	
#define FOLL_TRIED	0x800	
#define FOLL_REMOTE	0x2000	
#define FOLL_ANON	0x8000	
#define FOLL_LONGTERM	0x10000	
#define FOLL_SPLIT_PMD	0x20000	
#define FOLL_PIN	0x40000	
#define FOLL_FAST_ONLY	0x80000	



static inline int vm_fault_to_errno(vm_fault_t vm_fault, int foll_flags)
{
	if (vm_fault & VM_FAULT_OOM)
		return -ENOMEM;
	if (vm_fault & (VM_FAULT_HWPOISON | VM_FAULT_HWPOISON_LARGE))
		return (foll_flags & FOLL_HWPOISON) ? -EHWPOISON : -EFAULT;
	if (vm_fault & (VM_FAULT_SIGBUS | VM_FAULT_SIGSEGV))
		return -EFAULT;
	return 0;
}


static inline bool gup_must_unshare(unsigned int flags, struct page *page)
{
	
	if ((flags & (FOLL_WRITE | FOLL_PIN)) != FOLL_PIN)
		return false;
	
	if (!PageAnon(page))
		return false;
	
	return !PageAnonExclusive(page);
}

typedef int (*pte_fn_t)(pte_t *pte, unsigned long addr, void *data);
extern int apply_to_page_range(struct mm_struct *mm, unsigned long address,
			       unsigned long size, pte_fn_t fn, void *data);
extern int apply_to_existing_page_range(struct mm_struct *mm,
				   unsigned long address, unsigned long size,
				   pte_fn_t fn, void *data);

extern void init_mem_debugging_and_hardening(void);
#ifdef CONFIG_PAGE_POISONING
extern void __kernel_poison_pages(struct page *page, int numpages);
extern void __kernel_unpoison_pages(struct page *page, int numpages);
extern bool _page_poisoning_enabled_early;
DECLARE_STATIC_KEY_FALSE(_page_poisoning_enabled);
static inline bool page_poisoning_enabled(void)
{
	return _page_poisoning_enabled_early;
}

static inline bool page_poisoning_enabled_static(void)
{
	return static_branch_unlikely(&_page_poisoning_enabled);
}
static inline void kernel_poison_pages(struct page *page, int numpages)
{
	if (page_poisoning_enabled_static())
		__kernel_poison_pages(page, numpages);
}
static inline void kernel_unpoison_pages(struct page *page, int numpages)
{
	if (page_poisoning_enabled_static())
		__kernel_unpoison_pages(page, numpages);
}
#else
static inline bool page_poisoning_enabled(void) { return false; }
static inline bool page_poisoning_enabled_static(void) { return false; }
static inline void __kernel_poison_pages(struct page *page, int nunmpages) { }
static inline void kernel_poison_pages(struct page *page, int numpages) { }
static inline void kernel_unpoison_pages(struct page *page, int numpages) { }
#endif

DECLARE_STATIC_KEY_MAYBE(CONFIG_INIT_ON_ALLOC_DEFAULT_ON, init_on_alloc);
static inline bool want_init_on_alloc(gfp_t flags)
{
	if (static_branch_maybe(CONFIG_INIT_ON_ALLOC_DEFAULT_ON,
				&init_on_alloc))
		return true;
	return flags & __GFP_ZERO;
}

DECLARE_STATIC_KEY_MAYBE(CONFIG_INIT_ON_FREE_DEFAULT_ON, init_on_free);
static inline bool want_init_on_free(void)
{
	return static_branch_maybe(CONFIG_INIT_ON_FREE_DEFAULT_ON,
				   &init_on_free);
}

extern bool _debug_pagealloc_enabled_early;
DECLARE_STATIC_KEY_FALSE(_debug_pagealloc_enabled);

static inline bool debug_pagealloc_enabled(void)
{
	return IS_ENABLED(CONFIG_DEBUG_PAGEALLOC) &&
		_debug_pagealloc_enabled_early;
}


static inline bool debug_pagealloc_enabled_static(void)
{
	if (!IS_ENABLED(CONFIG_DEBUG_PAGEALLOC))
		return false;

	return static_branch_unlikely(&_debug_pagealloc_enabled);
}

#ifdef CONFIG_DEBUG_PAGEALLOC

extern void __kernel_map_pages(struct page *page, int numpages, int enable);

static inline void debug_pagealloc_map_pages(struct page *page, int numpages)
{
	if (debug_pagealloc_enabled_static())
		__kernel_map_pages(page, numpages, 1);
}

static inline void debug_pagealloc_unmap_pages(struct page *page, int numpages)
{
	if (debug_pagealloc_enabled_static())
		__kernel_map_pages(page, numpages, 0);
}
#else	
static inline void debug_pagealloc_map_pages(struct page *page, int numpages) {}
static inline void debug_pagealloc_unmap_pages(struct page *page, int numpages) {}
#endif	

#ifdef __HAVE_ARCH_GATE_AREA
extern struct vm_area_struct *get_gate_vma(struct mm_struct *mm);
extern int in_gate_area_no_mm(unsigned long addr);
extern int in_gate_area(struct mm_struct *mm, unsigned long addr);
#else
static inline struct vm_area_struct *get_gate_vma(struct mm_struct *mm)
{
	return NULL;
}
static inline int in_gate_area_no_mm(unsigned long addr) { return 0; }
static inline int in_gate_area(struct mm_struct *mm, unsigned long addr)
{
	return 0;
}
#endif	

extern bool process_shares_mm(struct task_struct *p, struct mm_struct *mm);

#ifdef CONFIG_SYSCTL
extern int sysctl_drop_caches;
int drop_caches_sysctl_handler(struct ctl_table *, int, void *, size_t *,
		loff_t *);
#endif

void drop_slab(void);

#ifndef CONFIG_MMU
#define randomize_va_space 0
#else
extern int randomize_va_space;
#endif

const char * arch_vma_name(struct vm_area_struct *vma);
#ifdef CONFIG_MMU
void print_vma_addr(char *prefix, unsigned long rip);
#else
static inline void print_vma_addr(char *prefix, unsigned long rip)
{
}
#endif

void *sparse_buffer_alloc(unsigned long size);
struct page * __populate_section_memmap(unsigned long pfn,
		unsigned long nr_pages, int nid, struct vmem_altmap *altmap,
		struct dev_pagemap *pgmap);
pgd_t *vmemmap_pgd_populate(unsigned long addr, int node);
p4d_t *vmemmap_p4d_populate(pgd_t *pgd, unsigned long addr, int node);
pud_t *vmemmap_pud_populate(p4d_t *p4d, unsigned long addr, int node);
pmd_t *vmemmap_pmd_populate(pud_t *pud, unsigned long addr, int node);
pte_t *vmemmap_pte_populate(pmd_t *pmd, unsigned long addr, int node,
			    struct vmem_altmap *altmap, struct page *reuse);
void *vmemmap_alloc_block(unsigned long size, int node);
struct vmem_altmap;
void *vmemmap_alloc_block_buf(unsigned long size, int node,
			      struct vmem_altmap *altmap);
void vmemmap_verify(pte_t *, int, unsigned long, unsigned long);
int vmemmap_populate_basepages(unsigned long start, unsigned long end,
			       int node, struct vmem_altmap *altmap);
int vmemmap_populate(unsigned long start, unsigned long end, int node,
		struct vmem_altmap *altmap);
void vmemmap_populate_print_last(void);
#ifdef CONFIG_MEMORY_HOTPLUG
void vmemmap_free(unsigned long start, unsigned long end,
		struct vmem_altmap *altmap);
#endif
void register_page_bootmem_memmap(unsigned long section_nr, struct page *map,
				  unsigned long nr_pages);

enum mf_flags {
	MF_COUNT_INCREASED = 1 << 0,
	MF_ACTION_REQUIRED = 1 << 1,
	MF_MUST_KILL = 1 << 2,
	MF_SOFT_OFFLINE = 1 << 3,
	MF_UNPOISON = 1 << 4,
	MF_SW_SIMULATED = 1 << 5,
	MF_NO_RETRY = 1 << 6,
};
int mf_dax_kill_procs(struct address_space *mapping, pgoff_t index,
		      unsigned long count, int mf_flags);
extern int memory_failure(unsigned long pfn, int flags);
extern void memory_failure_queue(unsigned long pfn, int flags);
extern void memory_failure_queue_kick(int cpu);
extern int unpoison_memory(unsigned long pfn);
extern int sysctl_memory_failure_early_kill;
extern int sysctl_memory_failure_recovery;
extern void shake_page(struct page *p);
extern atomic_long_t num_poisoned_pages __read_mostly;
extern int soft_offline_page(unsigned long pfn, int flags);
#ifdef CONFIG_MEMORY_FAILURE
extern int __get_huge_page_for_hwpoison(unsigned long pfn, int flags);
#else
static inline int __get_huge_page_for_hwpoison(unsigned long pfn, int flags)
{
	return 0;
}
#endif

#ifndef arch_memory_failure
static inline int arch_memory_failure(unsigned long pfn, int flags)
{
	return -ENXIO;
}
#endif

#ifndef arch_is_platform_page
static inline bool arch_is_platform_page(u64 paddr)
{
	return false;
}
#endif


enum mf_result {
	MF_IGNORED,	
	MF_FAILED,	
	MF_DELAYED,	
	MF_RECOVERED,	
};

enum mf_action_page_type {
	MF_MSG_KERNEL,
	MF_MSG_KERNEL_HIGH_ORDER,
	MF_MSG_SLAB,
	MF_MSG_DIFFERENT_COMPOUND,
	MF_MSG_HUGE,
	MF_MSG_FREE_HUGE,
	MF_MSG_UNMAP_FAILED,
	MF_MSG_DIRTY_SWAPCACHE,
	MF_MSG_CLEAN_SWAPCACHE,
	MF_MSG_DIRTY_MLOCKED_LRU,
	MF_MSG_CLEAN_MLOCKED_LRU,
	MF_MSG_DIRTY_UNEVICTABLE_LRU,
	MF_MSG_CLEAN_UNEVICTABLE_LRU,
	MF_MSG_DIRTY_LRU,
	MF_MSG_CLEAN_LRU,
	MF_MSG_TRUNCATED_LRU,
	MF_MSG_BUDDY,
	MF_MSG_DAX,
	MF_MSG_UNSPLIT_THP,
	MF_MSG_UNKNOWN,
};

#if defined(CONFIG_TRANSPARENT_HUGEPAGE) || defined(CONFIG_HUGETLBFS)
extern void clear_huge_page(struct page *page,
			    unsigned long addr_hint,
			    unsigned int pages_per_huge_page);
extern void copy_user_huge_page(struct page *dst, struct page *src,
				unsigned long addr_hint,
				struct vm_area_struct *vma,
				unsigned int pages_per_huge_page);
extern long copy_huge_page_from_user(struct page *dst_page,
				const void __user *usr_src,
				unsigned int pages_per_huge_page,
				bool allow_pagefault);


static inline bool vma_is_special_huge(const struct vm_area_struct *vma)
{
	return vma_is_dax(vma) || (vma->vm_file &&
				   (vma->vm_flags & (VM_PFNMAP | VM_MIXEDMAP)));
}

#endif 

#ifdef CONFIG_DEBUG_PAGEALLOC
extern unsigned int _debug_guardpage_minorder;
DECLARE_STATIC_KEY_FALSE(_debug_guardpage_enabled);

static inline unsigned int debug_guardpage_minorder(void)
{
	return _debug_guardpage_minorder;
}

static inline bool debug_guardpage_enabled(void)
{
	return static_branch_unlikely(&_debug_guardpage_enabled);
}

static inline bool page_is_guard(struct page *page)
{
	if (!debug_guardpage_enabled())
		return false;

	return PageGuard(page);
}
#else
static inline unsigned int debug_guardpage_minorder(void) { return 0; }
static inline bool debug_guardpage_enabled(void) { return false; }
static inline bool page_is_guard(struct page *page) { return false; }
#endif 

#if MAX_NUMNODES > 1
void __init setup_nr_node_ids(void);
#else
static inline void setup_nr_node_ids(void) {}
#endif

extern int memcmp_pages(struct page *page1, struct page *page2);

static inline int pages_identical(struct page *page1, struct page *page2)
{
	return !memcmp_pages(page1, page2);
}

#ifdef CONFIG_MAPPING_DIRTY_HELPERS
unsigned long clean_record_shared_mapping_range(struct address_space *mapping,
						pgoff_t first_index, pgoff_t nr,
						pgoff_t bitmap_pgoff,
						unsigned long *bitmap,
						pgoff_t *start,
						pgoff_t *end);

unsigned long wp_shared_mapping_range(struct address_space *mapping,
				      pgoff_t first_index, pgoff_t nr);
#endif

extern int sysctl_nr_trim_pages;

#ifdef CONFIG_PRINTK
void mem_dump_obj(void *object);
#else
static inline void mem_dump_obj(void *object) {}
#endif


static inline int seal_check_future_write(int seals, struct vm_area_struct *vma)
{
	if (seals & F_SEAL_FUTURE_WRITE) {
		
		if ((vma->vm_flags & VM_SHARED) && (vma->vm_flags & VM_WRITE))
			return -EPERM;

		
		if (vma->vm_flags & VM_SHARED)
			vma->vm_flags &= ~(VM_MAYWRITE);
	}

	return 0;
}

#ifdef CONFIG_ANON_VMA_NAME
int madvise_set_anon_name(struct mm_struct *mm, unsigned long start,
			  unsigned long len_in,
			  struct anon_vma_name *anon_name);
#else
static inline int
madvise_set_anon_name(struct mm_struct *mm, unsigned long start,
		      unsigned long len_in, struct anon_vma_name *anon_name) {
	return 0;
}
#endif

#endif 
