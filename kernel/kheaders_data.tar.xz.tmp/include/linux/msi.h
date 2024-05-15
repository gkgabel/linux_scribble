/* SPDX-License-Identifier: GPL-2.0 */
#ifndef LINUX_MSI_H
#define LINUX_MSI_H



#include <linux/cpumask.h>
#include <linux/xarray.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <asm/msi.h>


#ifndef arch_msi_msg_addr_lo
typedef struct arch_msi_msg_addr_lo {
	u32	address_lo;
} __attribute__ ((packed)) arch_msi_msg_addr_lo_t;
#endif

#ifndef arch_msi_msg_addr_hi
typedef struct arch_msi_msg_addr_hi {
	u32	address_hi;
} __attribute__ ((packed)) arch_msi_msg_addr_hi_t;
#endif

#ifndef arch_msi_msg_data
typedef struct arch_msi_msg_data {
	u32	data;
} __attribute__ ((packed)) arch_msi_msg_data_t;
#endif


struct msi_msg {
	union {
		u32			address_lo;
		arch_msi_msg_addr_lo_t	arch_addr_lo;
	};
	union {
		u32			address_hi;
		arch_msi_msg_addr_hi_t	arch_addr_hi;
	};
	union {
		u32			data;
		arch_msi_msg_data_t	arch_data;
	};
};

extern int pci_msi_ignore_mask;

struct irq_data;
struct msi_desc;
struct pci_dev;
struct platform_msi_priv_data;
struct device_attribute;

void __get_cached_msi_msg(struct msi_desc *entry, struct msi_msg *msg);
#ifdef CONFIG_GENERIC_MSI_IRQ
void get_cached_msi_msg(unsigned int irq, struct msi_msg *msg);
#else
static inline void get_cached_msi_msg(unsigned int irq, struct msi_msg *msg)
{
}
#endif

typedef void (*irq_write_msi_msg_t)(struct msi_desc *desc,
				    struct msi_msg *msg);


struct pci_msi_desc {
	union {
		u32 msi_mask;
		u32 msix_ctrl;
	};
	struct {
		u8	is_msix		: 1;
		u8	multiple	: 3;
		u8	multi_cap	: 3;
		u8	can_mask	: 1;
		u8	is_64		: 1;
		u8	is_virtual	: 1;
		unsigned default_irq;
	} msi_attrib;
	union {
		u8	mask_pos;
		void __iomem *mask_base;
	};
};

#define MSI_MAX_INDEX		((unsigned int)USHRT_MAX)


struct msi_desc {
	
	unsigned int			irq;
	unsigned int			nvec_used;
	struct device			*dev;
	struct msi_msg			msg;
	struct irq_affinity_desc	*affinity;
#ifdef CONFIG_IRQ_MSI_IOMMU
	const void			*iommu_cookie;
#endif
#ifdef CONFIG_SYSFS
	struct device_attribute		*sysfs_attrs;
#endif

	void (*write_msi_msg)(struct msi_desc *entry, void *data);
	void *write_msi_msg_data;

	u16				msi_index;
	struct pci_msi_desc		pci;
};


enum msi_desc_filter {
	
	MSI_DESC_ALL,
	
	MSI_DESC_NOTASSOCIATED,
	
	MSI_DESC_ASSOCIATED,
};


struct msi_device_data {
	unsigned long			properties;
	struct platform_msi_priv_data	*platform_data;
	struct mutex			mutex;
	struct xarray			__store;
	unsigned long			__iter_idx;
};

int msi_setup_device_data(struct device *dev);

unsigned int msi_get_virq(struct device *dev, unsigned int index);
void msi_lock_descs(struct device *dev);
void msi_unlock_descs(struct device *dev);

struct msi_desc *msi_first_desc(struct device *dev, enum msi_desc_filter filter);
struct msi_desc *msi_next_desc(struct device *dev, enum msi_desc_filter filter);


#define msi_for_each_desc(desc, dev, filter)			\
	for ((desc) = msi_first_desc((dev), (filter)); (desc);	\
	     (desc) = msi_next_desc((dev), (filter)))

#define msi_desc_to_dev(desc)		((desc)->dev)

