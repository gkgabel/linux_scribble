/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_PAGEMAP_H
#define _LINUX_PAGEMAP_H


#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/highmem.h>
#include <linux/compiler.h>
#include <linux/uaccess.h>
#include <linux/gfp.h>
#include <linux/bitops.h>
#include <linux/hardirq.h> 
#include <linux/hugetlb_inline.h>

struct folio_batch;

unsigned long invalidate_mapping_pages(struct address_space *mapping,
					pgoff_t start, pgoff_t end);

static inline void invalidate_remote_inode(struct inode *inode)
{
	if (S_ISREG(inode->i_mode) || S_ISDIR(inode->i_mode) ||
	    S_ISLNK(inode->i_mode))
		invalidate_mapping_pages(inode->i_mapping, 0, -1);
}
int invalidate_inode_pages2(struct address_space *mapping);
int invalidate_inode_pages2_range(struct address_space *mapping,
		pgoff_t start, pgoff_t end);
int write_inode_now(struct inode *, int sync);
int filemap_fdatawrite(struct address_space *);
int filemap_flush(struct address_space *);
int filemap_fdatawait_keep_errors(struct address_space *mapping);
int filemap_fdatawait_range(struct address_space *, loff_t lstart, loff_t lend);
int filemap_fdatawait_range_keep_errors(struct address_space *mapping,
		loff_t start_byte, loff_t end_byte);

static inline int filemap_fdatawait(struct address_space *mapping)
{
	return filemap_fdatawait_range(mapping, 0, LLONG_MAX);
}

bool filemap_range_has_page(struct address_space *, loff_t lstart, loff_t lend);
int filemap_write_and_wait_range(struct address_space *mapping,
		loff_t lstart, loff_t lend);
int __filemap_fdatawrite_range(struct address_space *mapping,
		loff_t start, loff_t end, int sync_mode);
int filemap_fdatawrite_range(struct address_space *mapping,
		loff_t start, loff_t end);
int filemap_check_errors(struct address_space *mapping);
void __filemap_set_wb_err(struct address_space *mapping, int err);
int filemap_fdatawrite_wbc(struct address_space *mapping,
			   struct writeback_control *wbc);

static inline int filemap_write_and_wait(struct address_space *mapping)
{
	return filemap_write_and_wait_range(mapping, 0, LLONG_MAX);
}


static inline void filemap_set_wb_err(struct address_space *mapping, int err)
{
	
	if (unlikely(err))
		__filemap_set_wb_err(mapping, err);
}


static inline int filemap_check_wb_err(struct address_space *mapping,
					errseq_t since)
{
	return errseq_check(&mapping->wb_err, since);
}


static inline errseq_t filemap_sample_wb_err(struct address_space *mapping)
{
	return errseq_sample(&mapping->wb_err);
}


static inline errseq_t file_sample_sb_err(struct file *file)
{
	return errseq_sample(&file->f_path.dentry->d_sb->s_wb_err);
}


static inline int inode_drain_writes(struct inode *inode)
{
	inode_dio_wait(inode);
	return filemap_write_and_wait(inode->i_mapping);
}

static inline bool mapping_empty(struct address_space *mapping)
{
	return xa_empty(&mapping->i_pages);
}


static inline bool mapping_shrinkable(struct address_space *mapping)
{
	void *head;

	
	if (IS_ENABLED(CONFIG_HIGHMEM))
		return true;

	
	head = rcu_access_pointer(mapping->i_pages.xa_head);
	if (!head)
		return true;

	
	if (!xa_is_node(head) && xa_is_value(head))
		return true;

	return false;
}


enum mapping_flags {
	AS_EIO		= 0,	
	AS_ENOSPC	= 1,	
	AS_MM_ALL_LOCKS	= 2,	
	AS_UNEVICTABLE	= 3,	
	AS_EXITING	= 4, 	
	
	AS_NO_WRITEBACK_TAGS = 5,
	AS_LARGE_FOLIO_SUPPORT = 6,
};


