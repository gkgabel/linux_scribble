#include <../arch/x86/include/asm/tlbflush.h>
#include <../include/linux/migrate.h>
#include <linux/syscalls.h>
#include <../include/linux/rmap.h>
#include <linux/page_owner.h>
#include <linux/page_ext.h>
#include <linux/pagemap.h>
#include <../drivers/iommu/intel/iommu.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include "../mm/internal.h"
#include "../drivers/vfio/vfio.h"
#include <linux/tracepoint.h>

//The naming scheme “subsys_event” is suggested here as a convention intended to limit collisions. 
//Tracepoint names are global to the kernel: they are considered as being the same whether they are in the core kernel image or in modules.
#include <trace/events/migration.h>
#define CREATE_TRACE_POINTS
#include <trace/events/migration.h>

int tempctr=0;
/**
 * UPDATED: OCT 26,2024
 * This file defines a system call to migrate pinned pages and update the IOVA 
 * mapping in corresponding IOMMU. The system call takes a PID as an argument
 * and migrates all pinned pages of the process with that PID. If the PID is -1,
 * the system call migrates all pinned pages of the calling process. There are 
 * many functions here that are specific to Intel IOMMU and may not work with 
 * other IOMMUs. Also it is designed keeping onlyx86 architecture in mind.
 * This requires PAGE_EXT config to be enabled in the kernel and  and page_owner
 * feature to be enabled in boot parameters. Also other kernel code has been 
 * modified to facilitate this system call. In the page_owner struct, a new 
 * field dma_device has been added to store the dma_device of the pinned page.
 * Another field gup_flag has been added to mark a pinned page. In future, we
 * can probably use the refcount of the page (using GUP_COUNTING_BIAS) to know 
 * if a page is pinned but that is prone to false positive errors. We also have 
 * other fields added to page_owner struct to store the iommu_domain and iov_pfn
 */

#define ENABLE_DEVICE_PAGE_MIGRATION 1
/**
 * Translates a virtual address to a page table entry (PTE).
 *
 * This function performs a page table walk through the multi-level page table
 * of a process. It starts from the top level (Page Global Directory, PGD) and
 * goes down to the PTE level. At each level, it uses the virtual page number
 * to index into the current level's table and obtain a pointer to the next
 * level's table. If it encounters a missing or bad entry at any level, it
 * immediately returns NULL. If it successfully walks through all levels of
 * the page table, it returns a pointer to the PTE.
 *
 * @param mm Pointer to the memory management structure of the process.
 * @param vpage Virtual page number.
 * @return Pointer to the PTE corresponding to the virtual address, or NULL
 *         if the translation could not be completed (e.g., because of a
 *         missing or bad entry in the page table).
 */
pte_t *va_to_pte(struct mm_struct *mm,unsigned long vpage)
{
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	//translate va to pte
	pgd = pgd_offset(mm, vpage);
	if (pgd_none(*pgd) || pgd_bad(*pgd))return NULL;
	p4d = p4d_offset(pgd, vpage);
	if (p4d_none(*p4d) || p4d_bad(*p4d))return NULL;
	pud = pud_offset(p4d, vpage);
	if (pud_none(*pud) || pud_bad(*pud))return NULL;
	pmd = pmd_offset(pud, vpage);
	if (pmd_none(*pmd) || pmd_bad(*pmd))return NULL;
	pte = pte_offset_map(pmd, vpage);
	return pte;
}

