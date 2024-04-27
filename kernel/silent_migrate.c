#include <../arch/x86/include/asm/tlbflush.h>
#include <../include/linux/migrate.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE0(silent_migrate)
{
	struct mm_struct *mm;
	struct vm_area_struct *vmi;
	struct folio *src_folio;
	unsigned long vpage;
	unsigned long abort = 0, success = 0, mismatch = 0;
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte, pte_access_unset, pte_new;
	pteval_t pte_flag;
	int flag = 1;
	char *vaddr_src, *vaddr_dst;
	mm = current->mm;
	
	for(vmi = mm->mmap; vmi != NULL ; vmi = vmi->vm_next)
	{	
		//printk("line 22\n"); printk("vmi_start vmi_end %lu %lu \n",vmi->vm_start,vmi->vm_end);
		//Current implementation only takes in account the anonymous vma region
		if(!vma_is_anonymous(vmi))
			continue;
		//printk("line 26\n");
		//Also skipping the stack pages currently
		if(mm->start_stack >= vmi->vm_start && mm->start_stack <= vmi->vm_end)
			continue;
		//printk("Line 30\n");
		//Iterate over base page size
		for( vpage = vmi->vm_start ; vpage < vmi->vm_end ; vpage += 4096 )
		{	
			pgd = pgd_offset(mm, vpage);
			if (pgd_none(*pgd) || pgd_bad(*pgd))
				continue;
			p4d = p4d_offset(pgd, vpage);
			if (p4d_none(*p4d) || p4d_bad(*p4d))
				continue;
			pud = pud_offset(p4d, vpage);
			if (pud_none(*pud) || pud_bad(*pud))
				continue;
			pmd = pmd_offset(pud, vpage);
			if (pmd_none(*pmd) || pmd_bad(*pmd))
				continue;
			pte = pte_offset_map(pmd, vpage);
			if (!pte_present(*pte)) 
				continue;
			if(pte_special(*pte))
			{
				//Currently Special PTE's cannot be handled. See memory.c -> vm_normal_page() for more info on "Special" PTE
				//printk("Special PTE\n");
				continue;
			}
			//printk("vpage addr = %lu\n", vpage);
			
			struct page *src = pte_page(*pte);
			printk("mapcount and refcount %d %d\n",page_count(src),total_mapcount(src));
			//Can rremove this later. iT WAS THERE JUST TO TEST
			if(PagePrivate(src))
			{
				printk("Page Private");
				continue;
			}
			if(total_mapcount(src)!=page_count(src))
				continue;
			struct page *dst = alloc_page(GFP_KERNEL);
			struct address_space *mapping = folio_mapping(page_folio(src));
			
			unsigned long dest_pfn_val = page_to_pfn(dst);
			//printk("Dest Page Frame number=%lu",dest_pfn_val);
			//It is important to hold a reference before starting the migration
			pte_access_unset = pte_mkold(*pte); //should be same as __clear_bit(_PAGE_BIT_ACCESSED,*pte);
			//printk("line 62\n");
			
			//nOT DEALING WITH PHYSICAL PAGES SO BELOW functions ARE IRRELEVANT
			//page_folio(src);
			// folio_clear_referenced();

			//Step 1: Unset Access Bit
			//we want atomic unset of access bit so pte_mkold() won't work
			// Position of access bit in PTE: _PAGE_BIT_ACCESSED = 5 
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
		}
		//printk("line 92\n");
	}
	printk("Number of Total & Successful & Aborted Attempts & Mismatch Contents= %lu %lu %lu %lu",success+abort,success,abort,mismatch);
	return 0;
}