static inline void mapping_set_error(struct address_space *mapping, int error)
{
	if (likely(!error))
		return;

	
	__filemap_set_wb_err(mapping, error);

	
	if (mapping->host)
		errseq_set(&mapping->host->i_sb->s_wb_err, error);

	
	if (error == -ENOSPC)
		set_bit(AS_ENOSPC, &mapping->flags);
	else
		set_bit(AS_EIO, &mapping->flags);
}

static inline void mapping_set_unevictable(struct address_space *mapping)
{
	set_bit(AS_UNEVICTABLE, &mapping->flags);
}

static inline void mapping_clear_unevictable(struct address_space *mapping)
{
	clear_bit(AS_UNEVICTABLE, &mapping->flags);
}

static inline bool mapping_unevictable(struct address_space *mapping)
{
	return mapping && test_bit(AS_UNEVICTABLE, &mapping->flags);
}

static inline void mapping_set_exiting(struct address_space *mapping)
{
	set_bit(AS_EXITING, &mapping->flags);
}

static inline int mapping_exiting(struct address_space *mapping)
{
	return test_bit(AS_EXITING, &mapping->flags);
}

static inline void mapping_set_no_writeback_tags(struct address_space *mapping)
{
	set_bit(AS_NO_WRITEBACK_TAGS, &mapping->flags);
}

static inline int mapping_use_writeback_tags(struct address_space *mapping)
{
	return !test_bit(AS_NO_WRITEBACK_TAGS, &mapping->flags);
}

static inline gfp_t mapping_gfp_mask(struct address_space * mapping)
{
	return mapping->gfp_mask;
}


static inline gfp_t mapping_gfp_constraint(struct address_space *mapping,
		gfp_t gfp_mask)
{
	return mapping_gfp_mask(mapping) & gfp_mask;
}


static inline void mapping_set_gfp_mask(struct address_space *m, gfp_t mask)
{
	m->gfp_mask = mask;
}


static inline void mapping_set_large_folios(struct address_space *mapping)
{
	__set_bit(AS_LARGE_FOLIO_SUPPORT, &mapping->flags);
}


static inline bool mapping_large_folio_support(struct address_space *mapping)
{
	return IS_ENABLED(CONFIG_TRANSPARENT_HUGEPAGE) &&
		test_bit(AS_LARGE_FOLIO_SUPPORT, &mapping->flags);
}

static inline int filemap_nr_thps(struct address_space *mapping)
{
#ifdef CONFIG_READ_ONLY_THP_FOR_FS
	return atomic_read(&mapping->nr_thps);
#else
	return 0;
#endif
}

static inline void filemap_nr_thps_inc(struct address_space *mapping)
{
#ifdef CONFIG_READ_ONLY_THP_FOR_FS
	if (!mapping_large_folio_support(mapping))
		atomic_inc(&mapping->nr_thps);
#else
	WARN_ON_ONCE(mapping_large_folio_support(mapping) == 0);
#endif
}

static inline void filemap_nr_thps_dec(struct address_space *mapping)
{
#ifdef CONFIG_READ_ONLY_THP_FOR_FS
	if (!mapping_large_folio_support(mapping))
		atomic_dec(&mapping->nr_thps);
#else
	WARN_ON_ONCE(mapping_large_folio_support(mapping) == 0);
#endif
}

struct address_space *page_mapping(struct page *);
struct address_space *folio_mapping(struct folio *);
struct address_space *swapcache_mapping(struct folio *);


static inline struct address_space *folio_file_mapping(struct folio *folio)
{
	if (unlikely(folio_test_swapcache(folio)))
		return swapcache_mapping(folio);

	return folio->mapping;
}

static inline struct address_space *page_file_mapping(struct page *page)
{
	return folio_file_mapping(page_folio(page));
}


static inline struct address_space *page_mapping_file(struct page *page)
{
	struct folio *folio = page_folio(page);

	if (unlikely(folio_test_swapcache(folio)))
		return NULL;
	return folio_mapping(folio);
}


static inline struct inode *folio_inode(struct folio *folio)
{
	return folio->mapping->host;
}


static inline void folio_attach_private(struct folio *folio, void *data)
{
	folio_get(folio);
	folio->private = data;
	folio_set_private(folio);
}


