

#ifndef _DRM_FILE_H_
#define _DRM_FILE_H_

#include <linux/types.h>
#include <linux/completion.h>
#include <linux/idr.h>

#include <uapi/drm/drm.h>

#include <drm/drm_prime.h>

struct dma_fence;
struct drm_file;
struct drm_device;
struct device;
struct file;




enum drm_minor_type {
	DRM_MINOR_PRIMARY,
	DRM_MINOR_CONTROL,
	DRM_MINOR_RENDER,
};


struct drm_minor {
	
	int index;			
	int type;                       
	struct device *kdev;		
	struct drm_device *dev;

	struct dentry *debugfs_root;

	struct list_head debugfs_list;
	struct mutex debugfs_lock; 
};


struct drm_pending_event {
	
	struct completion *completion;

	
	void (*completion_release)(struct completion *completion);

	
	struct drm_event *event;

	
	struct dma_fence *fence;

	
	struct drm_file *file_priv;

	
	struct list_head link;

	
	struct list_head pending_link;
};


struct drm_file {
	
	bool authenticated;

	
	bool stereo_allowed;

	
	bool universal_planes;

	
	bool atomic;

	
	bool aspect_ratio_allowed;

	
	bool writeback_connectors;

	
	bool was_master;

	
	bool is_master;

	
	struct drm_master *master;

	
	spinlock_t master_lookup_lock;

	
	struct pid *pid;

	
	drm_magic_t magic;

	
	struct list_head lhead;

	
	struct drm_minor *minor;

	
	struct idr object_idr;

	
	spinlock_t table_lock;

	
	struct idr syncobj_idr;
	
	spinlock_t syncobj_table_lock;

	
	struct file *filp;

	
	void *driver_priv;

	
	struct list_head fbs;

	
	struct mutex fbs_lock;

	
	struct list_head blobs;

	
	wait_queue_head_t event_wait;

	
	struct list_head pending_event_list;

	
	struct list_head event_list;

	
	int event_space;

	
	struct mutex event_read_lock;

	
	struct drm_prime_file_private prime;

	
#if IS_ENABLED(CONFIG_DRM_LEGACY)
	unsigned long lock_count; 
#endif
};


static inline bool drm_is_primary_client(const struct drm_file *file_priv)
{
	return file_priv->minor->type == DRM_MINOR_PRIMARY;
}


static inline bool drm_is_render_client(const struct drm_file *file_priv)
{
	return file_priv->minor->type == DRM_MINOR_RENDER;
}

int drm_open(struct inode *inode, struct file *filp);
ssize_t drm_read(struct file *filp, char __user *buffer,
		 size_t count, loff_t *offset);
int drm_release(struct inode *inode, struct file *filp);
int drm_release_noglobal(struct inode *inode, struct file *filp);
__poll_t drm_poll(struct file *filp, struct poll_table_struct *wait);
int drm_event_reserve_init_locked(struct drm_device *dev,
				  struct drm_file *file_priv,
				  struct drm_pending_event *p,
				  struct drm_event *e);
int drm_event_reserve_init(struct drm_device *dev,
			   struct drm_file *file_priv,
			   struct drm_pending_event *p,
			   struct drm_event *e);
void drm_event_cancel_free(struct drm_device *dev,
			   struct drm_pending_event *p);
void drm_send_event_locked(struct drm_device *dev, struct drm_pending_event *e);
void drm_send_event(struct drm_device *dev, struct drm_pending_event *e);
void drm_send_event_timestamp_locked(struct drm_device *dev,
				     struct drm_pending_event *e,
				     ktime_t timestamp);

struct file *mock_drm_getfile(struct drm_minor *minor, unsigned int flags);

#ifdef CONFIG_MMU
struct drm_vma_offset_manager;
unsigned long drm_get_unmapped_area(struct file *file,
				    unsigned long uaddr, unsigned long len,
				    unsigned long pgoff, unsigned long flags,
				    struct drm_vma_offset_manager *mgr);
#endif 


#endif 
