/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef VFIO_H
#define VFIO_H


#include <linux/iommu.h>
#include <linux/mm.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <uapi/linux/vfio.h>

struct kvm;


struct vfio_device_set {
	void *set_id;
	struct mutex lock;
	struct list_head device_list;
	unsigned int device_count;
};

struct vfio_device {
	struct device *dev;
	const struct vfio_device_ops *ops;
	
	const struct vfio_migration_ops *mig_ops;
	struct vfio_group *group;
	struct vfio_device_set *dev_set;
	struct list_head dev_set_list;
	unsigned int migration_flags;
	
	struct kvm *kvm;

	
	refcount_t refcount;
	unsigned int open_count;
	struct completion comp;
	struct list_head group_next;
	struct list_head iommu_entry;
};


struct vfio_device_ops {
	char	*name;
	int	(*open_device)(struct vfio_device *vdev);
	void	(*close_device)(struct vfio_device *vdev);
	ssize_t	(*read)(struct vfio_device *vdev, char __user *buf,
			size_t count, loff_t *ppos);
	ssize_t	(*write)(struct vfio_device *vdev, const char __user *buf,
			 size_t count, loff_t *size);
	long	(*ioctl)(struct vfio_device *vdev, unsigned int cmd,
			 unsigned long arg);
	int	(*mmap)(struct vfio_device *vdev, struct vm_area_struct *vma);
	void	(*request)(struct vfio_device *vdev, unsigned int count);
	int	(*match)(struct vfio_device *vdev, char *buf);
	void	(*dma_unmap)(struct vfio_device *vdev, u64 iova, u64 length);
	int	(*device_feature)(struct vfio_device *device, u32 flags,
				  void __user *arg, size_t argsz);
};


struct vfio_migration_ops {
	struct file *(*migration_set_state)(
		struct vfio_device *device,
		enum vfio_device_mig_state new_state);
	int (*migration_get_state)(struct vfio_device *device,
				   enum vfio_device_mig_state *curr_state);
};


static inline int vfio_check_feature(u32 flags, size_t argsz, u32 supported_ops,
				    size_t minsz)
{
	if ((flags & (VFIO_DEVICE_FEATURE_GET | VFIO_DEVICE_FEATURE_SET)) &
	    ~supported_ops)
		return -EINVAL;
	if (flags & VFIO_DEVICE_FEATURE_PROBE)
		return 0;
	
	if (!(flags & (VFIO_DEVICE_FEATURE_GET | VFIO_DEVICE_FEATURE_SET)))
		return -EINVAL;
	if (argsz < minsz)
		return -EINVAL;
	return 1;
}

void vfio_init_group_dev(struct vfio_device *device, struct device *dev,
			 const struct vfio_device_ops *ops);
void vfio_uninit_group_dev(struct vfio_device *device);
int vfio_register_group_dev(struct vfio_device *device);
int vfio_register_emulated_iommu_dev(struct vfio_device *device);
void vfio_unregister_group_dev(struct vfio_device *device);

int vfio_assign_device_set(struct vfio_device *device, void *set_id);

int vfio_mig_get_next_state(struct vfio_device *device,
			    enum vfio_device_mig_state cur_fsm,
			    enum vfio_device_mig_state new_fsm,
			    enum vfio_device_mig_state *next_fsm);


struct iommu_group *vfio_file_iommu_group(struct file *file);
bool vfio_file_enforced_coherent(struct file *file);
void vfio_file_set_kvm(struct file *file, struct kvm *kvm);
bool vfio_file_has_dev(struct file *file, struct vfio_device *device);

#define VFIO_PIN_PAGES_MAX_ENTRIES	(PAGE_SIZE/sizeof(unsigned long))

int vfio_pin_pages(struct vfio_device *device, dma_addr_t iova,
		   int npage, int prot, struct page **pages);
void vfio_unpin_pages(struct vfio_device *device, dma_addr_t iova, int npage);
int vfio_dma_rw(struct vfio_device *device, dma_addr_t iova,
		void *data, size_t len, bool write);


struct vfio_info_cap {
	struct vfio_info_cap_header *buf;
	size_t size;
};
struct vfio_info_cap_header *vfio_info_cap_add(struct vfio_info_cap *caps,
					       size_t size, u16 id,
					       u16 version);
void vfio_info_cap_shift(struct vfio_info_cap *caps, size_t offset);

int vfio_info_add_capability(struct vfio_info_cap *caps,
			     struct vfio_info_cap_header *cap, size_t size);

int vfio_set_irqs_validate_and_prepare(struct vfio_irq_set *hdr,
				       int num_irqs, int max_irq_type,
				       size_t *data_size);

struct pci_dev;
#if IS_ENABLED(CONFIG_VFIO_SPAPR_EEH)
void vfio_spapr_pci_eeh_open(struct pci_dev *pdev);
void vfio_spapr_pci_eeh_release(struct pci_dev *pdev);
long vfio_spapr_iommu_eeh_ioctl(struct iommu_group *group, unsigned int cmd,
				unsigned long arg);
#else
static inline void vfio_spapr_pci_eeh_open(struct pci_dev *pdev)
{
}

static inline void vfio_spapr_pci_eeh_release(struct pci_dev *pdev)
{
}

static inline long vfio_spapr_iommu_eeh_ioctl(struct iommu_group *group,
					      unsigned int cmd,
					      unsigned long arg)
{
	return -ENOTTY;
}
#endif 


struct virqfd {
	void			*opaque;
	struct eventfd_ctx	*eventfd;
	int			(*handler)(void *, void *);
	void			(*thread)(void *, void *);
	void			*data;
	struct work_struct	inject;
	wait_queue_entry_t		wait;
	poll_table		pt;
	struct work_struct	shutdown;
	struct virqfd		**pvirqfd;
};

int vfio_virqfd_enable(void *opaque, int (*handler)(void *, void *),
		       void (*thread)(void *, void *), void *data,
		       struct virqfd **pvirqfd, int fd);
void vfio_virqfd_disable(struct virqfd **pvirqfd);

#endif 
