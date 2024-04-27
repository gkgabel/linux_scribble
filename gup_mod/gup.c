#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/cdev.h>
#include <linux/cred.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/kallsyms.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/nvme.h>
#include <linux/pci.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/types.h>

unsigned long buff_addr,nr_pages;

module_param(buff_addr,ulong,0);
module_param(nr_pages,ulong,0);

void cleanup_module(void)
{
	printk("Unloading GUP module\n");
}

static int __init gup(void) {
    //struct bypassd_dev *dev_entry = ns_entry->bypassd_dev_entry;
    //struct bypassd_user_buf buf;
    struct page **pages;
    unsigned int flags;
    unsigned long ret;
    //phys_addr_t phys;
    //dma_addr_t *dma_addr_list;
    int i;
    //buf = 
    //copy_from_user(&buf, __buf, sizeof(buf));

    flags         = FOLL_WRITE;
    pages         = kvmalloc_array(nr_pages, sizeof(struct page *),
                        GFP_KERNEL);
    //dma_addr_list = kmalloc (sizeof(__u64) * buf.nr_pages, GFP_KERNEL);
    ret = get_user_pages_fast(current->mm->mmap->vm_start, nr_pages, flags, pages);
    //ret = get_user_pages_fast((unsigned long)buff_addr, nr_pages, flags, pages);
    printk("Number of pages allocated: %lu",ret);
    if (ret <= 0) {
        kvfree(pages);
        //kfree(dma_addr_list);
        printk(" get_user_pages_fast failed.\n");
        return -ENOMEM;
    }
    //unsigned long itr;
    //for(itr=0;itr<ret;itr++)
    //{
//	    if(pages[itr]==NULL)continue;
//	    put_page(pages[itr]);
  //  }
    //buf.nr_pages = ret;

    //for (i=0; i<ret; ++i) {
      //  phys             = page_to_phys(pages[i]);
       // dma_addr_list[i]  = (dma_addr_t)phys;
      //  dma_addr_list[i] -= ((dma_addr_t)dev_entry->pdev->dev.dma_pfn_offset << PAGE_SHIFT);
       // if (dma_addr_list[i] == 0) {
       //     pr_err("[bypassd]: Invalid page address while allocating DMA pages.\n");
       //     buf.nr_pages--;
       //}
    //}

    //copy_to_user(__buf, &buf, sizeof(buf));
    //copy_to_user(buf.dma_addr_list, dma_addr_list, sizeof(__u64) * ret);

    //kvfree(pages);
    //kfree(dma_addr_list);

    return 0;
}

module_init(gup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("I");
MODULE_DESCRIPTION("GUP");
