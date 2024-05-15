/* SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause) */


#ifndef __SDW_INTEL_H
#define __SDW_INTEL_H

#include <linux/irqreturn.h>
#include <linux/soundwire/sdw.h>

#define SDW_SHIM_BASE			0x2C000
#define SDW_ALH_BASE			0x2C800
#define SDW_SHIM_BASE_ACE		0x38000
#define SDW_ALH_BASE_ACE		0x24000
#define SDW_LINK_BASE			0x30000
#define SDW_LINK_SIZE			0x10000


#define SDW_SHIM_LCAP			0x0
#define SDW_SHIM_LCTL			0x4
#define SDW_SHIM_IPPTR			0x8
#define SDW_SHIM_SYNC			0xC

#define SDW_SHIM_CTLSCAP(x)		(0x010 + 0x60 * (x))
#define SDW_SHIM_CTLS0CM(x)		(0x012 + 0x60 * (x))
#define SDW_SHIM_CTLS1CM(x)		(0x014 + 0x60 * (x))
#define SDW_SHIM_CTLS2CM(x)		(0x016 + 0x60 * (x))
#define SDW_SHIM_CTLS3CM(x)		(0x018 + 0x60 * (x))
#define SDW_SHIM_PCMSCAP(x)		(0x020 + 0x60 * (x))

#define SDW_SHIM_PCMSYCHM(x, y)		(0x022 + (0x60 * (x)) + (0x2 * (y)))
#define SDW_SHIM_PCMSYCHC(x, y)		(0x042 + (0x60 * (x)) + (0x2 * (y)))
#define SDW_SHIM_PDMSCAP(x)		(0x062 + 0x60 * (x))
#define SDW_SHIM_IOCTL(x)		(0x06C + 0x60 * (x))
#define SDW_SHIM_CTMCTL(x)		(0x06E + 0x60 * (x))

#define SDW_SHIM_WAKEEN			0x190
#define SDW_SHIM_WAKESTS		0x192

#define SDW_SHIM_LCTL_SPA		BIT(0)
#define SDW_SHIM_LCTL_SPA_MASK		GENMASK(3, 0)
#define SDW_SHIM_LCTL_CPA		BIT(8)
#define SDW_SHIM_LCTL_CPA_MASK		GENMASK(11, 8)

#define SDW_SHIM_SYNC_SYNCPRD_VAL_24	(24000 / SDW_CADENCE_GSYNC_KHZ - 1)
#define SDW_SHIM_SYNC_SYNCPRD_VAL_38_4	(38400 / SDW_CADENCE_GSYNC_KHZ - 1)
#define SDW_SHIM_SYNC_SYNCPRD		GENMASK(14, 0)
#define SDW_SHIM_SYNC_SYNCCPU		BIT(15)
#define SDW_SHIM_SYNC_CMDSYNC_MASK	GENMASK(19, 16)
#define SDW_SHIM_SYNC_CMDSYNC		BIT(16)
#define SDW_SHIM_SYNC_SYNCGO		BIT(24)

#define SDW_SHIM_PCMSCAP_ISS		GENMASK(3, 0)
#define SDW_SHIM_PCMSCAP_OSS		GENMASK(7, 4)
#define SDW_SHIM_PCMSCAP_BSS		GENMASK(12, 8)

#define SDW_SHIM_PCMSYCM_LCHN		GENMASK(3, 0)
#define SDW_SHIM_PCMSYCM_HCHN		GENMASK(7, 4)
#define SDW_SHIM_PCMSYCM_STREAM		GENMASK(13, 8)
#define SDW_SHIM_PCMSYCM_DIR		BIT(15)

#define SDW_SHIM_PDMSCAP_ISS		GENMASK(3, 0)
#define SDW_SHIM_PDMSCAP_OSS		GENMASK(7, 4)
#define SDW_SHIM_PDMSCAP_BSS		GENMASK(12, 8)
#define SDW_SHIM_PDMSCAP_CPSS		GENMASK(15, 13)