static inline void *folio_change_private(struct folio *folio, void *data)
{
	void *old = folio_get_private(folio);

	folio->private = data;
	return old;
}


static inline void *folio_detach_private(struct folio *folio)
{
	void *data = folio_get_private(folio);

	if (!folio_test_private(folio))
		return NULL;
	folio_clear_private(folio);
	folio->private = NULL;
	folio_put(folio);

	return data;
}

static inline void attach_page_private(struct page *page, void *data)
{
	folio_attach_private(page_folio(page), data);
}

static inline void *detach_page_private(struct page *page)
{
	return folio_detach_private(page_folio(page));
}

#ifdef CONFIG_NUMA
struct folio *filemap_alloc_folio(gfp_t gfp, unsigned int order);
#else
static inline struct folio *filemap_alloc_folio(gfp_t gfp, unsigned int order)
{
	return folio_alloc(gfp, order);
}
#endif

static inline struct page *__page_cache_alloc(gfp_t gfp)
{
	return &filemap_alloc_folio(gfp, 0)->page;
}

static inline struct page *page_cache_alloc(struct address_space *x)
{
	return __page_cache_alloc(mapping_gfp_mask(x));
}

static inline gfp_t readahead_gfp_mask(struct address_space *x)
{
	return mapping_gfp_mask(x) | __GFP_NORETRY | __GFP_NOWARN;
}

typedef int filler_t(struct file *, struct folio *);

pgoff_t page_cache_next_miss(struct address_space *mapping,
			     pgoff_t index, unsigned long max_scan);
pgoff_t page_cache_prev_miss(struct address_space *mapping,
			     pgoff_t index, unsigned long max_scan);

#define FGP_ACCESSED		0x00000001
#define FGP_LOCK		0x00000002
#define FGP_CREAT		0x00000004
#define FGP_WRITE		0x00000008
#define FGP_NOFS		0x00000010
#define FGP_NOWAIT		0x00000020
#define FGP_FOR_MMAP		0x00000040
#define FGP_HEAD		0x00000080
#define FGP_ENTRY		0x00000100
#define FGP_STABLE		0x00000200

struct folio *__filemap_get_folio(struct address_space *mapping, pgoff_t index,
		int fgp_flags, gfp_t gfp);
struct page *pagecache_get_page(struct address_space *mapping, pgoff_t index,
		int fgp_flags, gfp_t gfp);


static inline struct folio *filemap_get_folio(struct address_space *mapping,
					pgoff_t index)
{
	return __filemap_get_folio(mapping, index, 0, 0);
}


static inline struct folio *filemap_lock_folio(struct address_space *mapping,
					pgoff_t index)
{
	return __filemap_get_folio(mapping, index, FGP_LOCK, 0);
}


static inline struct page *find_get_page(struct address_space *mapping,
					pgoff_t offset)
{
	return pagecache_get_page(mapping, offset, 0, 0);
}

static inline struct page *find_get_page_flags(struct address_space *mapping,
					pgoff_t offset, int fgp_flags)
{
	return pagecache_get_page(mapping, offset, fgp_flags, 0);
}


static inline struct page *find_lock_page(struct address_space *mapping,
					pgoff_t index)
{
	return pagecache_get_page(mapping, index, FGP_LOCK, 0);
}


static inline struct page *find_or_create_page(struct address_space *mapping,
					pgoff_t index, gfp_t gfp_mask)
{
	return pagecache_get_page(mapping, index,
					FGP_LOCK|FGP_ACCESSED|FGP_CREAT,
					gfp_mask);
}


static inline struct page *grab_cache_page_nowait(struct address_space *mapping,
				pgoff_t index)
{
	return pagecache_get_page(mapping, index,
			FGP_LOCK|FGP_CREAT|FGP_NOFS|FGP_NOWAIT,
			mapping_gfp_mask(mapping));
}

#define swapcache_index(folio)	__page_file_index(&(folio)->page)


static inline pgoff_t folio_index(struct folio *folio)
{
        if (unlikely(folio_test_swapcache(folio)))
                return swapcache_index(folio);
        return folio->index;
}