void print_page_info(struct page *pg, char *msg)
{
	unsigned long pfn = page_to_pfn(pg);
	bool is_anon = (page_folio(pg)->mapping == NULL);
	printk(KERN_INFO "%s : Pfn  %lu : is_Anon = %d RefCount = %d Mapcount =  %d ", 
			msg, pfn, is_anon, atomic_read(&pg->_refcount), 
			atomic_read(&pg->_mapcount));
	
	//The following code prints the number of entries in page's anon vma 
	/* 
	struct anon_vma *anon_vma;
	struct anon_vma_chain *avc;
    struct vm_area_struct *vma;
	unsigned int n=0;
	anon_vma = folio_lock_anon_vma_read(page_folio(pg), NULL);
    if (anon_vma) {
        anon_vma_interval_tree_foreach(avc, &anon_vma->rb_root, pg->index, pg->index) {
        	vma = avc->vma;
        	n++;
    	}
		page_unlock_anon_vma_read(anon_vma);
	}
	printk(KERN_INFO "PFN %lu mapped %u times=\n",pfn,n);
	*/
}

#if ENABLE_DEVICE_PAGE_MIGRATION
/**
 * Check if a page is pinned using the flag in the page_ext. If pinned, update 
 * the address of its struct page_owner.
 * @param pg Pointer to the struct page of the page to check.
 * @param pg_owner Address of a pointer to store the struct page_owner of the
 * 			   pinned page.
 */
int is_page_pinned(struct page *pg,struct page_owner **pg_owner)
{
	struct page_ext *page_ext;
    struct page_owner *temp;
	page_ext = lookup_page_ext(pg);

	if(page_ext==NULL)
		return -1;
	
	temp = (void *)page_ext + page_owner_ops.offset;

	if(temp->flag_gup == 0)
		return 0;

	*pg_owner=temp;
	return 1;
}

/**
 * Find the the iova pte corresponding to a pinned page. We store the iov_pfn in
 * the page_owner struct of the page. This function uses the iov_pfn to find the
 * corresponding iova pte.
 */
struct dma_pte *find_page_iova_pte(struct page *pg)
{
	struct page_owner *pg_owner;
	struct dma_pte *dma_pte;
	if(is_page_pinned(pg,&pg_owner)<=0) 
	{
		printk(KERN_INFO "Page does not have page_ext or not pinned");
		return NULL;
	}

	/**
	 * ---HACK---
	 * The extra condtion to check iommu domain is a hack. I am not able to get
	 * the dma_device for vfio driver case and directly uing the domain
	 */
	if(pg_owner->dma_device==NULL && pg_owner->iommu_domain==NULL)
	{
		printk(KERN_INFO "Page is pinned; but DMA device and IOMMU domain NULL");
		return NULL;
	}
	//can we use iommu_iova_to_phys() . no because that is not that something we are looking fr here. we want io pte.
	dma_pte = find_iova_pte(pg_owner->iov_pfn,pg_owner->dma_device,pg_owner->iommu_domain);
	gfp_t gfp_mask;
	int mt;
	gfp_mask = pg_owner->gfp_mask;
	mt = gfp_migratetype(gfp_mask);
	printk(KERN_INFO "IOV pfn %lu mapped to phys pfn %lu migratetype is %s",pg_owner->iov_pfn,page_to_pfn(pg),migratetype_names[mt]);
	trace_migrate_event(page_to_pfn(pg), "source");
	//printk("--------dma pte------%llu\n",dma_pte->val);
	return dma_pte;
}
#endif

/**
 * Core of the silent migration system call.
 * Function to migrate a pinned page to a new destination chosen arbitrarily
 * and update the IOVA mapping. This function is Intel IOMMU specific.
 * Ensure that dst page is allocated and its refcount is incremented before 
 * calling this function
 * @param dma_pte Pointer to the dma_pte of the pinned page.
 * @param pg_owner Pointer to the page_owner struct of the pinned page.
 * @param dst Pointer to the destination page.
 * @param src Pointer to the source page.
 * 
 * @return 0 on success, 1 on failure.
 */
int migrate_and_update_iova_mapping(struct dma_pte *dma_pte, 
									struct page *dst,struct page *src)
{	
#if ENABLE_DEVICE_PAGE_MIGRATION
	struct intel_iommu *iommu;
	u8 bus, devfn;
	//u64 clr_access_mask = ~(1ULL << DMA_FL_PTE_ACCESS); this is wrong because DMA_FL_PTE_ACCESS is already a mask which  has access bit set.
	u64 clr_access_mask = ~(DMA_FL_PTE_ACCESS)-1;
	unsigned long dest_pfn_val = page_to_pfn(dst);
	//Intel IOMMU specific structures
	struct dma_pte dma_pte_new, dma_pte_access_unset;
	u64 dma_pte_flag;
	struct page_owner *pg_owner;

