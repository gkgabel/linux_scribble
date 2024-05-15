/* SPDX-License-Identifier: GPL-2.0 */


#ifndef __LINUX_SOC_SAMSUNG_S3C_PM_H
#define __LINUX_SOC_SAMSUNG_S3C_PM_H __FILE__

#include <linux/types.h>




struct pm_uart_save {
	u32	ulcon;
	u32	ucon;
	u32	ufcon;
	u32	umcon;
	u32	ubrdiv;
	u32	udivslot;
};

#ifdef CONFIG_SAMSUNG_PM_DEBUG

extern void s3c_pm_dbg(const char *msg, ...);

#define S3C_PMDBG(fmt...) s3c_pm_dbg(fmt)

extern void s3c_pm_save_uarts(bool is_s3c24xx);
extern void s3c_pm_restore_uarts(bool is_s3c24xx);

#ifdef CONFIG_ARCH_S3C64XX
extern void s3c_pm_arch_update_uart(void __iomem *regs,
				    struct pm_uart_save *save);
#else
static inline void
s3c_pm_arch_update_uart(void __iomem *regs, struct pm_uart_save *save)
{
}
#endif

#else
#define S3C_PMDBG(fmt...) pr_debug(fmt)

static inline void s3c_pm_save_uarts(bool is_s3c24xx) { }
static inline void s3c_pm_restore_uarts(bool is_s3c24xx) { }
#endif



#ifdef CONFIG_SAMSUNG_PM_CHECK
extern void s3c_pm_check_prepare(void);
extern void s3c_pm_check_restore(void);
extern void s3c_pm_check_cleanup(void);
extern void s3c_pm_check_store(void);
#else
#define s3c_pm_check_prepare() do { } while (0)
#define s3c_pm_check_restore() do { } while (0)
#define s3c_pm_check_cleanup() do { } while (0)
#define s3c_pm_check_store()   do { } while (0)
#endif



extern struct bus_type s3c2410_subsys;
extern struct bus_type s3c2410a_subsys;
extern struct bus_type s3c2412_subsys;
extern struct bus_type s3c2416_subsys;
extern struct bus_type s3c2440_subsys;
extern struct bus_type s3c2442_subsys;
extern struct bus_type s3c2443_subsys;

#endif