static inline pgoff_t folio_next_index(struct folio *folio)
{
	return folio->index + folio_nr_pages(folio);
}


static inline struct page *folio_file_page(struct folio *folio, pgoff_t index)
{
	
	if (folio_test_hugetlb(folio))
		return &folio->page;
	return folio_page(folio, index & (folio_nr_pages(folio) - 1));
}


static inline bool folio_contains(struct folio *folio, pgoff_t index)
{
	
	if (folio_test_hugetlb(folio))
		return folio->index == index;
	return index - folio_index(folio) < folio_nr_pages(folio);
}


static inline struct page *find_subpage(struct page *head, pgoff_t index)
{
	
	if (PageHuge(head))
		return head;

	return head + (index & (thp_nr_pages(head) - 1));
}

unsigned filemap_get_folios(struct address_space *mapping, pgoff_t *start,
		pgoff_t end, struct folio_batch *fbatch);
unsigned find_get_pages_contig(struct address_space *mapping, pgoff_t start,
			       unsigned int nr_pages, struct page **pages);
unsigned find_get_pages_range_tag(struct address_space *mapping, pgoff_t *index,
			pgoff_t end, xa_mark_t tag, unsigned int nr_pages,
			struct page **pages);
static inline unsigned find_get_pages_tag(struct address_space *mapping,
			pgoff_t *index, xa_mark_t tag, unsigned int nr_pages,
			struct page **pages)
{
	return find_get_pages_range_tag(mapping, index, (pgoff_t)-1, tag,
					nr_pages, pages);
}

struct page *grab_cache_page_write_begin(struct address_space *mapping,
			pgoff_t index);


static inline struct page *grab_cache_page(struct address_space *mapping,
								pgoff_t index)
{
	return find_or_create_page(mapping, index, mapping_gfp_mask(mapping));
}

struct folio *read_cache_folio(struct address_space *, pgoff_t index,
		filler_t *filler, struct file *file);
struct page *read_cache_page(struct address_space *, pgoff_t index,
		filler_t *filler, struct file *file);
extern struct page * read_cache_page_gfp(struct address_space *mapping,
				pgoff_t index, gfp_t gfp_mask);

static inline struct page *read_mapping_page(struct address_space *mapping,
				pgoff_t index, struct file *file)
{
	return read_cache_page(mapping, index, NULL, file);
}

static inline struct folio *read_mapping_folio(struct address_space *mapping,
				pgoff_t index, struct file *file)
{
	return read_cache_folio(mapping, index, NULL, file);
}


static inline pgoff_t page_to_index(struct page *page)
{
	struct page *head;

	if (likely(!PageTransTail(page)))
		return page->index;

	head = compound_head(page);
	
	return head->index + page - head;
}

extern pgoff_t hugetlb_basepage_index(struct page *page);


static inline pgoff_t page_to_pgoff(struct page *page)
{
	if (unlikely(PageHuge(page)))
		return hugetlb_basepage_index(page);
	return page_to_index(page);
}


static inline loff_t page_offset(struct page *page)
{
	return ((loff_t)page->index) << PAGE_SHIFT;
}

static inline loff_t page_file_offset(struct page *page)
{
	return ((loff_t)page_index(page)) << PAGE_SHIFT;
}


static inline loff_t folio_pos(struct folio *folio)
{
	return page_offset(&folio->page);
}


static inline loff_t folio_file_pos(struct folio *folio)
{
	return page_file_offset(&folio->page);
}


static inline pgoff_t folio_pgoff(struct folio *folio)
{
	if (unlikely(folio_test_hugetlb(folio)))
		return hugetlb_basepage_index(&folio->page);
	return folio->index;
}

extern pgoff_t linear_hugepage_index(struct vm_area_struct *vma,
				     unsigned long address);

static inline pgoff_t linear_page_index(struct vm_area_struct *vma,
					unsigned long address)
{
	pgoff_t pgoff;
	if (unlikely(is_vm_hugetlb_page(vma)))
		return linear_hugepage_index(vma, address);
	pgoff = (address - vma->vm_start) >> PAGE_SHIFT;
	pgoff += vma->vm_pgoff;
	return pgoff;
}