	is_page_pinned(src,&pg_owner);
	/**
	 * Intel IOMMU Specific
	 * Since PTE contains page addresses that are address aligned to page size,
	 * the page offset bits VTD_PAGE_SHIFT bits are used for flag; 
	*/
	//MSB bit apart from mask is some special flag so that is also masked out, 
	//above this line does not make much sense to me so i have removed the mask  (~DMA_FL_PTE_XD)
	//but this can go very wrong so need to check again
	dma_pte_flag = dma_pte->val & ~VTD_PAGE_MASK;
	dma_pte_new.val = dma_pte_flag | (dest_pfn_val << VTD_PAGE_SHIFT);
	unsigned long src_pfn_from_pte = (dma_pte->val & VTD_PAGE_MASK)>>VTD_PAGE_SHIFT;
	unsigned long dst_pfn_from_pte = (dma_pte_new.val & VTD_PAGE_MASK)>>VTD_PAGE_SHIFT;
	printk(KERN_INFO "Source page pfn from pte %lu and dest page pfn from pte %lu",src_pfn_from_pte,dst_pfn_from_pte);
	/**
	 * Pinned Page Migration Step 1: clearing access bit for dma pte. shouldn't 
	 * it be an atomic operator?
	 */
	dma_pte->val = dma_pte->val & clr_access_mask;
	printk(KERN_INFO "clr access mask %llu and dma_pte->val  %llu",clr_access_mask,dma_pte->val);

	//making a copy to compare later
	dma_pte_access_unset.val = dma_pte->val & clr_access_mask;

	/**
	 * Pinned Page Migration Step 2: Shootdown
	 * IOTLB invalidation in intel iommu can be done at different granularities:
	 * DMA_TLB_GLOBAL_FLUSH, DMA_TLB_DSI_FLUSH (domain selective invalidation),
	 * DMA_TLB_PSI_FLUSH (page selective invalidation). Naively, we are going 
	 * with DMA_TLB_GLOBAL_FLUSH. Internally, the Intel tlb flushing logic calls
	 * the function __iommu_flush_iotlb() to flush the IOTLB. We set did=0 if 
	 * the domain flushing function is not present in the hardware. As of now,
	 * I think that is what I read about did parameter. Need to check again.
	 * It is an Intel IOMMU specific function as it uses the intel_iommu struct.
	 * 
	 * The else part is for the case when we are using iommu_domain instead of
	 * dma_device. This is a hack and needs to be fixed. But one advatahe here
	 * it is IOMMU agnostic. This might not be true. Need to check again.
	 */
	if(pg_owner->dma_device!=NULL)
	{
		iommu = device_to_iommu(pg_owner->dma_device, &bus, &devfn);
		if(iommu==NULL)
		{
			printk(KERN_WARNING "IOMMU struct NULL, skipping\n");
			return 1;
		}
		/**
		 * Doubt here: third last parametr is address but since it is global 
		 * invalidation putting a zero. What if it was psi, what address would
		 * go here.
		 */
		printk(KERN_INFO "IOTLB Flush\n");
		iommu->flush.flush_iotlb(iommu,0,0,0,DMA_TLB_GLOBAL_FLUSH); 
	}
	else if(pg_owner->iommu_domain!=NULL)
	{
		struct iommu_domain *domain = pg_owner->iommu_domain;
		printk(KERN_INFO "IOTLB Flush\n");
		domain->ops->flush_iotlb_all(domain);
	}
	else
	{
		printk(KERN_WARNING "Both IOMMU struct and domain are NULL, skipping\n");
		return 1;
	}
#endif
			
