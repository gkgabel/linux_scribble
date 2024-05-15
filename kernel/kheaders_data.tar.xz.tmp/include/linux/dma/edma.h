/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _DW_EDMA_H
#define _DW_EDMA_H

#include <linux/device.h>
#include <linux/dmaengine.h>

#define EDMA_MAX_WR_CH                                  8
#define EDMA_MAX_RD_CH                                  8

struct dw_edma;

struct dw_edma_region {
	phys_addr_t	paddr;
	void __iomem	*vaddr;
	size_t		sz;
};

struct dw_edma_core_ops {
	int (*irq_vector)(struct device *dev, unsigned int nr);
};

enum dw_edma_map_format {
	EDMA_MF_EDMA_LEGACY = 0x0,
	EDMA_MF_EDMA_UNROLL = 0x1,
	EDMA_MF_HDMA_COMPAT = 0x5
};


enum dw_edma_chip_flags {
	DW_EDMA_CHIP_LOCAL	= BIT(0),
};


struct dw_edma_chip {
	struct device		*dev;
	int			id;
	int			nr_irqs;
	const struct dw_edma_core_ops   *ops;
	u32			flags;

	void __iomem		*reg_base;

	u16			ll_wr_cnt;
	u16			ll_rd_cnt;
	
	struct dw_edma_region	ll_region_wr[EDMA_MAX_WR_CH];
	struct dw_edma_region	ll_region_rd[EDMA_MAX_RD_CH];

	
	struct dw_edma_region	dt_region_wr[EDMA_MAX_WR_CH];
	struct dw_edma_region	dt_region_rd[EDMA_MAX_RD_CH];

	enum dw_edma_map_format	mf;

	struct dw_edma		*dw;
};


#if IS_ENABLED(CONFIG_DW_EDMA)
int dw_edma_probe(struct dw_edma_chip *chip);
int dw_edma_remove(struct dw_edma_chip *chip);
#else
static inline int dw_edma_probe(struct dw_edma_chip *chip)
{
	return -ENODEV;
}

static inline int dw_edma_remove(struct dw_edma_chip *chip)
{
	return 0;
}
#endif 

#endif 
