/* SPDX-License-Identifier: GPL-2.0+ */


#ifndef __LINUX_OF_GPIO_H
#define __LINUX_OF_GPIO_H

#include <linux/compiler.h>
#include <linux/gpio/driver.h>
#include <linux/gpio.h>		
#include <linux/of.h>

struct device_node;


enum of_gpio_flags {
	OF_GPIO_ACTIVE_LOW = 0x1,
	OF_GPIO_SINGLE_ENDED = 0x2,
	OF_GPIO_OPEN_DRAIN = 0x4,
	OF_GPIO_TRANSITORY = 0x8,
	OF_GPIO_PULL_UP = 0x10,
	OF_GPIO_PULL_DOWN = 0x20,
	OF_GPIO_PULL_DISABLE = 0x40,
};

#ifdef CONFIG_OF_GPIO

#include <linux/kernel.h>


struct of_mm_gpio_chip {
	struct gpio_chip gc;
	void (*save_regs)(struct of_mm_gpio_chip *mm_gc);
	void __iomem *regs;
};

static inline struct of_mm_gpio_chip *to_of_mm_gpio_chip(struct gpio_chip *gc)
{
	return container_of(gc, struct of_mm_gpio_chip, gc);
}

extern int of_get_named_gpio_flags(const struct device_node *np,
		const char *list_name, int index, enum of_gpio_flags *flags);

extern int of_mm_gpiochip_add_data(struct device_node *np,
				   struct of_mm_gpio_chip *mm_gc,
				   void *data);
static inline int of_mm_gpiochip_add(struct device_node *np,
				     struct of_mm_gpio_chip *mm_gc)
{
	return of_mm_gpiochip_add_data(np, mm_gc, NULL);
}
extern void of_mm_gpiochip_remove(struct of_mm_gpio_chip *mm_gc);

#else 

#include <linux/errno.h>


static inline int of_get_named_gpio_flags(const struct device_node *np,
		const char *list_name, int index, enum of_gpio_flags *flags)
{
	if (flags)
		*flags = 0;

	return -ENOSYS;
}

#endif 


static inline int of_gpio_named_count(const struct device_node *np,
				      const char *propname)
{
	return of_count_phandle_with_args(np, propname, "#gpio-cells");
}


static inline int of_gpio_count(const struct device_node *np)
{
	return of_gpio_named_count(np, "gpios");
}

static inline int of_get_gpio_flags(const struct device_node *np, int index,
		      enum of_gpio_flags *flags)
{
	return of_get_named_gpio_flags(np, "gpios", index, flags);
}


static inline int of_get_named_gpio(const struct device_node *np,
                                   const char *propname, int index)
{
	return of_get_named_gpio_flags(np, propname, index, NULL);
}


static inline int of_get_gpio(const struct device_node *np, int index)
{
	return of_get_gpio_flags(np, index, NULL);
}

#endif 