	/**
	 * Pinned Page Migration Step 3: Copy
	 */
	//migrate_folio(mapping, page_folio(dst), page_folio(src), MIGRATE_SYNC);
	folio_migrate_copy(page_folio(dst), page_folio(src));

	/**
	 * Pinned Page Migration Step 4: Compare access bit and Xchange  PTE
	 */

	/**
	 * We assume that the src page has only one userspace mapping. If the mapcount
	 * of the src page is more than 1, we should be handle that case, but for
	 * simplicity we assume single mapping. Shoul we do it, won't it be handled
	 * inside functions.
	 */
	//do i need to do this? why? migration od userspace mappinf already rtakes care of this in my guess
	//printk("Incrementing mapcount and refcount of dst page\n");
	//atomic_inc(&dst->_mapcount);
	//atomic_inc(&dst->_refcount);

#if ENABLE_DEVICE_PAGE_MIGRATION
	printk(KERN_INFO "Replacing the IO PTE");
	printk("dma pte val %llu\n",dma_pte->val);
	printk("before updatingnew pfn from pte %llu\n",(dma_pte->val & VTD_PAGE_MASK)>>VTD_PAGE_SHIFT);

	printk("original phys add before migration %llu and one that we stored %llu\n",iommu_iova_to_phys(pg_owner->iommu_domain,pg_owner->iov_pfn<<VTD_PAGE_SHIFT),pg_owner->phys_pfn);
	/**
	 * This needs to be given proper attention, can be source of error
	 */
	//a lot of chance this might be wrong
	if(dma_pte_access_unset.val != 
		cmpxchg(&(dma_pte->val), dma_pte_access_unset.val, dma_pte_new.val))
	{
		//page_ref_add(dst, GUP_PIN_COUNTING_BIAS);
		//atomic_sub(GUP_PIN_COUNTING_BIAS, &dst->_refcount);
		//atomic_dec(&dst->_mapcount);
		//atomic_dec(&dst->_refcount);
		return 1;
	}
	else
	{
		atomic_sub(GUP_PIN_COUNTING_BIAS, &src->_refcount);
		page_ref_add(dst, GUP_PIN_COUNTING_BIAS);
		//atomic_dec(&src->_mapcount);
	}
	printk("dma pte val %llu\n",dma_pte->val);
	printk("after updatingnew pfn from pte %llu\n",(dma_pte->val & VTD_PAGE_MASK)>>VTD_PAGE_SHIFT);
	if(find_page_iova_pte(src))
	{
		printk(KERN_WARNING "Source page still has IOVA PTE with dest page %llu\n",iommu_iova_to_phys(pg_owner->iommu_domain,pg_owner->iov_pfn<<VTD_PAGE_SHIFT));
	}
	printk(KERN_INFO "IOV pfn %lu remapped to phys pfn %lu in device %lu but only in IO page table",
			pg_owner->iov_pfn,page_to_pfn(dst),(unsigned long)pg_owner->dma_device);
	trace_migrate_event(page_to_pfn(dst), "destination");

	return 0;
#else
	return 0;
#endif
}

/**
 * The below code has been simply copied from folio_migrate_mapping()
 * function. Apart from copying the contents of a page, we also need to
 * copy the relevant metadata for the page in the respective kernel
 * address space they are mapped to. In case of issues, we can refer to
 * the function folio_migrate_mapping() in mm/migrate.c
 * Copied this comment for reference.
 * The number of remaining references must be:
 * 1 for anonymous folios without a mapping
 * 2 for folios with a mapping
 * 3 for folios with a mapping and PagePrivate/PagePrivate2 set.
 */
void copy_mapping(struct address_space *mapping, struct folio *fdst, 
					struct folio *fsrc)
{
	XA_STATE(xas, &mapping->i_pages, folio_index(fsrc));
	if(!mapping)
	{
		fdst->mapping = fsrc->mapping;
		fdst->index = fsrc->index;
		return;
	}

