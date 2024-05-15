/* SPDX-License-Identifier: GPL-2.0+ */


#ifndef __LINUX_MFD_RSMU_H
#define __LINUX_MFD_RSMU_H


enum rsmu_type {
	RSMU_CM		= 0x34000,
	RSMU_SABRE	= 0x33810,
	RSMU_SL		= 0x19850,
};


struct rsmu_ddata {
	struct device *dev;
	struct regmap *regmap;
	struct mutex lock;
	enum rsmu_type type;
	u16 page;
};
#endif 