#ifdef CONFIG_IRQ_MSI_IOMMU
static inline const void *msi_desc_get_iommu_cookie(struct msi_desc *desc)
{
	return desc->iommu_cookie;
}

static inline void msi_desc_set_iommu_cookie(struct msi_desc *desc,
					     const void *iommu_cookie)
{
	desc->iommu_cookie = iommu_cookie;
}
#else
static inline const void *msi_desc_get_iommu_cookie(struct msi_desc *desc)
{
	return NULL;
}

static inline void msi_desc_set_iommu_cookie(struct msi_desc *desc,
					     const void *iommu_cookie)
{
}
#endif

#ifdef CONFIG_PCI_MSI
struct pci_dev *msi_desc_to_pci_dev(struct msi_desc *desc);
void pci_write_msi_msg(unsigned int irq, struct msi_msg *msg);
#else 
static inline void pci_write_msi_msg(unsigned int irq, struct msi_msg *msg)
{
}
#endif 

int msi_add_msi_desc(struct device *dev, struct msi_desc *init_desc);
void msi_free_msi_descs_range(struct device *dev, enum msi_desc_filter filter,
			      unsigned int first_index, unsigned int last_index);


static inline void msi_free_msi_descs(struct device *dev)
{
	msi_free_msi_descs_range(dev, MSI_DESC_ALL, 0, MSI_MAX_INDEX);
}

void __pci_read_msi_msg(struct msi_desc *entry, struct msi_msg *msg);
void __pci_write_msi_msg(struct msi_desc *entry, struct msi_msg *msg);

void pci_msi_mask_irq(struct irq_data *data);
void pci_msi_unmask_irq(struct irq_data *data);


#ifdef CONFIG_PCI_MSI_ARCH_FALLBACKS
int arch_setup_msi_irq(struct pci_dev *dev, struct msi_desc *desc);
void arch_teardown_msi_irq(unsigned int irq);
int arch_setup_msi_irqs(struct pci_dev *dev, int nvec, int type);
void arch_teardown_msi_irqs(struct pci_dev *dev);
#ifdef CONFIG_SYSFS
int msi_device_populate_sysfs(struct device *dev);
void msi_device_destroy_sysfs(struct device *dev);
#else 
static inline int msi_device_populate_sysfs(struct device *dev) { return 0; }
static inline void msi_device_destroy_sysfs(struct device *dev) { }
#endif 
#endif 


bool arch_restore_msi_irqs(struct pci_dev *dev);

#ifdef CONFIG_GENERIC_MSI_IRQ_DOMAIN

#include <linux/irqhandler.h>

struct irq_domain;
struct irq_domain_ops;
struct irq_chip;
struct device_node;
struct fwnode_handle;
struct msi_domain_info;


struct msi_domain_ops {
	irq_hw_number_t	(*get_hwirq)(struct msi_domain_info *info,
				     msi_alloc_info_t *arg);
	int		(*msi_init)(struct irq_domain *domain,
				    struct msi_domain_info *info,
				    unsigned int virq, irq_hw_number_t hwirq,
				    msi_alloc_info_t *arg);
	void		(*msi_free)(struct irq_domain *domain,
				    struct msi_domain_info *info,
				    unsigned int virq);
	int		(*msi_check)(struct irq_domain *domain,
				     struct msi_domain_info *info,
				     struct device *dev);
	int		(*msi_prepare)(struct irq_domain *domain,
				       struct device *dev, int nvec,
				       msi_alloc_info_t *arg);
	void		(*set_desc)(msi_alloc_info_t *arg,
				    struct msi_desc *desc);
	int		(*domain_alloc_irqs)(struct irq_domain *domain,
					     struct device *dev, int nvec);
	void		(*domain_free_irqs)(struct irq_domain *domain,
					    struct device *dev);
};


struct msi_domain_info {
	u32			flags;
	struct msi_domain_ops	*ops;
	struct irq_chip		*chip;
	void			*chip_data;
	irq_flow_handler_t	handler;
	void			*handler_data;
	const char		*handler_name;
	void			*data;
};


enum {
	