	//long nr = folio_nr_pages(fsrc);
	//int expected_count = 1+nr;

	fdst->mapping = fsrc->mapping;
	fdst->index = fsrc->index;

	xas_lock_irq(&xas);
	/**
	 * Had to commente this because this will never succeed if page is pinned
	 * but it means we are leaving some ref counts unhandled
	 */
	
	/*
	if (!folio_ref_freeze(page_folio(src), expected_count)) 
	{
		xas_unlock_irq(&xas);
		return -EAGAIN;
	}
	*/
	
	/**
	 * Many cases have been left behind here. First need to understand the 
	 * function folio_migrate_mapping() properly and then put it.
	 */
	/*
    if (folio_test_swapbacked(page_folio(src))) {
		__folio_set_swapbacked(page_folio(dst));
		if (folio_test_swapcache(page_folio(src))) {
			folio_set_swapcache(page_folio(dst));
			page_folio(dst)->private = folio_get_private(page_folio(src));
		}
	} else {
		VM_BUG_ON_FOLIO(folio_test_swapcache(page_folio(src)), folio);
	}
	*/
	xas_store(&xas, fdst);
	
	/**
	 * Commented because did not understand the code
	 */
	/*
	folio_ref_unfreeze(page_folio(dst), expected_count - nr);
	*/
	xas_unlock(&xas);
	local_irq_enable();
	
		
	//folio_ref_add(fdst, nr); /* add cache reference */
	/*Also update in vfio_iommu_type1 driver*/
	struct page_owner *pg_owner;
	if(is_page_pinned(folio_page(fsrc,0),&pg_owner)>0)
	{
		struct vfio_iommu *iommu;
		dma_addr_t iova;

		iommu= pg_owner->vfio_iommu;
		iova = pg_owner->iov_pfn << VTD_PAGE_SHIFT;
		
		vfio_change_mapping(iommu,iova,page_to_pfn(folio_page(fsrc,0)),page_to_pfn(folio_page(fdst,0)));
	}
}

/**
 * This function allocates a new page and migrates the pinned page to the new
 * page.
 */