struct wait_page_key {
	struct folio *folio;
	int bit_nr;
	int page_match;
};

struct wait_page_queue {
	struct folio *folio;
	int bit_nr;
	wait_queue_entry_t wait;
};

static inline bool wake_page_match(struct wait_page_queue *wait_page,
				  struct wait_page_key *key)
{
	if (wait_page->folio != key->folio)
	       return false;
	key->page_match = 1;

	if (wait_page->bit_nr != key->bit_nr)
		return false;

	return true;
}

void __folio_lock(struct folio *folio);
int __folio_lock_killable(struct folio *folio);
bool __folio_lock_or_retry(struct folio *folio, struct mm_struct *mm,
				unsigned int flags);
void unlock_page(struct page *page);
void folio_unlock(struct folio *folio);


static inline bool folio_trylock(struct folio *folio)
{
	return likely(!test_and_set_bit_lock(PG_locked, folio_flags(folio, 0)));
}


static inline int trylock_page(struct page *page)
{
	return folio_trylock(page_folio(page));
}


static inline void folio_lock(struct folio *folio)
{
	might_sleep();
	if (!folio_trylock(folio))
		__folio_lock(folio);
}


static inline void lock_page(struct page *page)
{
	struct folio *folio;
	might_sleep();

	folio = page_folio(page);
	if (!folio_trylock(folio))
		__folio_lock(folio);
}


static inline int folio_lock_killable(struct folio *folio)
{
	might_sleep();
	if (!folio_trylock(folio))
		return __folio_lock_killable(folio);
	return 0;
}


static inline int lock_page_killable(struct page *page)
{
	return folio_lock_killable(page_folio(page));
}


static inline bool lock_page_or_retry(struct page *page, struct mm_struct *mm,
				     unsigned int flags)
{
	struct folio *folio;
	might_sleep();

	folio = page_folio(page);
	return folio_trylock(folio) || __folio_lock_or_retry(folio, mm, flags);
}


void folio_wait_bit(struct folio *folio, int bit_nr);
int folio_wait_bit_killable(struct folio *folio, int bit_nr);


static inline void folio_wait_locked(struct folio *folio)
{
	if (folio_test_locked(folio))
		folio_wait_bit(folio, PG_locked);
}

static inline int folio_wait_locked_killable(struct folio *folio)
{
	if (!folio_test_locked(folio))
		return 0;
	return folio_wait_bit_killable(folio, PG_locked);
}

static inline void wait_on_page_locked(struct page *page)
{
	folio_wait_locked(page_folio(page));
}

static inline int wait_on_page_locked_killable(struct page *page)
{
	return folio_wait_locked_killable(page_folio(page));
}

int folio_put_wait_locked(struct folio *folio, int state);
void wait_on_page_writeback(struct page *page);
void folio_wait_writeback(struct folio *folio);
int folio_wait_writeback_killable(struct folio *folio);
void end_page_writeback(struct page *page);
void folio_end_writeback(struct folio *folio);
void wait_for_stable_page(struct page *page);
void folio_wait_stable(struct folio *folio);
void __folio_mark_dirty(struct folio *folio, struct address_space *, int warn);
static inline void __set_page_dirty(struct page *page,
		struct address_space *mapping, int warn)
{
	__folio_mark_dirty(page_folio(page), mapping, warn);
}
void folio_account_cleaned(struct folio *folio, struct bdi_writeback *wb);
void __folio_cancel_dirty(struct folio *folio);
static inline void folio_cancel_dirty(struct folio *folio)
{
	
	if (folio_test_dirty(folio))
		__folio_cancel_dirty(folio);
}
bool folio_clear_dirty_for_io(struct folio *folio);
bool clear_page_dirty_for_io(struct page *page);
void folio_invalidate(struct folio *folio, size_t offset, size_t length);
int __must_check folio_write_one(struct folio *folio);
static inline int __must_check write_one_page(struct page *page)
{
	return folio_write_one(page_folio(page));
}

