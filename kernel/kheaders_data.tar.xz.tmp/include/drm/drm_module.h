/* SPDX-License-Identifier: MIT */

#ifndef DRM_MODULE_H
#define DRM_MODULE_H

#include <linux/pci.h>
#include <linux/platform_device.h>

#include <drm/drm_drv.h>





static inline int __init drm_pci_register_driver(struct pci_driver *pci_drv)
{
	if (drm_firmware_drivers_only())
		return -ENODEV;

	return pci_register_driver(pci_drv);
}


#define drm_module_pci_driver(__pci_drv) \
	module_driver(__pci_drv, drm_pci_register_driver, pci_unregister_driver)

static inline int __init
drm_pci_register_driver_if_modeset(struct pci_driver *pci_drv, int modeset)
{
	if (drm_firmware_drivers_only() && modeset == -1)
		return -ENODEV;
	if (modeset == 0)
		return -ENODEV;

	return pci_register_driver(pci_drv);
}

static inline void __exit
drm_pci_unregister_driver_if_modeset(struct pci_driver *pci_drv, int modeset)
{
	pci_unregister_driver(pci_drv);
}


#define drm_module_pci_driver_if_modeset(__pci_drv, __modeset) \
	module_driver(__pci_drv, drm_pci_register_driver_if_modeset, \
		      drm_pci_unregister_driver_if_modeset, __modeset)



static inline int __init
drm_platform_driver_register(struct platform_driver *platform_drv)
{
	if (drm_firmware_drivers_only())
		return -ENODEV;

	return platform_driver_register(platform_drv);
}


#define drm_module_platform_driver(__platform_drv) \
	module_driver(__platform_drv, drm_platform_driver_register, \
		      platform_driver_unregister)

#endif
