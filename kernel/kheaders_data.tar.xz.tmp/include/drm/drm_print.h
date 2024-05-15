

#ifndef DRM_PRINT_H_
#define DRM_PRINT_H_

#include <linux/compiler.h>
#include <linux/printk.h>
#include <linux/seq_file.h>
#include <linux/device.h>
#include <linux/debugfs.h>

#include <drm/drm.h>


extern unsigned int __drm_debug;




struct drm_printer {
	
	void (*printfn)(struct drm_printer *p, struct va_format *vaf);
	void (*puts)(struct drm_printer *p, const char *str);
	void *arg;
	const char *prefix;
};

void __drm_printfn_coredump(struct drm_printer *p, struct va_format *vaf);
void __drm_puts_coredump(struct drm_printer *p, const char *str);
void __drm_printfn_seq_file(struct drm_printer *p, struct va_format *vaf);
void __drm_puts_seq_file(struct drm_printer *p, const char *str);
void __drm_printfn_info(struct drm_printer *p, struct va_format *vaf);
void __drm_printfn_debug(struct drm_printer *p, struct va_format *vaf);
void __drm_printfn_err(struct drm_printer *p, struct va_format *vaf);

__printf(2, 3)
void drm_printf(struct drm_printer *p, const char *f, ...);
void drm_puts(struct drm_printer *p, const char *str);
void drm_print_regset32(struct drm_printer *p, struct debugfs_regset32 *regset);
void drm_print_bits(struct drm_printer *p, unsigned long value,
		    const char * const bits[], unsigned int nbits);

__printf(2, 0)

static inline void
drm_vprintf(struct drm_printer *p, const char *fmt, va_list *va)
{
	struct va_format vaf = { .fmt = fmt, .va = va };

	p->printfn(p, &vaf);
}


