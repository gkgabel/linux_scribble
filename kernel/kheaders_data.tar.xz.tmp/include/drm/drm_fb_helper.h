
#ifndef DRM_FB_HELPER_H
#define DRM_FB_HELPER_H

struct drm_fb_helper;

#include <drm/drm_client.h>
#include <drm/drm_crtc.h>
#include <drm/drm_device.h>
#include <linux/fb.h>
#include <linux/kgdb.h>

enum mode_set_atomic {
	LEAVE_ATOMIC_MODE_SET,
	ENTER_ATOMIC_MODE_SET,
};


struct drm_fb_helper_surface_size {
	u32 fb_width;
	u32 fb_height;
	u32 surface_width;
	u32 surface_height;
	u32 surface_bpp;
	u32 surface_depth;
};


struct drm_fb_helper_funcs {
	
	int (*fb_probe)(struct drm_fb_helper *helper,
			struct drm_fb_helper_surface_size *sizes);
};


struct drm_fb_helper {
	
	struct drm_client_dev client;

	
	struct drm_client_buffer *buffer;

	struct drm_framebuffer *fb;
	struct drm_device *dev;
	const struct drm_fb_helper_funcs *funcs;
	struct fb_info *fbdev;
	u32 pseudo_palette[17];
	struct drm_clip_rect damage_clip;
	spinlock_t damage_lock;
	struct work_struct damage_work;
	struct work_struct resume_work;

	
	struct mutex lock;

	
	struct list_head kernel_fb_list;

	
	bool delayed_hotplug;

	
	bool deferred_setup;

	
	int preferred_bpp;
};

static inline struct drm_fb_helper *
drm_fb_helper_from_client(struct drm_client_dev *client)
{
	return container_of(client, struct drm_fb_helper, client);
}


#define DRM_FB_HELPER_DEFAULT_OPS \
	.fb_check_var	= drm_fb_helper_check_var, \
	.fb_set_par	= drm_fb_helper_set_par, \
	.fb_setcmap	= drm_fb_helper_setcmap, \
	.fb_blank	= drm_fb_helper_blank, \
	.fb_pan_display	= drm_fb_helper_pan_display, \
	.fb_debug_enter = drm_fb_helper_debug_enter, \
	.fb_debug_leave = drm_fb_helper_debug_leave, \
	.fb_ioctl	= drm_fb_helper_ioctl

#ifdef CONFIG_DRM_FBDEV_EMULATION
void drm_fb_helper_prepare(struct drm_device *dev, struct drm_fb_helper *helper,
			   const struct drm_fb_helper_funcs *funcs);
int drm_fb_helper_init(struct drm_device *dev, struct drm_fb_helper *helper);
void drm_fb_helper_fini(struct drm_fb_helper *helper);
int drm_fb_helper_blank(int blank, struct fb_info *info);
int drm_fb_helper_pan_display(struct fb_var_screeninfo *var,
			      struct fb_info *info);
int drm_fb_helper_set_par(struct fb_info *info);
int drm_fb_helper_check_var(struct fb_var_screeninfo *var,
			    struct fb_info *info);

int drm_fb_helper_restore_fbdev_mode_unlocked(struct drm_fb_helper *fb_helper);

struct fb_info *drm_fb_helper_alloc_fbi(struct drm_fb_helper *fb_helper);
void drm_fb_helper_unregister_fbi(struct drm_fb_helper *fb_helper);
void drm_fb_helper_fill_info(struct fb_info *info,
			     struct drm_fb_helper *fb_helper,
			     struct drm_fb_helper_surface_size *sizes);

void drm_fb_helper_deferred_io(struct fb_info *info, struct list_head *pagereflist);

ssize_t drm_fb_helper_sys_read(struct fb_info *info, char __user *buf,
			       size_t count, loff_t *ppos);
ssize_t drm_fb_helper_sys_write(struct fb_info *info, const char __user *buf,
				size_t count, loff_t *ppos);

void drm_fb_helper_sys_fillrect(struct fb_info *info,
				const struct fb_fillrect *rect);
void drm_fb_helper_sys_copyarea(struct fb_info *info,
				const struct fb_copyarea *area);
void drm_fb_helper_sys_imageblit(struct fb_info *info,
				 const struct fb_image *image);

void drm_fb_helper_cfb_fillrect(struct fb_info *info,
				const struct fb_fillrect *rect);
void drm_fb_helper_cfb_copyarea(struct fb_info *info,
				const struct fb_copyarea *area);
void drm_fb_helper_cfb_imageblit(struct fb_info *info,
				 const struct fb_image *image);

void drm_fb_helper_set_suspend(struct drm_fb_helper *fb_helper, bool suspend);
void drm_fb_helper_set_suspend_unlocked(struct drm_fb_helper *fb_helper,
					bool suspend);

int drm_fb_helper_setcmap(struct fb_cmap *cmap, struct fb_info *info);

