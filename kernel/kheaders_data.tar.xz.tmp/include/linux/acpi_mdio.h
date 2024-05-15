/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __LINUX_ACPI_MDIO_H
#define __LINUX_ACPI_MDIO_H

#include <linux/phy.h>

#if IS_ENABLED(CONFIG_ACPI_MDIO)
int acpi_mdiobus_register(struct mii_bus *mdio, struct fwnode_handle *fwnode);
#else 
static inline int
acpi_mdiobus_register(struct mii_bus *mdio, struct fwnode_handle *fwnode)
{
	

	return mdiobus_register(mdio);
}
#endif

#endif 