int __set_page_dirty_nobuffers(struct page *page);
bool noop_dirty_folio(struct address_space *mapping, struct folio *folio);

#ifdef CONFIG_MIGRATION
int filemap_migrate_folio(struct address_space *mapping, struct folio *dst,
		struct folio *src, enum migrate_mode mode);
#else
#define filemap_migrate_folio NULL
#endif
void page_endio(struct page *page, bool is_write, int err);

void folio_end_private_2(struct folio *folio);
void folio_wait_private_2(struct folio *folio);
int folio_wait_private_2_killable(struct folio *folio);


void folio_add_wait_queue(struct folio *folio, wait_queue_entry_t *waiter);


size_t fault_in_writeable(char __user *uaddr, size_t size);
size_t fault_in_subpage_writeable(char __user *uaddr, size_t size);
size_t fault_in_safe_writeable(const char __user *uaddr, size_t size);
size_t fault_in_readable(const char __user *uaddr, size_t size);

int add_to_page_cache_lru(struct page *page, struct address_space *mapping,
		pgoff_t index, gfp_t gfp);
int filemap_add_folio(struct address_space *mapping, struct folio *folio,
		pgoff_t index, gfp_t gfp);
void filemap_remove_folio(struct folio *folio);
void delete_from_page_cache(struct page *page);
void __filemap_remove_folio(struct folio *folio, void *shadow);
void replace_page_cache_page(struct page *old, struct page *new);
void delete_from_page_cache_batch(struct address_space *mapping,
				  struct folio_batch *fbatch);
int try_to_release_page(struct page *page, gfp_t gfp);
bool filemap_release_folio(struct folio *folio, gfp_t gfp);
loff_t mapping_seek_hole_data(struct address_space *, loff_t start, loff_t end,
		int whence);


int __filemap_add_folio(struct address_space *mapping, struct folio *folio,
		pgoff_t index, gfp_t gfp, void **shadowp);

bool filemap_range_has_writeback(struct address_space *mapping,
				 loff_t start_byte, loff_t end_byte);


static inline bool filemap_range_needs_writeback(struct address_space *mapping,
						 loff_t start_byte,
						 loff_t end_byte)
{
	if (!mapping->nrpages)
		return false;
	if (!mapping_tagged(mapping, PAGECACHE_TAG_DIRTY) &&
	    !mapping_tagged(mapping, PAGECACHE_TAG_WRITEBACK))
		return false;
	return filemap_range_has_writeback(mapping, start_byte, end_byte);
}


struct readahead_control {
	struct file *file;
	struct address_space *mapping;
	struct file_ra_state *ra;

	pgoff_t _index;
	unsigned int _nr_pages;
	unsigned int _batch_count;
};

#define DEFINE_READAHEAD(ractl, f, r, m, i)				\
	struct readahead_control ractl = {				\
		.file = f,						\
		.mapping = m,						\
		.ra = r,						\
		._index = i,						\
	}

#define VM_READAHEAD_PAGES	(SZ_128K / PAGE_SIZE)

void page_cache_ra_unbounded(struct readahead_control *,
		unsigned long nr_to_read, unsigned long lookahead_count);
void page_cache_sync_ra(struct readahead_control *, unsigned long req_count);
void page_cache_async_ra(struct readahead_control *, struct folio *,
		unsigned long req_count);
void readahead_expand(struct readahead_control *ractl,
		      loff_t new_start, size_t new_len);


static inline
void page_cache_sync_readahead(struct address_space *mapping,
		struct file_ra_state *ra, struct file *file, pgoff_t index,
		unsigned long req_count)
{
	DEFINE_READAHEAD(ractl, file, ra, mapping, index);
	page_cache_sync_ra(&ractl, req_count);
}


static inline
void page_cache_async_readahead(struct address_space *mapping,
		struct file_ra_state *ra, struct file *file,
		struct folio *folio, pgoff_t index, unsigned long req_count)
{
	DEFINE_READAHEAD(ractl, file, ra, mapping, index);
	page_cache_async_ra(&ractl, folio, req_count);
}

