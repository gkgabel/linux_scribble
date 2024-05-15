/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef _PRUSS_DRIVER_H_
#define _PRUSS_DRIVER_H_

#include <linux/types.h>


enum pruss_mem {
	PRUSS_MEM_DRAM0 = 0,
	PRUSS_MEM_DRAM1,
	PRUSS_MEM_SHRD_RAM2,
	PRUSS_MEM_MAX,
};


struct pruss_mem_region {
	void __iomem *va;
	phys_addr_t pa;
	size_t size;
};


struct pruss {
	struct device *dev;
	void __iomem *cfg_base;
	struct regmap *cfg_regmap;
	struct pruss_mem_region mem_regions[PRUSS_MEM_MAX];
	struct clk *core_clk_mux;
	struct clk *iep_clk_mux;
};

#endif	