int drm_fb_helper_ioctl(struct fb_info *info, unsigned int cmd,
			unsigned long arg);

int drm_fb_helper_hotplug_event(struct drm_fb_helper *fb_helper);
int drm_fb_helper_initial_config(struct drm_fb_helper *fb_helper, int bpp_sel);
int drm_fb_helper_debug_enter(struct fb_info *info);
int drm_fb_helper_debug_leave(struct fb_info *info);

void drm_fb_helper_lastclose(struct drm_device *dev);
void drm_fb_helper_output_poll_changed(struct drm_device *dev);

void drm_fbdev_generic_setup(struct drm_device *dev,
			     unsigned int preferred_bpp);
#else
static inline void drm_fb_helper_prepare(struct drm_device *dev,
					struct drm_fb_helper *helper,
					const struct drm_fb_helper_funcs *funcs)
{
}

static inline int drm_fb_helper_init(struct drm_device *dev,
		       struct drm_fb_helper *helper)
{
	
	helper->dev = dev;
	dev->fb_helper = helper;

	return 0;
}

static inline void drm_fb_helper_fini(struct drm_fb_helper *helper)
{
	if (helper && helper->dev)
		helper->dev->fb_helper = NULL;
}

static inline int drm_fb_helper_blank(int blank, struct fb_info *info)
{
	return 0;
}

static inline int drm_fb_helper_pan_display(struct fb_var_screeninfo *var,
					    struct fb_info *info)
{
	return 0;
}

static inline int drm_fb_helper_set_par(struct fb_info *info)
{
	return 0;
}

static inline int drm_fb_helper_check_var(struct fb_var_screeninfo *var,
					  struct fb_info *info)
{
	return 0;
}

static inline int
drm_fb_helper_restore_fbdev_mode_unlocked(struct drm_fb_helper *fb_helper)
{
	return 0;
}

static inline struct fb_info *
drm_fb_helper_alloc_fbi(struct drm_fb_helper *fb_helper)
{
	return NULL;
}

static inline void drm_fb_helper_unregister_fbi(struct drm_fb_helper *fb_helper)
{
}

static inline void
drm_fb_helper_fill_info(struct fb_info *info,
			struct drm_fb_helper *fb_helper,
			struct drm_fb_helper_surface_size *sizes)
{
}

static inline int drm_fb_helper_setcmap(struct fb_cmap *cmap,
					struct fb_info *info)
{
	return 0;
}

static inline int drm_fb_helper_ioctl(struct fb_info *info, unsigned int cmd,
				      unsigned long arg)
{
	return 0;
}

static inline void drm_fb_helper_deferred_io(struct fb_info *info,
					     struct list_head *pagelist)
{
}

static inline int drm_fb_helper_defio_init(struct drm_fb_helper *fb_helper)
{
	return -ENODEV;
}

static inline ssize_t drm_fb_helper_sys_read(struct fb_info *info,
					     char __user *buf, size_t count,
					     loff_t *ppos)
{
	return -ENODEV;
}

static inline ssize_t drm_fb_helper_sys_write(struct fb_info *info,
					      const char __user *buf,
					      size_t count, loff_t *ppos)
{
	return -ENODEV;
}

static inline void drm_fb_helper_sys_fillrect(struct fb_info *info,
					      const struct fb_fillrect *rect)
{
}

static inline void drm_fb_helper_sys_copyarea(struct fb_info *info,
					      const struct fb_copyarea *area)
{
}

static inline void drm_fb_helper_sys_imageblit(struct fb_info *info,
					       const struct fb_image *image)
{
}

static inline void drm_fb_helper_cfb_fillrect(struct fb_info *info,
					      const struct fb_fillrect *rect)
{
}

static inline void drm_fb_helper_cfb_copyarea(struct fb_info *info,
					      const struct fb_copyarea *area)
{
}

static inline void drm_fb_helper_cfb_imageblit(struct fb_info *info,
					       const struct fb_image *image)
{
}

static inline void drm_fb_helper_set_suspend(struct drm_fb_helper *fb_helper,
					     bool suspend)
{
}

static inline void
drm_fb_helper_set_suspend_unlocked(struct drm_fb_helper *fb_helper, bool suspend)
{
}

static inline int drm_fb_helper_hotplug_event(struct drm_fb_helper *fb_helper)
{
	return 0;
}

static inline int drm_fb_helper_initial_config(struct drm_fb_helper *fb_helper,
					       int bpp_sel)
{
	return 0;
}

static inline int drm_fb_helper_debug_enter(struct fb_info *info)
{
	return 0;
}

static inline int drm_fb_helper_debug_leave(struct fb_info *info)
{
	return 0;
}

static inline void drm_fb_helper_lastclose(struct drm_device *dev)
{
}

static inline void drm_fb_helper_output_poll_changed(struct drm_device *dev)
{
}

static inline void
drm_fbdev_generic_setup(struct drm_device *dev, unsigned int preferred_bpp)
{
}

#endif

#endif
