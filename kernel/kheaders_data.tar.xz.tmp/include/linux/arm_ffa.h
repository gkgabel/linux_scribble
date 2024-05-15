/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef _LINUX_ARM_FFA_H
#define _LINUX_ARM_FFA_H

#include <linux/device.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/uuid.h>


struct ffa_device {
	int vm_id;
	bool mode_32bit;
	uuid_t uuid;
	struct device dev;
};

#define to_ffa_dev(d) container_of(d, struct ffa_device, dev)

struct ffa_device_id {
	uuid_t uuid;
};

struct ffa_driver {
	const char *name;
	int (*probe)(struct ffa_device *sdev);
	void (*remove)(struct ffa_device *sdev);
	const struct ffa_device_id *id_table;

	struct device_driver driver;
};

#define to_ffa_driver(d) container_of(d, struct ffa_driver, driver)

static inline void ffa_dev_set_drvdata(struct ffa_device *fdev, void *data)
{
	dev_set_drvdata(&fdev->dev, data);
}

static inline void *ffa_dev_get_drvdata(struct ffa_device *fdev)
{
	return dev_get_drvdata(&fdev->dev);
}

#if IS_REACHABLE(CONFIG_ARM_FFA_TRANSPORT)
struct ffa_device *ffa_device_register(const uuid_t *uuid, int vm_id);
void ffa_device_unregister(struct ffa_device *ffa_dev);
int ffa_driver_register(struct ffa_driver *driver, struct module *owner,
			const char *mod_name);
void ffa_driver_unregister(struct ffa_driver *driver);
bool ffa_device_is_valid(struct ffa_device *ffa_dev);
const struct ffa_dev_ops *ffa_dev_ops_get(struct ffa_device *dev);

#else
static inline
struct ffa_device *ffa_device_register(const uuid_t *uuid, int vm_id)
{
	return NULL;
}

static inline void ffa_device_unregister(struct ffa_device *dev) {}

static inline int
ffa_driver_register(struct ffa_driver *driver, struct module *owner,
		    const char *mod_name)
{
	return -EINVAL;
}

static inline void ffa_driver_unregister(struct ffa_driver *driver) {}

static inline
bool ffa_device_is_valid(struct ffa_device *ffa_dev) { return false; }

static inline
const struct ffa_dev_ops *ffa_dev_ops_get(struct ffa_device *dev)
{
	return NULL;
}
#endif 

#define ffa_register(driver) \
	ffa_driver_register(driver, THIS_MODULE, KBUILD_MODNAME)
#define ffa_unregister(driver) \
	ffa_driver_unregister(driver)


#define module_ffa_driver(__ffa_driver)	\
	module_driver(__ffa_driver, ffa_register, ffa_unregister)


struct ffa_partition_info {
	u16 id;
	u16 exec_ctxt;

#define FFA_PARTITION_DIRECT_RECV	BIT(0)

#define FFA_PARTITION_DIRECT_SEND	BIT(1)

#define FFA_PARTITION_INDIRECT_MSG	BIT(2)
	u32 properties;
};


struct ffa_send_direct_data {
	unsigned long data0; 
	unsigned long data1; 
	unsigned long data2; 
	unsigned long data3; 
	unsigned long data4; 
};

struct ffa_mem_region_addr_range {
	
	u64 address;
	
	u32 pg_cnt;
	u32 reserved;
};

struct ffa_composite_mem_region {
	
	u32 total_pg_cnt;
	
	u32 addr_range_cnt;
	u64 reserved;
	
	struct ffa_mem_region_addr_range constituents[];
};

struct ffa_mem_region_attributes {
	
	u16 receiver;
	
#define FFA_MEM_EXEC		BIT(3)
#define FFA_MEM_NO_EXEC		BIT(2)
#define FFA_MEM_RW		BIT(1)
#define FFA_MEM_RO		BIT(0)
	u8 attrs;
	
#define FFA_MEM_RETRIEVE_SELF_BORROWER	BIT(0)
	u8 flag;
	u32 composite_off;
	
	u64 reserved;
};

struct ffa_mem_region {
	
	u16 sender_id;
#define FFA_MEM_NORMAL		BIT(5)
#define FFA_MEM_DEVICE		BIT(4)

#define FFA_MEM_WRITE_BACK	(3 << 2)
#define FFA_MEM_NON_CACHEABLE	(1 << 2)

#define FFA_DEV_nGnRnE		(0 << 2)
#define FFA_DEV_nGnRE		(1 << 2)
#define FFA_DEV_nGRE		(2 << 2)
#define FFA_DEV_GRE		(3 << 2)

#define FFA_MEM_NON_SHAREABLE	(0)
#define FFA_MEM_OUTER_SHAREABLE	(2)
#define FFA_MEM_INNER_SHAREABLE	(3)
	u8 attributes;
	u8 reserved_0;

#define FFA_MEM_CLEAR			BIT(0)

#define FFA_TIME_SLICE_ENABLE		BIT(1)

#define FFA_MEM_RETRIEVE_TYPE_IN_RESP	(0 << 3)
#define FFA_MEM_RETRIEVE_TYPE_SHARE	(1 << 3)
#define FFA_MEM_RETRIEVE_TYPE_LEND	(2 << 3)
#define FFA_MEM_RETRIEVE_TYPE_DONATE	(3 << 3)

#define FFA_MEM_RETRIEVE_ADDR_ALIGN_HINT	BIT(9)
#define FFA_MEM_RETRIEVE_ADDR_ALIGN(x)		((x) << 5)
	
	u32 flags;
#define HANDLE_LOW_MASK		GENMASK_ULL(31, 0)
#define HANDLE_HIGH_MASK	GENMASK_ULL(63, 32)
#define HANDLE_LOW(x)		((u32)(FIELD_GET(HANDLE_LOW_MASK, (x))))
#define	HANDLE_HIGH(x)		((u32)(FIELD_GET(HANDLE_HIGH_MASK, (x))))

#define PACK_HANDLE(l, h)		\
	(FIELD_PREP(HANDLE_LOW_MASK, (l)) | FIELD_PREP(HANDLE_HIGH_MASK, (h)))
	
	u64 handle;
	
	u64 tag;
	u32 reserved_1;
	
	u32 ep_count;
	
	struct ffa_mem_region_attributes ep_mem_access[];
};

#define	COMPOSITE_OFFSET(x)	\
	(offsetof(struct ffa_mem_region, ep_mem_access[x]))
#define CONSTITUENTS_OFFSET(x)	\
	(offsetof(struct ffa_composite_mem_region, constituents[x]))
#define COMPOSITE_CONSTITUENTS_OFFSET(x, y)	\
	(COMPOSITE_OFFSET(x) + CONSTITUENTS_OFFSET(y))

struct ffa_mem_ops_args {
	bool use_txbuf;
	u32 nattrs;
	u32 flags;
	u64 tag;
	u64 g_handle;
	struct scatterlist *sg;
	struct ffa_mem_region_attributes *attrs;
};

struct ffa_dev_ops {
	u32 (*api_version_get)(void);
	int (*partition_info_get)(const char *uuid_str,
				  struct ffa_partition_info *buffer);
	void (*mode_32bit_set)(struct ffa_device *dev);
	int (*sync_send_receive)(struct ffa_device *dev,
				 struct ffa_send_direct_data *data);
	int (*memory_reclaim)(u64 g_handle, u32 flags);
	int (*memory_share)(struct ffa_device *dev,
			    struct ffa_mem_ops_args *args);
	int (*memory_lend)(struct ffa_device *dev,
			   struct ffa_mem_ops_args *args);
};

#endif 
