/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __DMA_IOMMU_H
#define __DMA_IOMMU_H

#include <linux/errno.h>
#include <linux/types.h>

#ifdef CONFIG_IOMMU_DMA
#include <linux/dma-mapping.h>
#include <linux/iommu.h>
#include <linux/msi.h>


int iommu_get_dma_cookie(struct iommu_domain *domain);
int iommu_get_msi_cookie(struct iommu_domain *domain, dma_addr_t base);
void iommu_put_dma_cookie(struct iommu_domain *domain);


void iommu_setup_dma_ops(struct device *dev, u64 dma_base, u64 dma_limit);
int iommu_dma_init_fq(struct iommu_domain *domain);



int iommu_dma_prepare_msi(struct msi_desc *desc, phys_addr_t msi_addr);


void iommu_dma_compose_msi_msg(struct msi_desc *desc,
			       struct msi_msg *msg);

void iommu_dma_get_resv_regions(struct device *dev, struct list_head *list);

void iommu_dma_free_cpu_cached_iovas(unsigned int cpu,
		struct iommu_domain *domain);

extern bool iommu_dma_forcedac;

#else 

struct iommu_domain;
struct msi_desc;
struct msi_msg;
struct device;

static inline void iommu_setup_dma_ops(struct device *dev, u64 dma_base,
				       u64 dma_limit)
{
}

static inline int iommu_dma_init_fq(struct iommu_domain *domain)
{
	return -EINVAL;
}

static inline int iommu_get_dma_cookie(struct iommu_domain *domain)
{
	return -ENODEV;
}

static inline int iommu_get_msi_cookie(struct iommu_domain *domain, dma_addr_t base)
{
	return -ENODEV;
}

static inline void iommu_put_dma_cookie(struct iommu_domain *domain)
{
}

static inline int iommu_dma_prepare_msi(struct msi_desc *desc,
					phys_addr_t msi_addr)
{
	return 0;
}

static inline void iommu_dma_compose_msi_msg(struct msi_desc *desc,
					     struct msi_msg *msg)
{
}

static inline void iommu_dma_get_resv_regions(struct device *dev, struct list_head *list)
{
}

#endif	
#endif	