	MSI_FLAG_USE_DEF_DOM_OPS	= (1 << 0),
	
	MSI_FLAG_USE_DEF_CHIP_OPS	= (1 << 1),
	
	MSI_FLAG_MULTI_PCI_MSI		= (1 << 2),
	
	MSI_FLAG_PCI_MSIX		= (1 << 3),
	
	MSI_FLAG_ACTIVATE_EARLY		= (1 << 4),
	
	MSI_FLAG_MUST_REACTIVATE	= (1 << 5),
	
	MSI_FLAG_LEVEL_CAPABLE		= (1 << 6),
	
	MSI_FLAG_DEV_SYSFS		= (1 << 7),
	
	MSI_FLAG_MSIX_CONTIGUOUS	= (1 << 8),
	
	MSI_FLAG_ALLOC_SIMPLE_MSI_DESCS	= (1 << 9),
	
	MSI_FLAG_FREE_MSI_DESCS		= (1 << 10),
};

int msi_domain_set_affinity(struct irq_data *data, const struct cpumask *mask,
			    bool force);

struct irq_domain *msi_create_irq_domain(struct fwnode_handle *fwnode,
					 struct msi_domain_info *info,
					 struct irq_domain *parent);
int __msi_domain_alloc_irqs(struct irq_domain *domain, struct device *dev,
			    int nvec);
int msi_domain_alloc_irqs_descs_locked(struct irq_domain *domain, struct device *dev,
				       int nvec);
int msi_domain_alloc_irqs(struct irq_domain *domain, struct device *dev,
			  int nvec);
void __msi_domain_free_irqs(struct irq_domain *domain, struct device *dev);
void msi_domain_free_irqs_descs_locked(struct irq_domain *domain, struct device *dev);
void msi_domain_free_irqs(struct irq_domain *domain, struct device *dev);
struct msi_domain_info *msi_get_domain_info(struct irq_domain *domain);

struct irq_domain *platform_msi_create_irq_domain(struct fwnode_handle *fwnode,
						  struct msi_domain_info *info,
						  struct irq_domain *parent);
int platform_msi_domain_alloc_irqs(struct device *dev, unsigned int nvec,
				   irq_write_msi_msg_t write_msi_msg);
void platform_msi_domain_free_irqs(struct device *dev);


int msi_domain_prepare_irqs(struct irq_domain *domain, struct device *dev,
			    int nvec, msi_alloc_info_t *args);
int msi_domain_populate_irqs(struct irq_domain *domain, struct device *dev,
			     int virq, int nvec, msi_alloc_info_t *args);
struct irq_domain *
__platform_msi_create_device_domain(struct device *dev,
				    unsigned int nvec,
				    bool is_tree,
				    irq_write_msi_msg_t write_msi_msg,
				    const struct irq_domain_ops *ops,
				    void *host_data);

#define platform_msi_create_device_domain(dev, nvec, write, ops, data)	\
	__platform_msi_create_device_domain(dev, nvec, false, write, ops, data)
#define platform_msi_create_device_tree_domain(dev, nvec, write, ops, data) \
	__platform_msi_create_device_domain(dev, nvec, true, write, ops, data)

int platform_msi_device_domain_alloc(struct irq_domain *domain, unsigned int virq,
				     unsigned int nr_irqs);
void platform_msi_device_domain_free(struct irq_domain *domain, unsigned int virq,
				     unsigned int nvec);
void *platform_msi_get_host_data(struct irq_domain *domain);
#endif 

#ifdef CONFIG_PCI_MSI_IRQ_DOMAIN
struct irq_domain *pci_msi_create_irq_domain(struct fwnode_handle *fwnode,
					     struct msi_domain_info *info,
					     struct irq_domain *parent);
u32 pci_msi_domain_get_msi_rid(struct irq_domain *domain, struct pci_dev *pdev);
struct irq_domain *pci_msi_get_device_domain(struct pci_dev *pdev);
bool pci_dev_has_special_msi_domain(struct pci_dev *pdev);
#else
static inline struct irq_domain *pci_msi_get_device_domain(struct pci_dev *pdev)
{
	return NULL;
}
#endif 

#endif 