//the second argument pte is only for checking, so it can removed later
int migrate_arbitrary(struct page *src, struct page *dst, pte_t *pte)
{
	//struct page *dst;
	struct address_space *mapping;
	//struct migration_target_control mtc;
	swp_entry_t swap_entry;
	struct dma_pte *dma_pte;

	//these 4 lines were here to test if it fails alloc . remove it. also it failed
	/*
	mtc.nid = NUMA_NO_NODE;
	mtc.gfp_mask = GFP_HIGHUSER_MOVABLE;
	dst = alloc_migration_target(src, (unsigned long)&mtc);
	print_page_info(dst,"Destination Page, Freshly allocated");
	

	print_page_info(src,"Source Page");
*/
	dma_pte = find_page_iova_pte(src);
	if(dma_pte == NULL)
	{
		printk(KERN_WARNING "PFN %lu: Pinned page but DMA Pte is NUll\n",
				page_to_pfn(src));
		return 1;
	}
	printk(KERN_INFO "PFN %lu: Pinned page found with DMA Pte %lu\n",
				page_to_pfn(src),dma_pte->val);

	/**
	 * folio_mapping(page_folio(src)) does not give the mapping of the page that
	 * is anon, it returns NULL if PAGEFLAG ANON OR MOVABLE is set. SSo we use
	 * the below function instead.
	 */
	mapping = page_folio(src)->mapping; //use this instead of the one before
	if(mapping == NULL){
				printk(KERN_WARNING "Mapping is NULL for the src page\n");
	}
	if (folio_test_anon(page_folio(src))) {
		printk(KERN_INFO "src folio is an anonymous folio\n");
	}
	else{
		printk(KERN_INFO "src folio is not an anonymous folio\n");
	}
	//this did not work out well, an issu with kernel crashing while allocating page moved it to where this function is called
	/**
	 * Since we are working with user space pages we should not use GFP_KERNEL flag
	 * dst = alloc_page(GFP_KERNEL);
	 * Rather, using something directly from migration code because need 
	 * to copy source permissions and prpperties for folios also.
	 * This can cause a lot of issues.
	 * Went the way of kernel inbuilt. 
	 * The migration_target_control struct has been copied from gup.c
	 * However need to check if GFP_HIGHUSER_MOVABLE is the right flag
	 * for pinning pages. Some issues are mentioned in documentation.
	 */
	//mapping = folio_mapping(page_folio(src));
	/*mtc.nid = NUMA_NO_NODE;
	mtc.gfp_mask = GFP_HIGHUSER_MOVABLE;
	dst = alloc_migration_target(src, (unsigned long)&mtc);
	
	print_page_info(dst,"Destination Page, Freshly allocated");
	*/
	copy_mapping(mapping,page_folio(dst),page_folio(src));
	if(page_folio(dst)->mapping != page_folio(src)->mapping)
	{
		printk(KERN_WARNING "Source and Destination folio mapppings don't match");
	}
	
	/**
	 * It is important to hold a reference to both src and dst page before 
	 * starting the migration.
	 */
	print_page_info(src,"Source Page: Before blocking user access and changing ref count:");
	//the above fact dpes not seem to be true cpmpletely. Need to check again.Probably
	//won't need to hold a reference for destination also source if it is pinned already
	atomic_inc(&dst->_refcount);
	atomic_inc(&src->_refcount);

	/**
	 * Idea of user space page blocking
	 * 1. Increment the refcount of the src page
	 * 2. Lock the page
	 * 3. Call try_to_migrate() to unmap the page from user va spaces
	 * 4. After IOVA mapping is updated, decrement the refcount of src page and remove migration ptes and update new ptes
	 */

	/*-------------Part of user page blocking for facilitating iova mapping change.-------------------*/
	
	if(!page_mapped(src))
	{
		printk(KERN_WARNING "Page is not mapped to user space, so skipping\n");
		return 1;
	}
	/**
	 * Likely a lot of errors since it has been taken from kernel without proper
	 * regard for different corner cases. This is taken from funtion 
	 * __unmap_and_move() in mm/migrate.c
	 */
	if (!trylock_page(src)) {
		printk(KERN_INFO "Page has not been locked so we attempt to get a lock\n");
		lock_page(src);
		if (!trylock_page(src)) 
		{
			printk(KERN_WARNING "Page could not be locked, skipping\n");
			return 1;
		}
		if(!PageLocked(src))
		{
			printk(KERN_WARNING "src Page is not locked\n");
			return 1;
		}
	}

	/**
	 * Do we need to lock the destination page. I have only done this because at the current moment 
	 * while writing this, I have been getting error related to PageLocked() being false for a migration entry
	 * but it was of now use. I am still keepng it here because when removing the migration ptes 
	 * I saw that the pages involved are unlocked later
	 */
	if (!trylock_page(dst)) {
		printk(KERN_INFO "dst Page has not been locked so we attempt to get a lock\n");
		lock_page(dst);
		if (!trylock_page(dst)) 
		{
			printk(KERN_WARNING "dst Page could not be locked, skipping\n");
			return 1;
		}
		if(!PageLocked(dst))
		{
			printk(KERN_WARNING "dst Page is not locked\n");
			return 1;
		}
	}

	/**
	 * Lock acquired so try to block user va access
	*/
	print_page_info(src,"Source Page: Before blocking user access:");
	try_to_migrate(page_folio(src), 0);
	print_page_info(src,"Source Page: After blocking user access:");

	swap_entry=pte_to_swp_entry(*pte);
	if(is_migration_entry(swap_entry))
	{
		printk(KERN_INFO "Migration entry found in src PTE\n");			
	}
	else
	{
		printk(KERN_WARNING "Migration entry not found in src PTE\n");
		return 1;
	}

	/**
	 * If IOVA migration was successful, we remove the migration ptes and update
	 * with the new pte. Else we restore like below. Currently 
	 * migrate_and_update_iova_mapping() returns 0 on success and 1 on failure. 
	 */
	if(migrate_and_update_iova_mapping(dma_pte,dst,src))
	{
		remove_migration_ptes(page_folio(src),page_folio(src),false);
		unlock_page(src);
		unlock_page(dst);
		
		//instead of decrementing ref count doing put page to do the same
		//atomic_dec(&src->_refcount);
		put_page(dst);
		put_page(src);
		put_page(dst);

		//this is weird because page ownewr should bw cleared with free pages but we are not doing thsat
		reset_page_owner(dst, 0);
		//we are fqcing issue that folio is still mapped to user space. So, let us check if it somehingfrom oue side casuing the poblem
		if(folio_mapped(page_folio(dst)))
		{
			printk(KERN_WARNING "Destination page is still mapped to user space\n");
		}

		print_page_info(src,"Source Page: After migration failure:");
		print_page_info(dst,"Destination Page: After migration failure:");
		
		/**
		 * ---HACK---
		 * This must be done. we have to free the dst page. But we are
		 * not doing it right now because of the dangerous null ptr deref
		 * This is a hack and should be fixed. But do we need to free pages,
		 * they should be freed if ref count goes to 0?
		 */
		//__free_pages(dst,0); 
		return 1;
	}

	remove_migration_ptes(page_folio(src),page_folio(dst),false);
	if(src==pte_page(*pte))
	{
		printk(KERN_WARNING "Destination page not updated in pte, skipping\n");
		return 1;
	}

	unlock_page(src);
	unlock_page(dst);
	/**
	 * instead of decrementing refcount of src page, we use put_page(), 
	 * if refcount becomes 0 it automatically frees them
	 */
	put_page(src);
	put_page(dst);
	put_page(src);
	//this is weird because page ownewr should bw cleared with free pages but we are not doing thsat
	reset_page_owner(src, 0);
	//we are fqcing issue that folio is still mapped to user space. So, let us check if it somehingfrom oue side casuing the poblem
	if(folio_mapped(page_folio(src)))
	{
		printk(KERN_WARNING "source page is still mapped to user space\n");
		tempctr++;
	}


	//atomic_dec(&src->_refcount);
	print_page_info(src,"Source Page: After migration success:");
	print_page_info(dst,"Destination Page: After migration success:");

	/**
	 * ---HACK---
	 * This must be done. we have to free the src page. But we are 
	 * not doing it right now because of the dangerous null ptr deref. But see 
	 * above comment about put_page, it should be freed if ref count goes to 0
	 * automatically.
	 */
	//__free_pages(src,0); 		
	return 0;
}

