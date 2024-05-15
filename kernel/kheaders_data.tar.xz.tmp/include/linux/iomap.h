/* SPDX-License-Identifier: GPL-2.0 */
#ifndef LINUX_IOMAP_H
#define LINUX_IOMAP_H 1

#include <linux/atomic.h>
#include <linux/bitmap.h>
#include <linux/blk_types.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/mm_types.h>
#include <linux/blkdev.h>

struct address_space;
struct fiemap_extent_info;
struct inode;
struct iomap_dio;
struct iomap_writepage_ctx;
struct iov_iter;
struct kiocb;
struct page;
struct vm_area_struct;
struct vm_fault;


#define IOMAP_HOLE	0	
#define IOMAP_DELALLOC	1	
#define IOMAP_MAPPED	2	
#define IOMAP_UNWRITTEN	3	
#define IOMAP_INLINE	4	


#define IOMAP_F_NEW		0x01
#define IOMAP_F_DIRTY		0x02
#define IOMAP_F_SHARED		0x04
#define IOMAP_F_MERGED		0x08
#define IOMAP_F_BUFFER_HEAD	0x10
#define IOMAP_F_ZONE_APPEND	0x20


#define IOMAP_F_SIZE_CHANGED	0x100


#define IOMAP_F_PRIVATE		0x1000



#define IOMAP_NULL_ADDR -1ULL	

struct iomap_page_ops;

struct iomap {
	u64			addr; 
	loff_t			offset;	
	u64			length;	
	u16			type;	
	u16			flags;	
	struct block_device	*bdev;	
	struct dax_device	*dax_dev; 
	void			*inline_data;
	void			*private; 
	const struct iomap_page_ops *page_ops;
};

static inline sector_t iomap_sector(const struct iomap *iomap, loff_t pos)
{
	return (iomap->addr + pos - iomap->offset) >> SECTOR_SHIFT;
}


static inline void *iomap_inline_data(const struct iomap *iomap, loff_t pos)
{
	return iomap->inline_data + pos - iomap->offset;
}


static inline bool iomap_inline_data_valid(const struct iomap *iomap)
{
	return iomap->length <= PAGE_SIZE - offset_in_page(iomap->inline_data);
}


struct iomap_page_ops {
	int (*page_prepare)(struct inode *inode, loff_t pos, unsigned len);
	void (*page_done)(struct inode *inode, loff_t pos, unsigned copied,
			struct page *page);
};


#define IOMAP_WRITE		(1 << 0) 
#define IOMAP_ZERO		(1 << 1) 
#define IOMAP_REPORT		(1 << 2) 
#define IOMAP_FAULT		(1 << 3) 
#define IOMAP_DIRECT		(1 << 4) 
#define IOMAP_NOWAIT		(1 << 5) 
#define IOMAP_OVERWRITE_ONLY	(1 << 6) 
#define IOMAP_UNSHARE		(1 << 7) 
#ifdef CONFIG_FS_DAX
#define IOMAP_DAX		(1 << 8) 
#else
#define IOMAP_DAX		0
#endif 

struct iomap_ops {
	
	int (*iomap_begin)(struct inode *inode, loff_t pos, loff_t length,
			unsigned flags, struct iomap *iomap,
			struct iomap *srcmap);

	
	int (*iomap_end)(struct inode *inode, loff_t pos, loff_t length,
			ssize_t written, unsigned flags, struct iomap *iomap);
};


struct iomap_iter {
	struct inode *inode;
	loff_t pos;
	u64 len;
	s64 processed;
	unsigned flags;
	struct iomap iomap;
	struct iomap srcmap;
	void *private;
};

int iomap_iter(struct iomap_iter *iter, const struct iomap_ops *ops);


static inline u64 iomap_length(const struct iomap_iter *iter)
{
	u64 end = iter->iomap.offset + iter->iomap.length;

	if (iter->srcmap.type != IOMAP_HOLE)
		end = min(end, iter->srcmap.offset + iter->srcmap.length);
	return min(iter->len, end - iter->pos);
}


static inline const struct iomap *iomap_iter_srcmap(const struct iomap_iter *i)
{
	if (i->srcmap.type != IOMAP_HOLE)
		return &i->srcmap;
	return &i->iomap;
}

ssize_t iomap_file_buffered_write(struct kiocb *iocb, struct iov_iter *from,
		const struct iomap_ops *ops);
