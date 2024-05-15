/* SPDX-License-Identifier: GPL-2.0 */

#ifndef MDIO_I2C_H
#define MDIO_I2C_H

struct device;
struct i2c_adapter;
struct mii_bus;

struct mii_bus *mdio_i2c_alloc(struct device *parent, struct i2c_adapter *i2c);

#endif
