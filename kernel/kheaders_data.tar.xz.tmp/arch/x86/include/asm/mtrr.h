
#ifndef _ASM_X86_MTRR_H
#define _ASM_X86_MTRR_H

#include <uapi/asm/mtrr.h>

void mtrr_bp_init(void);


# ifdef CONFIG_MTRR
extern u8 mtrr_type_lookup(u64 addr, u64 end, u8 *uniform);
extern void mtrr_save_fixed_ranges(void *);
extern void mtrr_save_state(void);
extern int mtrr_add(unsigned long base, unsigned long size,
		    unsigned int type, bool increment);
extern int mtrr_add_page(unsigned long base, unsigned long size,
			 unsigned int type, bool increment);
extern int mtrr_del(int reg, unsigned long base, unsigned long size);
extern int mtrr_del_page(int reg, unsigned long base, unsigned long size);
extern void mtrr_centaur_report_mcr(int mcr, u32 lo, u32 hi);
extern void mtrr_ap_init(void);
extern void set_mtrr_aps_delayed_init(void);
extern void mtrr_aps_init(void);
extern void mtrr_bp_restore(void);
extern int mtrr_trim_uncached_memory(unsigned long end_pfn);
extern int amd_special_default_mtrr(void);
#  else
static inline u8 mtrr_type_lookup(u64 addr, u64 end, u8 *uniform)
{
	
	return MTRR_TYPE_INVALID;
}
#define mtrr_save_fixed_ranges(arg) do {} while (0)
#define mtrr_save_state() do {} while (0)
static inline int mtrr_add(unsigned long base, unsigned long size,
			   unsigned int type, bool increment)
{
    return -ENODEV;
}
static inline int mtrr_add_page(unsigned long base, unsigned long size,
				unsigned int type, bool increment)
{
    return -ENODEV;
}
static inline int mtrr_del(int reg, unsigned long base, unsigned long size)
{
    return -ENODEV;
}
static inline int mtrr_del_page(int reg, unsigned long base, unsigned long size)
{
    return -ENODEV;
}
static inline int mtrr_trim_uncached_memory(unsigned long end_pfn)
{
	return 0;
}
static inline void mtrr_centaur_report_mcr(int mcr, u32 lo, u32 hi)
{
}
#define mtrr_ap_init() do {} while (0)
#define set_mtrr_aps_delayed_init() do {} while (0)
#define mtrr_aps_init() do {} while (0)
#define mtrr_bp_restore() do {} while (0)
#  endif

#ifdef CONFIG_COMPAT
#include <linux/compat.h>

struct mtrr_sentry32 {
    compat_ulong_t base;    
    compat_uint_t size;    
    compat_uint_t type;     
};

struct mtrr_gentry32 {
    compat_ulong_t regnum;   
    compat_uint_t base;    
    compat_uint_t size;    
    compat_uint_t type;     
};

#define MTRR_IOCTL_BASE 'M'

#define MTRRIOC32_ADD_ENTRY      _IOW(MTRR_IOCTL_BASE,  0, struct mtrr_sentry32)
#define MTRRIOC32_SET_ENTRY      _IOW(MTRR_IOCTL_BASE,  1, struct mtrr_sentry32)
#define MTRRIOC32_DEL_ENTRY      _IOW(MTRR_IOCTL_BASE,  2, struct mtrr_sentry32)
#define MTRRIOC32_GET_ENTRY      _IOWR(MTRR_IOCTL_BASE, 3, struct mtrr_gentry32)
#define MTRRIOC32_KILL_ENTRY     _IOW(MTRR_IOCTL_BASE,  4, struct mtrr_sentry32)
#define MTRRIOC32_ADD_PAGE_ENTRY _IOW(MTRR_IOCTL_BASE,  5, struct mtrr_sentry32)
#define MTRRIOC32_SET_PAGE_ENTRY _IOW(MTRR_IOCTL_BASE,  6, struct mtrr_sentry32)
#define MTRRIOC32_DEL_PAGE_ENTRY _IOW(MTRR_IOCTL_BASE,  7, struct mtrr_sentry32)
#define MTRRIOC32_GET_PAGE_ENTRY _IOWR(MTRR_IOCTL_BASE, 8, struct mtrr_gentry32)
#define MTRRIOC32_KILL_PAGE_ENTRY		\
				 _IOW(MTRR_IOCTL_BASE,  9, struct mtrr_sentry32)
#endif 


#define MTRR_STATE_MTRR_FIXED_ENABLED	0x01
#define MTRR_STATE_MTRR_ENABLED		0x02

#endif 
