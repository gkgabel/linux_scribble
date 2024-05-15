/* SPDX-License-Identifier: GPL-2.0 */


#ifndef LINUX_PCI_DOE_H
#define LINUX_PCI_DOE_H

struct pci_doe_protocol {
	u16 vid;
	u8 type;
};

struct pci_doe_mb;


struct pci_doe_task {
	struct pci_doe_protocol prot;
	u32 *request_pl;
	size_t request_pl_sz;
	u32 *response_pl;
	size_t response_pl_sz;
	int rv;
	void (*complete)(struct pci_doe_task *task);
	void *private;

	
	struct work_struct work;
	struct pci_doe_mb *doe_mb;
};


#define pci_doe_for_each_off(pdev, off) \
	for (off = pci_find_next_ext_capability(pdev, off, \
					PCI_EXT_CAP_ID_DOE); \
		off > 0; \
		off = pci_find_next_ext_capability(pdev, off, \
					PCI_EXT_CAP_ID_DOE))

struct pci_doe_mb *pcim_doe_create_mb(struct pci_dev *pdev, u16 cap_offset);
bool pci_doe_supports_prot(struct pci_doe_mb *doe_mb, u16 vid, u8 type);
int pci_doe_submit_task(struct pci_doe_mb *doe_mb, struct pci_doe_task *task);

#endif
