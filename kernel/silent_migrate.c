#include <../arch/x86/include/asm/tlbflush.h>
#include <../include/linux/migrate.h>
#include <linux/syscalls.h>
#include <../include/linux/rmap.h>
#include <linux/page_owner.h>
#include <linux/page_ext.h>
#include <linux/pagemap.h>
#include <../drivers/iommu/intel/iommu.h>

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

/**
 * Check if a page is pinned using the flag in the page_ext. If pinned, update 
 * the address of its struct page_owner.
 * @param pg Pointer to the struct page of the page to check.
 * @param pg_owner Address of a pointer to store the struct page_owner of the
 * 			   pinned page.
 */

bool is_page_pinned(struct page *pg,struct page_owner **pg_owner)
{
	struct page_ext *page_ext;
    struct page_owner *temp;
	page_ext = lookup_page_ext(pg);

	if(page_ext==NULL)
	{
		printk(KERN_WARNING "Page extension is null for struct page * = %lu\n",
				(unsigned long)pg);
		return false;
	}
	temp = (void *)page_ext + page_owner_ops.offset;

	if(temp->flag_gup == 0)
		return false;

	*pg_owner=temp;
	return true;
}

void print_page_info(struct page *pg)
{
	struct anon_vma *anon_vma;
	struct anon_vma_chain *avc;
    struct vm_area_struct *vma;
	unsigned int n=0;
	unsigned long pfn = page_to_pfn(pg);
	printk(KERN_INFO "page_count = %d and mapcount =  %d for pfn  %lu",atomic_read(&pg->_refcount),atomic_read(&pg->_mapcount),pfn);
	printk(KERN_INFO "Page pcp list=%lu",pg->pcp_list);
	anon_vma = folio_lock_anon_vma_read(page_folio(pg), NULL);
    if (anon_vma) {
        //anon_vma_lock_read(anon_vma);
        anon_vma_interval_tree_foreach(avc, &anon_vma->rb_root, pg->index, pg->index) {
        	vma = avc->vma;
        	n++;
    	}
		page_unlock_anon_vma_read(anon_vma);
	}
	printk(KERN_INFO "PFN %lu mapped %u times=\n",pfn,n);
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
	if(!is_page_pinned(pg,&pg_owner)) 
	{
		printk(KERN_INFO "Page is not pinned; no IOVA info");
		return NULL;
	}
	/**
	 * ---HACK---
	 * The extra condtion to check iommu domain is a hack. I am not able to get
	 * the dma_device for vfio driver case and directly uing the domain
	 */
	if(pg_owner->dma_device==NULL && pg_owner->iommu_domain==NULL)
	{
		printk(KERN_INFO "Page is pinned; but NULL DMA device and iommu domain");
		return NULL;
	}
	dma_pte = find_iova_pte(pg_owner->iov_pfn,pg_owner->dma_device,pg_owner->iommu_domain);
	printk(KERN_INFO "iov pfn %lu mapped to phys pfn %lu",pg_owner->iov_pfn,page_to_pfn(pg));
	//printk("--------dma pte------%llu\n",dma_pte->val);
	return dma_pte;
}

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
 * @return 1 on success, 0 on failure. However, the return value is not
 * consistent with the convention of returning 0 on success and 1 on failure.
 */
int migrate_and_update_iova_mapping(struct dma_pte *dma_pte, 
									struct page_owner *pg_owner, 
									struct page *dst,struct page *src)
{	
	struct intel_iommu *iommu;
	u8 bus, devfn;
	u64 clr_access_mask = ~(1ULL << DMA_FL_PTE_ACCESS);
	unsigned long dest_pfn_val = page_to_pfn(dst);
	//Intel IOMMU specific structures
	struct dma_pte dma_pte_new, dma_pte_access_unset;
	u64 dma_pte_flag;
	
	/**
	 * Intel IOMMU Specific
	 * Since PTE contains page addresses that are address aligned to page size,
	 * the page offset bits VTD_PAGE_SHIFT bits are used for flag; 
	 * MSB bit apart from mask is some special flag so that is also masked out
	*/
	dma_pte_flag = dma_pte->val & VTD_PAGE_MASK & (~DMA_FL_PTE_XD);
	dma_pte_new.val = dma_pte_flag | (dest_pfn_val << VTD_PAGE_SHIFT);
	
