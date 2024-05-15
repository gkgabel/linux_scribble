/* SPDX-License-Identifier: MIT */


#ifndef _I915_PXP_TEE_INTERFACE_H_
#define _I915_PXP_TEE_INTERFACE_H_

#include <linux/mutex.h>
#include <linux/device.h>


struct i915_pxp_component_ops {
	
	struct module *owner;

	int (*send)(struct device *dev, const void *message, size_t size);
	int (*recv)(struct device *dev, void *buffer, size_t size);
};


struct i915_pxp_component {
	struct device *tee_dev;
	const struct i915_pxp_component_ops *ops;

	
	struct mutex mutex;
};

#endif 
