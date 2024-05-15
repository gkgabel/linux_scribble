/* SPDX-License-Identifier: GPL-2.0-or-later */


#ifndef MEDIA_M5MOLS_H
#define MEDIA_M5MOLS_H


struct m5mols_platform_data {
	int (*set_power)(struct device *dev, int on);
};

#endif	
