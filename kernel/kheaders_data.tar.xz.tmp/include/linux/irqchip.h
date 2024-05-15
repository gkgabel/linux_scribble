

#ifndef _LINUX_IRQCHIP_H
#define _LINUX_IRQCHIP_H

#include <linux/acpi.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>


extern of_irq_init_cb_t typecheck_irq_init_cb;

#define typecheck_irq_init_cb(fn)					\
	(__typecheck(typecheck_irq_init_cb, &fn) ? fn : fn)


#define IRQCHIP_DECLARE(name, compat, fn)	\
	OF_DECLARE_2(irqchip, name, compat, typecheck_irq_init_cb(fn))

extern int platform_irqchip_probe(struct platform_device *pdev);

#define IRQCHIP_PLATFORM_DRIVER_BEGIN(drv_name) \
static const struct of_device_id drv_name##_irqchip_match_table[] = {

#define IRQCHIP_MATCH(compat, fn) { .compatible = compat,		\
				    .data = typecheck_irq_init_cb(fn), },

#define IRQCHIP_PLATFORM_DRIVER_END(drv_name)				\
	{},								\
};									\
MODULE_DEVICE_TABLE(of, drv_name##_irqchip_match_table);		\
static struct platform_driver drv_name##_driver = {			\
	.probe  = IS_ENABLED(CONFIG_IRQCHIP) ? 				\
			platform_irqchip_probe : NULL,			\
	.driver = {							\
		.name = #drv_name,					\
		.owner = THIS_MODULE,					\
		.of_match_table = drv_name##_irqchip_match_table,	\
		.suppress_bind_attrs = true,				\
	},								\
};									\
builtin_platform_driver(drv_name##_driver)


#define IRQCHIP_ACPI_DECLARE(name, subtable, validate, data, fn)	\
	ACPI_DECLARE_SUBTABLE_PROBE_ENTRY(irqchip, name,		\
					  ACPI_SIG_MADT, subtable,	\
					  validate, data, fn)

#ifdef CONFIG_IRQCHIP
void irqchip_init(void);
#else
static inline void irqchip_init(void) {}
#endif

#endif