static inline struct folio *__readahead_folio(struct readahead_control *ractl)
{
	struct folio *folio;

	BUG_ON(ractl->_batch_count > ractl->_nr_pages);
	ractl->_nr_pages -= ractl->_batch_count;
	ractl->_index += ractl->_batch_count;

	if (!ractl->_nr_pages) {
		ractl->_batch_count = 0;
		return NULL;
	}

	folio = xa_load(&ractl->mapping->i_pages, ractl->_index);
	VM_BUG_ON_FOLIO(!folio_test_locked(folio), folio);
	ractl->_batch_count = folio_nr_pages(folio);

	return folio;
}


static inline struct page *readahead_page(struct readahead_control *ractl)
{
	struct folio *folio = __readahead_folio(ractl);

	return &folio->page;
}


static inline struct folio *readahead_folio(struct readahead_control *ractl)
{
	struct folio *folio = __readahead_folio(ractl);

	if (folio)
		folio_put(folio);
	return folio;
}

static inline unsigned int __readahead_batch(struct readahead_control *rac,
		struct page **array, unsigned int array_sz)
{
	unsigned int i = 0;
	XA_STATE(xas, &rac->mapping->i_pages, 0);
	struct page *page;

	BUG_ON(rac->_batch_count > rac->_nr_pages);
	rac->_nr_pages -= rac->_batch_count;
	rac->_index += rac->_batch_count;
	rac->_batch_count = 0;

	xas_set(&xas, rac->_index);
	rcu_read_lock();
	xas_for_each(&xas, page, rac->_index + rac->_nr_pages - 1) {
		if (xas_retry(&xas, page))
			continue;
		VM_BUG_ON_PAGE(!PageLocked(page), page);
		VM_BUG_ON_PAGE(PageTail(page), page);
		array[i++] = page;
		rac->_batch_count += thp_nr_pages(page);
		if (i == array_sz)
			break;
	}
	rcu_read_unlock();

	return i;
}


#define readahead_page_batch(rac, array)				\
	__readahead_batch(rac, array, ARRAY_SIZE(array))


static inline loff_t readahead_pos(struct readahead_control *rac)
{
	return (loff_t)rac->_index * PAGE_SIZE;
}


static inline size_t readahead_length(struct readahead_control *rac)
{
	return rac->_nr_pages * PAGE_SIZE;
}


static inline pgoff_t readahead_index(struct readahead_control *rac)
{
	return rac->_index;
}


static inline unsigned int readahead_count(struct readahead_control *rac)
{
	return rac->_nr_pages;
}


static inline size_t readahead_batch_length(struct readahead_control *rac)
{
	return rac->_batch_count * PAGE_SIZE;
}

static inline unsigned long dir_pages(struct inode *inode)
{
	return (unsigned long)(inode->i_size + PAGE_SIZE - 1) >>
			       PAGE_SHIFT;
}


static inline ssize_t folio_mkwrite_check_truncate(struct folio *folio,
					      struct inode *inode)
{
	loff_t size = i_size_read(inode);
	pgoff_t index = size >> PAGE_SHIFT;
	size_t offset = offset_in_folio(folio, size);

	if (!folio->mapping)
		return -EFAULT;

	
	if (folio_next_index(folio) - 1 < index)
		return folio_size(folio);
	
	if (folio->index > index || !offset)
		return -EFAULT;
	
	return offset;
}


static inline int page_mkwrite_check_truncate(struct page *page,
					      struct inode *inode)
{
	loff_t size = i_size_read(inode);
	pgoff_t index = size >> PAGE_SHIFT;
	int offset = offset_in_page(size);

	if (page->mapping != inode->i_mapping)
		return -EFAULT;

	
	if (page->index < index)
		return PAGE_SIZE;
	
	if (page->index > index || !offset)
		return -EFAULT;
	
	return offset;
}


static inline
unsigned int i_blocks_per_folio(struct inode *inode, struct folio *folio)
{
	return folio_size(folio) >> inode->i_blkbits;
}

static inline
unsigned int i_blocks_per_page(struct inode *inode, struct page *page)
{
	return i_blocks_per_folio(inode, page_folio(page));
}
#endif 