#define SDW_SHIM_IOCTL_MIF		BIT(0)
#define SDW_SHIM_IOCTL_CO		BIT(1)
#define SDW_SHIM_IOCTL_COE		BIT(2)
#define SDW_SHIM_IOCTL_DO		BIT(3)
#define SDW_SHIM_IOCTL_DOE		BIT(4)
#define SDW_SHIM_IOCTL_BKE		BIT(5)
#define SDW_SHIM_IOCTL_WPDD		BIT(6)
#define SDW_SHIM_IOCTL_CIBD		BIT(8)
#define SDW_SHIM_IOCTL_DIBD		BIT(9)

#define SDW_SHIM_CTMCTL_DACTQE		BIT(0)
#define SDW_SHIM_CTMCTL_DODS		BIT(1)
#define SDW_SHIM_CTMCTL_DOAIS		GENMASK(4, 3)

#define SDW_SHIM_WAKEEN_ENABLE		BIT(0)
#define SDW_SHIM_WAKESTS_STATUS		BIT(0)


#define SDW_ALH_STRMZCFG(x)		(0x000 + (0x4 * (x)))
#define SDW_ALH_NUM_STREAMS		64

#define SDW_ALH_STRMZCFG_DMAT_VAL	0x3
#define SDW_ALH_STRMZCFG_DMAT		GENMASK(7, 0)
#define SDW_ALH_STRMZCFG_CHN		GENMASK(19, 16)


struct sdw_intel_stream_params_data {
	int stream;
	struct snd_soc_dai *dai;
	struct snd_pcm_hw_params *hw_params;
	int link_id;
	int alh_stream_id;
};


struct sdw_intel_stream_free_data {
	int stream;
	struct snd_soc_dai *dai;
	int link_id;
};


struct sdw_intel_ops {
	int (*params_stream)(struct device *dev,
			     struct sdw_intel_stream_params_data *params_data);
	int (*free_stream)(struct device *dev,
			   struct sdw_intel_stream_free_data *free_data);
	int (*trigger)(struct snd_soc_dai *dai, int cmd, int stream);
};


struct sdw_intel_acpi_info {
	acpi_handle handle;
	int count;
	u32 link_mask;
};

struct sdw_intel_link_dev;




#define SDW_INTEL_CLK_STOP_NOT_ALLOWED		BIT(0)


#define SDW_INTEL_CLK_STOP_TEARDOWN		BIT(1)


#define SDW_INTEL_CLK_STOP_WAKE_CAPABLE_ONLY	BIT(2)


#define SDW_INTEL_CLK_STOP_BUS_RESET		BIT(3)

struct sdw_intel_slave_id {
	int link_id;
	struct sdw_slave_id id;
};


struct sdw_intel_ctx {
	int count;
	void __iomem *mmio_base;
	u32 link_mask;
	int num_slaves;
	acpi_handle handle;
	struct sdw_intel_link_dev **ldev;
	struct sdw_intel_slave_id *ids;
	struct list_head link_list;
	struct mutex shim_lock; 
	u32 shim_mask;
	u32 shim_base;
	u32 alh_base;
};


struct sdw_intel_res {
	int count;
	void __iomem *mmio_base;
	int irq;
	acpi_handle handle;
	struct device *parent;
	const struct sdw_intel_ops *ops;
	struct device *dev;
	u32 link_mask;
	u32 clock_stop_quirks;
	u32 shim_base;
	u32 alh_base;
};


int sdw_intel_acpi_scan(acpi_handle *parent_handle,
			struct sdw_intel_acpi_info *info);

void sdw_intel_process_wakeen_event(struct sdw_intel_ctx *ctx);

struct sdw_intel_ctx *
sdw_intel_probe(struct sdw_intel_res *res);

int sdw_intel_startup(struct sdw_intel_ctx *ctx);

void sdw_intel_exit(struct sdw_intel_ctx *ctx);

void sdw_intel_enable_irq(void __iomem *mmio_base, bool enable);

irqreturn_t sdw_intel_thread(int irq, void *dev_id);

#define SDW_INTEL_QUIRK_MASK_BUS_DISABLE      BIT(1)

#endif