	/**
	 * Pinned Page Migration Step 1: clearing access bit for dma pte
	 */
	dma_pte->val = dma_pte->val & clr_access_mask;
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
	 * it is IOMMU agnostic.
	 */
	if(pg_owner->dma_device!=NULL)
	{
		iommu = device_to_iommu(pg_owner->dma_device, &bus, &devfn);
		if(iommu==NULL)
		{
			printk(KERN_WARNING "IOMMU struct is NULL, no migration, returning\n");
			return 0;
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
		printk(KERN_WARNING "Both IOMMU struct and domain are NULL", 
				"no migration, returning\n");
		return 0;
	}
	
			
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
	printk("incrementing mapcount and refcount of dst page\n");
	atomic_inc(&dst->_mapcount);
	//atomic_inc(&dst->_refcount);
	printk("updated mapcount and refcount of dst page %d %d\n",
			atomic_read(&dst->_mapcount),atomic_read(&dst->_refcount));
	page_ref_add(dst, GUP_PIN_COUNTING_BIAS);
	printk(KERN_INFO "updating pte with new pfn based on condition");
	cmpxchg(&(dma_pte->val), dma_pte_access_unset.val, dma_pte_new.val);
	if(dma_pte->val != dma_pte_new.val)//recheck this condition
	{
		atomic_sub(GUP_PIN_COUNTING_BIAS, &dst->_refcount);
		//atomic_dec(&dst->_mapcount);
		//atomic_dec(&dst->_refcount);
		return 0;
	}
	else
	{
		atomic_sub(GUP_PIN_COUNTING_BIAS, &src->_refcount);
		//atomic_dec(&src->_mapcount);
	}
	return 1;
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

	custom_printk_flag= get_current()->pid;

	printk(KERN_INFO "Iterating on each vma in process VA space\n");
	for(vmi = mm->mmap; vmi != NULL ; vmi = vmi->vm_next)
	{	
		unsigned long vpage; //temp variable for iterating over each virtual page in vma
		printk(KERN_INFO "vma_start %lu vmi_end%lu \n",vmi->vm_start,vmi->vm_end);
		
		/**
		 * Uncomment this if only anonymous pages are concerned. 
		 * Remember sometimes applications can pin pages that are mapped to a 
		 * virtual file. So, it is better to keep this commented.
		 */
		/*
		if(!vma_is_anonymous(vmi))
			continue;
		*/

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
			pte_t *pte;
			struct page *src, *dst;
			struct page_owner *pg_owner;
			struct address_space *mapping;
			struct dma_pte *dma_pte;

			//printk(KERN_INFO "vpage addr = %lu\n", vpage);
			pte = va_to_pte(mm,vpage);
			
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

			if(!is_page_pinned(src,&pg_owner))
			{
				//printk(KERN_INFO "Skipping a unpinned page\n");
				continue;
			}
			
			//custom_printk_flag = 1; //this flag is for custom printk sometimes used in kernel, remove it after debugging
			//dst = alloc_page(GFP_KERNEL);
			//custom_printk_flag = 0; //this flag is for custom printk sometimes used in kernel, remove it after debugging
			printk(KERN_INFO "pte = %lu\n", pte->pte);
			printk(KERN_INFO "It is a pinned page\n");

			print_page_info(src);
			//custom_printk_flag = 1; //this flag is for custom printk sometimes used in kernel, remove it after debugging
			//dst = alloc_page(GFP_KERNEL);
			//custom_printk_flag = 0; //this flag is for custom printk sometimes used in kernel, remove it after debugging
			dma_pte = find_page_iova_pte(src);
			if(dma_pte == NULL)
			{
				printk(KERN_WARNING "DMA Pte is NUll for the pinned page\n");
				continue;
			}
			printk("line 319-----");
			//custom_printk_flag = 1; //this flag is for custom printk sometimes used in kernel, remove it after debugging
			
			//Here since we are working with user space pages we should not use GFP_KERNEL flag
			//dst = alloc_page(GFP_KERNEL);
			dst = alloc_page(GFP_HIGHUSER_MOVABLE);
			printk("mapcount and refcount of dst page %d %d\n",
					atomic_read(&dst->_mapcount),atomic_read(&dst->_refcount));
			//custom_printk_flag = 0; //this flag is for custom printk sometimes used in kernel, remove it after debugging
			printk("line 321-----");
			mapping = folio_mapping(page_folio(src));
			printk("line 323-----");
			
			/*
			It is important to hold a reference before starting the migration which we already have for src page,
			but not for dst page so we increment the refcount of dst page
			*/
			atomic_inc(&dst->_refcount);
			printk("line 330-----");
			/*
			Idea of user space page blocking
			1. Increment the refcount of the src page
			2. Lock the page
			3. Call try_to_migrate() to unmap the page from user va spaces
			4. After IOVA mapping is updated, decrement the refcount of src page and remove migration ptes and update new ptes
			*/
			
			/*-------------Part of user page blocking for facilitating iova mapping change.-------------------*/

			atomic_inc(&src->_refcount);
			/*
			Likely a lot of errors since it has been taken from Linux code without proper regard for different cases
			this is taken from line 989 of funtion __unmap_and_move() in mm/migrate.c
			*/
			printk("line 346-----");	
			if(!page_mapped(src))
			{
				printk(KERN_WARNING "Page is not mapped to user space, so skipping\n");
				continue;
			}
			printk("line 352-----");
			if (!trylock_page(src)) {
				printk(KERN_INFO "Page has not been locked so we attempt to get a lock\n");
				lock_page(src);
				if (!trylock_page(src)) 
				{
					printk(KERN_WARNING "Page could not be locked and hence skipping\n");
					continue;
				}
				if(!PageLocked(src))
				{
					printk(KERN_WARNING "src Page is not locked\n");
					continue;
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
					printk(KERN_WARNING "dst Page could not be locked and hence skipping\n");
					continue;
				}
				if(!PageLocked(dst))
				{
					printk(KERN_WARNING "dst Page is not locked\n");
					continue;
				}
			}
			printk("line 362-----");
			/*
			Lock acquired so try to block user va access
			*/
			try_to_migrate(page_folio(src), 0);
			/*--------------------------------------------------------------------------------------------------*/
			printk("line 368");

			if(migrate_and_update_iova_mapping(dma_pte,pg_owner,dst,src))
			{
				/*-------------Part of user page blocking for facilitating iova mapping change.-------------------*/
				printk("line 372-----");
				remove_migration_ptes(page_folio(src),page_folio(dst),false);
				printk("line 374-----");
				unlock_page(src);
				unlock_page(dst);
				printk("line 376-----");
				/**
				 * instead of decrementing refcount of src page, we use put_page(), if refcount becomes 0 it automaticlly frees them
				*/
				put_page(src);
				//atomic_dec(&src->_refcount);
				printk("src refcount and mapcount on success %d %d\n",atomic_read(&src->_refcount),atomic_read(&src->_mapcount));
				printk("dst refcount and mapcount on success %d %d\n",atomic_read(&dst->_refcount),atomic_read(&dst->_mapcount));
				/**
				 * ---HACK---
				 * This must be done. we have to free the src page. But we are 
				 * not doing it right now because of the dangerous null ptr deref
				 */
				//__free_pages(src,0); 
				printk("line 378----");
				/*--------------------------------------------------------------------------------------------------*/
				success++;
				printk(KERN_INFO "iov pfn %lu remapped to phys pfn %lu in device %lu",pg_owner->iov_pfn,page_to_pfn(dst),(unsigned long)pg_owner->dma_device);
				print_page_info(dst);
			}
			else
			{
				/*-------------Part of user page blocking for facilitating iova mapping change.-------------------*/
				printk("line 387-----");
				remove_migration_ptes(page_folio(src),page_folio(src),false);
				//__SetPageLocked(src);
				printk("line 389-----");
				unlock_page(src);
				unlock_page(dst);
				printk("line 391-----");
				//instead of decrementing ref count doing put page to do the same
				put_page(src);
				//atomic_dec(&src->_refcount);
				printk("src refcount and mapcount on failure %d %d\n",atomic_read(&src->_refcount),atomic_read(&src->_mapcount));
				printk("dst refcount and mapcount on failure %d %d\n",atomic_read(&dst->_refcount),atomic_read(&dst->_mapcount));
				/**
				 * ---HACK---
				 * This must be done. we have to free the dst page. But we are
				 * not doing it right now because of the dangerous null ptr deref
				 * This is a hack and should be fixed.
				 */
				//__free_pages(dst,0); 
				/*-------------Part of user page blocking for facilitating iova mapping change.-------------------*/
				abort++;
			}
			printk("line 396-----");
			
			
		/* 
		This will be used when we are using he protocol to update a user va
		BTW below after some lines, there is another unset happening. why are we unsetting the access bit twice. this is right, don't change this.
		 this is done because we want to compare with this in cmxchg

			pte_access_unset = pte_mkold(*pte); //should be same as __clear_bit(_PAGE_BIT_ACCESSED,*pte);

			
			
			
			//printk("line 62\n");
			
			//nOT DEALING WITH PHYSICAL PAGES SO BELOW functions ARE IRRELEVANT
			//page_folio(src);
			// folio_clear_referenced();

			//Step 1: Unset Access Bit
			//we want atomic unset of access bit so pte_mkold() won't work
			// Position of access bit in PTE: _PAGE_BIT_ACCESSED = 5 for user va and #define DMA_FL_PTE_ACCESS	BIT_ULL(5) for Intel IOMMU
			//the subsystems that turn off this bit need to be turned off, which has not been done here

			__clear_bit(_PAGE_BIT_ACCESSED, &pte->pte);
			
			
			pte_flag = pte_flags(*pte);
			
			pte_new.pte = pte_flag | (dest_pfn_val << PAGE_SHIFT);
			//printk("Dest PTE to PFN : %lu",pte_pfn(pte_new));
			
			//Step 2: Shootdown 
			flush_tlb_page(vmi, vpage);
			
			//Step 3: Copy 
			//migrate_folio(mapping, page_folio(dst), page_folio(src), MIGRATE_SYNC);
			folio_migrate_copy(page_folio(dst), page_folio(src));
			
			//We need to copy the old PTE status bits to the new PTE 
			//If mapcount of struct page is more than 0 i.e. more than one mapping exists then we should abort, but that is not added here
			//Step 4: Compare access bit and Xchange  PTE
			//printk("map_count %lu=",src->_mapcount);
		
			atomic_inc(&dst->_mapcount);
			atomic_inc(&dst->_refcount);
			//vaddr_src = kmap(src);
			//char temp[1]={32};
			//memcpy(vaddr_src,temp,1);
			//kunmap(src);
			cmpxchg(&(pte->pte), pte_access_unset.pte, pte_new.pte);
			if(pte->pte != pte_new.pte)//recheck this condition
			{	
				flag = 0;
				abort++;
				//printk("line 88\n");
			}	
			else
			{
				atomic_dec(&src->_mapcount);
				atomic_dec(&src->_refcount);
				__free_pages(src,0);
				success++;
			}
			//printk("%lu--%lu--%lu",pte->pte,pte_access_unset.pte,pte_new.pte);
			//printk("line 90\n");
			//break;
			//Step 5: Verify
			//char *vaddr_src = kmap(src), *vaddr_dst = kmap(dst);
			//char temp1[4096],temp2[4096];
			//memcpy(temp1,vaddr_src,4096);
			//memcpy(temp2,vaddr_dst,4096);
			//for(int i = 0; flag && i < 4096; i++)
			//{
			//	if(temp1[i] != temp2[i])printk("Mismatch in Page Contents");
			//	mismatch++;
			//	break;
			//}
			//kunmap(src);
			//kunmap(dst);
		
		flag = 1;
		
		*/

			//printk("line 214 silent_migrate\n");


		}
		//printk("line 92\n");
	}
	printk(KERN_INFO "Number of Total & Successful & Aborted Attempts & Mismatch Contents= %lu %lu %lu %lu\n",success+abort,success,abort,mismatch);
	custom_printk_flag = -1;
	return 0;
}