int iomap_read_folio(struct folio *folio, const struct iomap_ops *ops);
void iomap_readahead(struct readahead_control *, const struct iomap_ops *ops);
bool iomap_is_partially_uptodate(struct folio *, size_t from, size_t count);
bool iomap_release_folio(struct folio *folio, gfp_t gfp_flags);
void iomap_invalidate_folio(struct folio *folio, size_t offset, size_t len);
int iomap_file_unshare(struct inode *inode, loff_t pos, loff_t len,
		const struct iomap_ops *ops);
int iomap_zero_range(struct inode *inode, loff_t pos, loff_t len,
		bool *did_zero, const struct iomap_ops *ops);
int iomap_truncate_page(struct inode *inode, loff_t pos, bool *did_zero,
		const struct iomap_ops *ops);
vm_fault_t iomap_page_mkwrite(struct vm_fault *vmf,
			const struct iomap_ops *ops);
int iomap_fiemap(struct inode *inode, struct fiemap_extent_info *fieinfo,
		u64 start, u64 len, const struct iomap_ops *ops);
loff_t iomap_seek_hole(struct inode *inode, loff_t offset,
		const struct iomap_ops *ops);
loff_t iomap_seek_data(struct inode *inode, loff_t offset,
		const struct iomap_ops *ops);
sector_t iomap_bmap(struct address_space *mapping, sector_t bno,
		const struct iomap_ops *ops);


struct iomap_ioend {
	struct list_head	io_list;	
	u16			io_type;
	u16			io_flags;	
	u32			io_folios;	
	struct inode		*io_inode;	
	size_t			io_size;	
	loff_t			io_offset;	
	sector_t		io_sector;	
	struct bio		*io_bio;	
	struct bio		io_inline_bio;	
};

struct iomap_writeback_ops {
	
	int (*map_blocks)(struct iomap_writepage_ctx *wpc, struct inode *inode,
				loff_t offset);

	
	int (*prepare_ioend)(struct iomap_ioend *ioend, int status);

	
	void (*discard_folio)(struct folio *folio, loff_t pos);
};

struct iomap_writepage_ctx {
	struct iomap		iomap;
	struct iomap_ioend	*ioend;
	const struct iomap_writeback_ops *ops;
};

void iomap_finish_ioends(struct iomap_ioend *ioend, int error);
void iomap_ioend_try_merge(struct iomap_ioend *ioend,
		struct list_head *more_ioends);
void iomap_sort_ioends(struct list_head *ioend_list);
int iomap_writepages(struct address_space *mapping,
		struct writeback_control *wbc, struct iomap_writepage_ctx *wpc,
		const struct iomap_writeback_ops *ops);


#define IOMAP_DIO_UNWRITTEN	(1 << 0)	
#define IOMAP_DIO_COW		(1 << 1)	

struct iomap_dio_ops {
	int (*end_io)(struct kiocb *iocb, ssize_t size, int error,
		      unsigned flags);
	void (*submit_io)(const struct iomap_iter *iter, struct bio *bio,
		          loff_t file_offset);

	
	struct bio_set *bio_set;
};


#define IOMAP_DIO_FORCE_WAIT	(1 << 0)


#define IOMAP_DIO_OVERWRITE_ONLY	(1 << 1)


#define IOMAP_DIO_PARTIAL		(1 << 2)


#define IOMAP_DIO_NOSYNC		(1 << 3)

ssize_t iomap_dio_rw(struct kiocb *iocb, struct iov_iter *iter,
		const struct iomap_ops *ops, const struct iomap_dio_ops *dops,
		unsigned int dio_flags, void *private, size_t done_before);
struct iomap_dio *__iomap_dio_rw(struct kiocb *iocb, struct iov_iter *iter,
		const struct iomap_ops *ops, const struct iomap_dio_ops *dops,
		unsigned int dio_flags, void *private, size_t done_before);
ssize_t iomap_dio_complete(struct iomap_dio *dio);
void iomap_dio_bio_end_io(struct bio *bio);

#ifdef CONFIG_SWAP
struct file;
struct swap_info_struct;

int iomap_swapfile_activate(struct swap_info_struct *sis,
		struct file *swap_file, sector_t *pagespan,
		const struct iomap_ops *ops);
#else
# define iomap_swapfile_activate(sis, swapfile, pagespan, ops)	(-EIO)
#endif 

#endif 