#define drm_printf_indent(printer, indent, fmt, ...) \
	drm_printf((printer), "%.*s" fmt, (indent), "\t\t\t\t\tX", ##__VA_ARGS__)


struct drm_print_iterator {
	void *data;
	ssize_t start;
	ssize_t remain;
	
	ssize_t offset;
};


static inline struct drm_printer
drm_coredump_printer(struct drm_print_iterator *iter)
{
	struct drm_printer p = {
		.printfn = __drm_printfn_coredump,
		.puts = __drm_puts_coredump,
		.arg = iter,
	};

	
	iter->offset = 0;

	return p;
}


static inline struct drm_printer drm_seq_file_printer(struct seq_file *f)
{
	struct drm_printer p = {
		.printfn = __drm_printfn_seq_file,
		.puts = __drm_puts_seq_file,
		.arg = f,
	};
	return p;
}


static inline struct drm_printer drm_info_printer(struct device *dev)
{
	struct drm_printer p = {
		.printfn = __drm_printfn_info,
		.arg = dev,
	};
	return p;
}


static inline struct drm_printer drm_debug_printer(const char *prefix)
{
	struct drm_printer p = {
		.printfn = __drm_printfn_debug,
		.prefix = prefix
	};
	return p;
}


static inline struct drm_printer drm_err_printer(const char *prefix)
{
	struct drm_printer p = {
		.printfn = __drm_printfn_err,
		.prefix = prefix
	};
	return p;
}


enum drm_debug_category {
	
	DRM_UT_CORE		= 0x01,
	
	DRM_UT_DRIVER		= 0x02,
	
	DRM_UT_KMS		= 0x04,
	
	DRM_UT_PRIME		= 0x08,
	
	DRM_UT_ATOMIC		= 0x10,
	
	DRM_UT_VBL		= 0x20,
	
	DRM_UT_STATE		= 0x40,
	
	DRM_UT_LEASE		= 0x80,
	
	DRM_UT_DP		= 0x100,
	
	DRM_UT_DRMRES		= 0x200,
};

static inline bool drm_debug_enabled(enum drm_debug_category category)
{
	return unlikely(__drm_debug & category);
}



__printf(3, 4)
void drm_dev_printk(const struct device *dev, const char *level,
		    const char *format, ...);
__printf(3, 4)
void drm_dev_dbg(const struct device *dev, enum drm_debug_category category,
		 const char *format, ...);


#define DRM_DEV_ERROR(dev, fmt, ...)					\
	drm_dev_printk(dev, KERN_ERR, "*ERROR* " fmt, ##__VA_ARGS__)


#define DRM_DEV_ERROR_RATELIMITED(dev, fmt, ...)			\
({									\
	static DEFINE_RATELIMIT_STATE(_rs,				\
				      DEFAULT_RATELIMIT_INTERVAL,	\
				      DEFAULT_RATELIMIT_BURST);		\
									\
	if (__ratelimit(&_rs))						\
		DRM_DEV_ERROR(dev, fmt, ##__VA_ARGS__);			\
})


#define DRM_DEV_INFO(dev, fmt, ...)				\
	drm_dev_printk(dev, KERN_INFO, fmt, ##__VA_ARGS__)


#define DRM_DEV_INFO_ONCE(dev, fmt, ...)				\
({									\
	static bool __print_once __read_mostly;				\
	if (!__print_once) {						\
		__print_once = true;					\
		DRM_DEV_INFO(dev, fmt, ##__VA_ARGS__);			\
	}								\
})


#define DRM_DEV_DEBUG(dev, fmt, ...)					\
	drm_dev_dbg(dev, DRM_UT_CORE, fmt, ##__VA_ARGS__)

#define DRM_DEV_DEBUG_DRIVER(dev, fmt, ...)				\
	drm_dev_dbg(dev, DRM_UT_DRIVER,	fmt, ##__VA_ARGS__)

#define DRM_DEV_DEBUG_KMS(dev, fmt, ...)				\
	drm_dev_dbg(dev, DRM_UT_KMS, fmt, ##__VA_ARGS__)




#define __drm_printk(drm, level, type, fmt, ...)			\
	dev_##level##type((drm)->dev, "[drm] " fmt, ##__VA_ARGS__)


#define drm_info(drm, fmt, ...)					\
	__drm_printk((drm), info,, fmt, ##__VA_ARGS__)

#define drm_notice(drm, fmt, ...)				\
	__drm_printk((drm), notice,, fmt, ##__VA_ARGS__)

#define drm_warn(drm, fmt, ...)					\
	__drm_printk((drm), warn,, fmt, ##__VA_ARGS__)

#define drm_err(drm, fmt, ...)					\
	__drm_printk((drm), err,, "*ERROR* " fmt, ##__VA_ARGS__)


#define drm_info_once(drm, fmt, ...)				\
	__drm_printk((drm), info, _once, fmt, ##__VA_ARGS__)

#define drm_notice_once(drm, fmt, ...)				\
	__drm_printk((drm), notice, _once, fmt, ##__VA_ARGS__)

#define drm_warn_once(drm, fmt, ...)				\
	__drm_printk((drm), warn, _once, fmt, ##__VA_ARGS__)

#define drm_err_once(drm, fmt, ...)				\
	__drm_printk((drm), err, _once, "*ERROR* " fmt, ##__VA_ARGS__)


#define drm_err_ratelimited(drm, fmt, ...)				\
	__drm_printk((drm), err, _ratelimited, "*ERROR* " fmt, ##__VA_ARGS__)


#define drm_dbg_core(drm, fmt, ...)					\
	drm_dev_dbg((drm) ? (drm)->dev : NULL, DRM_UT_CORE, fmt, ##__VA_ARGS__)
#define drm_dbg(drm, fmt, ...)						\
	drm_dev_dbg((drm) ? (drm)->dev : NULL, DRM_UT_DRIVER, fmt, ##__VA_ARGS__)
#define drm_dbg_kms(drm, fmt, ...)					\
	drm_dev_dbg((drm) ? (drm)->dev : NULL, DRM_UT_KMS, fmt, ##__VA_ARGS__)
#define drm_dbg_prime(drm, fmt, ...)					\
	drm_dev_dbg((drm) ? (drm)->dev : NULL, DRM_UT_PRIME, fmt, ##__VA_ARGS__)
#define drm_dbg_atomic(drm, fmt, ...)					\
	drm_dev_dbg((drm) ? (drm)->dev : NULL, DRM_UT_ATOMIC, fmt, ##__VA_ARGS__)
#define drm_dbg_vbl(drm, fmt, ...)					\
	drm_dev_dbg((drm) ? (drm)->dev : NULL, DRM_UT_VBL, fmt, ##__VA_ARGS__)
#define drm_dbg_state(drm, fmt, ...)					\
	drm_dev_dbg((drm) ? (drm)->dev : NULL, DRM_UT_STATE, fmt, ##__VA_ARGS__)
#define drm_dbg_lease(drm, fmt, ...)					\
	drm_dev_dbg((drm) ? (drm)->dev : NULL, DRM_UT_LEASE, fmt, ##__VA_ARGS__)
#define drm_dbg_dp(drm, fmt, ...)					\
	drm_dev_dbg((drm) ? (drm)->dev : NULL, DRM_UT_DP, fmt, ##__VA_ARGS__)
#define drm_dbg_drmres(drm, fmt, ...)					\
	drm_dev_dbg((drm) ? (drm)->dev : NULL, DRM_UT_DRMRES, fmt, ##__VA_ARGS__)




__printf(2, 3)
void __drm_dbg(enum drm_debug_category category, const char *format, ...);
__printf(1, 2)
void __drm_err(const char *format, ...);



#define _DRM_PRINTK(once, level, fmt, ...)				\
	printk##once(KERN_##level "[" DRM_NAME "] " fmt, ##__VA_ARGS__)


#define DRM_INFO(fmt, ...)						\
	_DRM_PRINTK(, INFO, fmt, ##__VA_ARGS__)

#define DRM_NOTE(fmt, ...)						\
	_DRM_PRINTK(, NOTICE, fmt, ##__VA_ARGS__)

#define DRM_WARN(fmt, ...)						\
	_DRM_PRINTK(, WARNING, fmt, ##__VA_ARGS__)


#define DRM_INFO_ONCE(fmt, ...)						\
	_DRM_PRINTK(_once, INFO, fmt, ##__VA_ARGS__)

#define DRM_NOTE_ONCE(fmt, ...)						\
	_DRM_PRINTK(_once, NOTICE, fmt, ##__VA_ARGS__)

#define DRM_WARN_ONCE(fmt, ...)						\
	_DRM_PRINTK(_once, WARNING, fmt, ##__VA_ARGS__)


#define DRM_ERROR(fmt, ...)						\
	__drm_err(fmt, ##__VA_ARGS__)


#define DRM_ERROR_RATELIMITED(fmt, ...)					\
	DRM_DEV_ERROR_RATELIMITED(NULL, fmt, ##__VA_ARGS__)


#define DRM_DEBUG(fmt, ...)						\
	__drm_dbg(DRM_UT_CORE, fmt, ##__VA_ARGS__)


#define DRM_DEBUG_DRIVER(fmt, ...)					\
	__drm_dbg(DRM_UT_DRIVER, fmt, ##__VA_ARGS__)


#define DRM_DEBUG_KMS(fmt, ...)						\
	__drm_dbg(DRM_UT_KMS, fmt, ##__VA_ARGS__)


#define DRM_DEBUG_PRIME(fmt, ...)					\
	__drm_dbg(DRM_UT_PRIME, fmt, ##__VA_ARGS__)


#define DRM_DEBUG_ATOMIC(fmt, ...)					\
	__drm_dbg(DRM_UT_ATOMIC, fmt, ##__VA_ARGS__)


#define DRM_DEBUG_VBL(fmt, ...)						\
	__drm_dbg(DRM_UT_VBL, fmt, ##__VA_ARGS__)


#define DRM_DEBUG_LEASE(fmt, ...)					\
	__drm_dbg(DRM_UT_LEASE, fmt, ##__VA_ARGS__)


#define DRM_DEBUG_DP(fmt, ...)						\
	__drm_dbg(DRM_UT_DP, fmt, ## __VA_ARGS__)

#define __DRM_DEFINE_DBG_RATELIMITED(category, drm, fmt, ...)					\
({												\
	static DEFINE_RATELIMIT_STATE(rs_, DEFAULT_RATELIMIT_INTERVAL, DEFAULT_RATELIMIT_BURST);\
	const struct drm_device *drm_ = (drm);							\
												\
	if (drm_debug_enabled(DRM_UT_ ## category) && __ratelimit(&rs_))			\
		drm_dev_printk(drm_ ? drm_->dev : NULL, KERN_DEBUG, fmt, ## __VA_ARGS__);	\
})

#define drm_dbg_kms_ratelimited(drm, fmt, ...) \
	__DRM_DEFINE_DBG_RATELIMITED(KMS, drm, fmt, ## __VA_ARGS__)


#define DRM_DEBUG_KMS_RATELIMITED(fmt, ...) drm_dbg_kms_ratelimited(NULL, fmt, ## __VA_ARGS__)




#define drm_WARN(drm, condition, format, arg...)			\
	WARN(condition, "%s %s: " format,				\
			dev_driver_string((drm)->dev),			\
			dev_name((drm)->dev), ## arg)

#define drm_WARN_ONCE(drm, condition, format, arg...)			\
	WARN_ONCE(condition, "%s %s: " format,				\
			dev_driver_string((drm)->dev),			\
			dev_name((drm)->dev), ## arg)

#define drm_WARN_ON(drm, x)						\
	drm_WARN((drm), (x), "%s",					\
		 "drm_WARN_ON(" __stringify(x) ")")

#define drm_WARN_ON_ONCE(drm, x)					\
	drm_WARN_ONCE((drm), (x), "%s",					\
		      "drm_WARN_ON_ONCE(" __stringify(x) ")")

#endif 