/**
 * System call to migrate pinned pages of a process.
 */
SYSCALL_DEFINE1(silent_migrate,pid_t,pid)
{
	struct mm_struct *mm; //mm_struct of pid whose pages are to be migrated
	struct vm_area_struct *vmi; //iterator for VMA
	unsigned long abort = 0, success = 0, mismatch = 0; //stats counters 

	if(pid == -1)
		mm = current->mm;
	else
	{
		mm = get_pid_task(find_get_pid(pid),PIDTYPE_PID)->mm;
		if(mm==NULL)
		{
			printk(KERN_ERR "mm struct for process is NULL\n");
			return -1;
		}
	}

	//this flag is for pid conditoned printk debugging at places in kernel
	//custom_printk_flag= get_current()->pid;
	unsigned long page_counter=0;
	for(vmi = mm->mmap; vmi != NULL ; vmi = vmi->vm_next)
	{	
		unsigned long vpage; //temp variable for iterating over each virtual page in vma
		printk(KERN_INFO "vma_start %lu vmi_end%lu \n",vmi->vm_start,vmi->vm_end);
		
		/**
		 * Uncomment this if only anonymous pages are concerned. 
		 * Remember sometimes applications can pin pages that are mapped to a 
		 * virtual file. So, it is better to keep this commented.
		 */
		//if(!vma_is_anonymous(vmi))
		//	continue;

		/**
		 * Uncomment this if only stack pages are to be skipped
		 */
		/*
		if(mm->start_stack >= vmi->vm_start && mm->start_stack <= vmi->vm_end);
			continue;
		*/

		//Iterate over the vma with base page size
		for( vpage = vmi->vm_start ; vpage < vmi->vm_end ; vpage += 4096 )
		{	
			//char *vaddr_src, *vaddr_dst;
			pte_t *pte, temp_pte_val;
			struct page *src;
			struct page_owner *pg_owner;
			


			//printk(KERN_INFO "vpage addr = %lu\n", vpage);
			pte = va_to_pte(mm,vpage);
			temp_pte_val = *pte;
			if(pte==NULL)
				continue;

			if (!pte_present(*pte))
				continue;

			/**
			 * ---HACK---
			 * Currently, we are not handling special PTEs.  
			 * See memory.c -> vm_normal_page() for more info on "Special" PTE
			 */
			if(pte_special(*pte))
				continue;

			src = pte_page(*pte);
			if(src==NULL)
			{
				printk(KERN_INFO "Could get a valid page_struct from PTE\n");
				continue;
			}
			
			/**
			 * ---HACK---
			 * PagePrivate() probably means that we do not have a struct page 
			 * for a physical page, so skip those for now
			 */
			if(PagePrivate(src))
				continue;

#if ENABLE_DEVICE_PAGE_MIGRATION
			if(!is_page_pinned(src,&pg_owner))
			{
				//printk(KERN_INFO "Skipping a unpinned page\n");
				continue;
			}
			page_counter++;
			printk(KERN_INFO "page counter = %lu\n", page_counter);
			//printk(KERN_INFO "It is a pinned page\n");
			
#endif
//these 4 lines here to see if we fail alloc, here we don't but inside the fun we do???????
/*			struct migration_target_control mtc;
			mtc.nid = NUMA_NO_NODE;
			mtc.gfp_mask = GFP_HIGHUSER_MOVABLE; //HIGHUSER MOVABLE MIGHT NOT BE THE RIGHT FLAG FOR PINN PAGES SEE THE DOCUMENTATIONit points out the issue
			struct page *dst = alloc_migration_target(src, (unsigned long)&mtc);
*/			
			//this did not work out well in side migrate_arbitrary() function, an issue with kernel crashing while allocating page moved it to where this function is called
	/**
	 * Since we are working with user space pages we should not use GFP_KERNEL flag
	 * dst = alloc_page(GFP_KERNEL);
	 * Rather, using something directly from migration code because need 
	 * to copy source permissions and prpperties for folios also.
	 * This can cause a lot of issues.
	 * Went the way of kernel inbuilt. 
	 * The migration_target_control struct has been copied from gup.c
	 * However need to check if GFP_HIGHUSER_MOVABLE is the right flag
	 * for pinning pages. Some issues are mentioned in documentation.
	 */
	//mapping = folio_mapping(page_folio(src));
	struct page *dst;
	struct migration_target_control mtc;

	mtc.nid = NUMA_NO_NODE;
	mtc.gfp_mask = GFP_HIGHUSER_MOVABLE;
	//got a kernel panic du to below. because did not set nmask
	mtc.nmask = NULL;
	dst = alloc_migration_target(src, (unsigned long)&mtc);
	
	print_page_info(dst,"Destination Page, Freshly allocated");
	
			if(!migrate_arbitrary(src,dst,pte))
			{
				success++;
			}
			else
			{
				abort++;
			}	
		}
	}
	printk(KERN_INFO "Number of Total & Successful & Aborted Attempts &tempctr= %lu %lu %lu %lu %lu\n",
			success+abort,success,abort,mismatch,tempctr);
	//custom_printk_flag = pid;
	return 0;
}
