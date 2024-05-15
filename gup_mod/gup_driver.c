#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/blk-mq.h>
#include <linux/cred.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/kallsyms.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/nvme.h>
#include <linux/pci.h>
#include <linux/printk.h>
#include <linux/page_owner.h>
#include <linux/page_ext.h>
#include <linux/pagemap.h>

static dev_t first; // Global variable for the first device number
static struct cdev c_dev; // Global variable for the character device structure
static struct class *cl; // Global variable for the device class

static int my_open(struct inode *i, struct file *f)
{
    printk(KERN_INFO "Driver: open()\n");
    return 0;
}
static int my_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "Driver: close()\n");
    return 0;
}
static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Driver: read()\n");
    return 0;
}
static ssize_t my_write(struct file *f, const char __user *buf, size_t len,
    loff_t *off)
{
    int ret;
    struct page_ext *page_ext;
    struct page_owner *pg_owner;
    struct page **pages = kvmalloc_array(len / 4096, sizeof(struct page *),GFP_KERNEL);
    printk(KERN_INFO "Driver: write()\n");

    //Pin pages
    ret = get_user_pages_fast( (unsigned long)buf, len/4096 , FOLL_WRITE , pages);
    printk(KERN_INFO "Number of pages requested: %lu", len/4096);
    printk(KERN_INFO "Number of pages allocated: %lu", ret);
    if (ret <= 0) {
        kvfree(pages);
        printk(KERN_INFO "get_user_pages_fast failed.\n");
        return -ENOMEM;
    }
    
    unsigned long temp = 0;
    for(int i=0; i<ret;i++)
    {
        //printk(KERN_INFO "page_count and mapcount & page to pfn %d %d %lu",page_count(pages[i]),page_mapcount(pages[i]),page_to_pfn(pages[i]));
        
        //for using page_owner, page_owner=on need to be set in boot params

        page_ext = lookup_page_ext(pages[i]);
        if(page_ext == NULL)
        {
            printk(KERN_INFO "page ext null");
            continue;
        }
		pg_owner = (void *)page_ext + page_owner_ops.offset;
        if(pg_owner->flag_gup==0)
        {
            printk(KERN_INFO "not flagged. Check kernel");
        }
        else
        {
            printk(KERN_INFO "---pfn pinned %lu---",page_to_pfn(pages[i]));
        }
        //pg_owner->flag_gup++;

        if(page_mapping(pages[i]))
            temp++;

        struct page * pg_temp = pfn_to_page(page_to_pfn(pages[i])+1);
        page_ext = lookup_page_ext(pg_temp);
        if(page_ext == NULL)
            continue;
        if(page_mapping(pg_temp))
            printk("page map spotted");
        
		pg_owner = (void *)page_ext + page_owner_ops.offset;
        printk("next page pfn flag gup: %d",pg_owner->flag_gup);
        
        
    }
    printk(KERN_INFO "non null mapping exists for %lu pages",temp);
    /*long itr = 0;
    for(itr=0;itr<ret;itr++)
    {
        if( itr%512==0 || itr%512==255 || itr%512==511)
            continue;
        put_page(pages[itr]);
    }
    */
    //msleep(60000);
    /*for(itr=0;itr<ret;itr++)
    {
        if( itr%512==0 || itr%512==255 || itr%512==511)
            put_page(pages[itr]);
    }
    */
    return ret;
}

static struct file_operations custom_fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write
};

static unsigned long count_gup_flagged(void)
{
    struct zone *zone;
    unsigned long ret = 0;
    //return ret;
    //unsigned long *hist ;
    unsigned long hist_flag = 0,count = 0;
    for_each_zone(zone)
    {
        struct page *page;
	    unsigned long pfn = zone->zone_start_pfn;
	    unsigned long end_pfn = pfn + zone->spanned_pages;
        //for(unsigned long i = pfn/10000; i < end_pfn/10000; i++)
        //    hist[i]=0;
	    int pageblock_mt;
	    pfn = zone->zone_start_pfn;
        for (; pfn < end_pfn; pfn++)
        {
            struct page_ext *page_ext;
            struct page_owner *pg_owner;
	        struct page *page_temp = pfn_to_page(pfn);
            if(!page_temp)continue;
            page_ext = lookup_page_ext(page_temp);
            
	        if(page_ext == NULL)
                continue;

	        pg_owner = (void *)page_ext + page_owner_ops.offset;
            if(pfn%512 == 0)
            {
                hist_flag = 0;        
            }
            if(pg_owner->flag_gup>0)
            {
                //hist[pfn/10000]++;
                if(hist_flag == 0)
                {
                    hist_flag = 1;
                    count++;
                }
                ret++;
            }
        }
    }
   printk(KERN_INFO "COUNT OF DISTINCT PAGEBLOCK WITH GUP FLAGGED PAGES : %lu",count);
    return ret;
}


static int __init ofcd_init(void) /* Constructor */
{
    int ret;
    struct device *dev_ret;

    printk(KERN_INFO "Registered Driver");
    if ((ret = alloc_chrdev_region(&first, 0, 1, "GUP")) < 0)
    {
        return ret;
    }
    if (IS_ERR(cl = class_create(THIS_MODULE, "gupdrv")))
    {
        unregister_chrdev_region(first, 1);
        return PTR_ERR(cl);
    }
    if (IS_ERR(dev_ret = device_create(cl, NULL, first, NULL, "devnull")))
    {
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return PTR_ERR(dev_ret);
    }

    cdev_init(&c_dev, &custom_fops);
    if ((ret = cdev_add(&c_dev, first, 1)) < 0)
    {
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return ret;
    }

    unsigned long gup_flagged = 0;
    gup_flagged = count_gup_flagged();
    printk(KERN_INFO "gup flagged pfn(shpuld be zero ideally) %lu \n",gup_flagged);
    return 0;
}

static void __exit ofcd_exit(void) /* Destructor */
{
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    printk(KERN_INFO "Unregistered Driver");
}

module_init(ofcd_init);
module_exit(ofcd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("glubag");
MODULE_DESCRIPTION("Tree Pie");
