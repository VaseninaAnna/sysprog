#include <linux/module.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/errno.h>	/* error codes */
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/moduleparam.h>
#include <linux/string.h>
#include <linux/device.h>
#define KERNEL_SECTOR_SIZE 512
#define bio_first_sector(bio) ((bio_end_sector(bio) - bio_sectors(bio)))

char BLKDEV_NAME[32] = "ram_blk";
int BLKDEV_SIZE = 2*1024*100;
static int hardsect_size = 512;
static int blk_major;
static int blk_minor =1;


struct ram_blk {
    char *name;
    int size;
    u8 *data;
    struct request_queue *queue;
    struct gendisk *gd;
};
struct ram_blk *ram_blk;

static const struct block_device_operations ram_blk_fops = {
    .owner = THIS_MODULE,
};

blk_qc_t ram_blk_make_request(struct request_queue *q, struct bio *bio){
	if (bio_sectors(bio) == 0) {
		bio->bi_error = 0;
		bio_endio(bio);
		return BLK_QC_T_NONE;
	}

    struct ram_blk *mydev = q->queuedata;
    struct bio_vec *bvec = bio->bi_io_vec;
    uint64_t copied = 0;
	char *user_buf;
    int i;

    int bio_len = bio->bi_vcnt;
    char *our_buf = mydev->data+512*(bio_first_sector(bio));
    if (bio_data_dir(bio)==WRITE){
        for (i = 0; i < bio_len; i++){
            user_buf = (char *)kmap_atomic(bvec[i].bv_page);           
            memcpy(our_buf+copied, user_buf+bvec[i].bv_offset, bvec[i].bv_len);
            copied += bvec[i].bv_len;
            kunmap_atomic(user_buf);
        }
    } 
    else{
        for (i = 0; i < bio_len; i++){
            user_buf = (char *)kmap_atomic(bvec[i].bv_page);
            memcpy(user_buf+bvec[i].bv_offset, our_buf+copied, bvec[i].bv_len);
            copied += bvec[i].bv_len;            
            kunmap_atomic(user_buf);
        }        
    }
    bio_endio(bio);
    return BLK_QC_T_NONE;
}

static void destroy_ram_dev(void){
    printk("Destroying device %s \n", BLKDEV_NAME);
    if(ram_blk==NULL){
        printk("ram_blk is NULL. Destroyed\n");
        return;
    }
    if(ram_blk->gd){
        del_gendisk(ram_blk->gd);
		put_disk(ram_blk->gd);
		printk("For dev %s gendisk deleted\n", BLKDEV_NAME);
	}

	if(ram_blk->data) {
		vfree(ram_blk->data);
        printk ("For dev %s data destroyed\n", BLKDEV_NAME);
	}

	if(ram_blk->queue){
		blk_cleanup_queue(ram_blk->queue);
		printk("For dev %s queue cleaned\n", BLKDEV_NAME);
	}

    if(ram_blk->name){
        kfree(ram_blk->name);   
    }

    kfree(ram_blk);
    ram_blk=NULL;
    printk("Device destroyed");
    return;
}

static int create_ram_dev(void){
    struct gendisk *gd;
    int ret = 0;

    ram_blk = kzalloc(sizeof(struct ram_blk), GFP_KERNEL);
    if(!ram_blk){
        ret=-ENOMEM;
        printk("Not enough memory for allocating ram_blk");
        goto out;
    }

    ram_blk->name = kstrdup(BLKDEV_NAME, GFP_KERNEL);
    if(!ram_blk){
        ret = -ENOMEM;
        printk("Canot allocate name %s \n", BLKDEV_NAME);
        goto out;        
    }

    ram_blk->queue = blk_alloc_queue(GFP_KERNEL);
    if(!ram_blk->queue){
        ret = -ENOMEM;
        printk("Canot allocate queue for %s \n", BLKDEV_NAME);
        goto out;
    }

    blk_queue_make_request(ram_blk->queue, ram_blk_make_request); //passing function to process requests
    ram_blk->queue->queuedata = ram_blk;    //we can use this field as we like, it;s convinient to access the blk dev

    ram_blk->size = BLKDEV_SIZE;
	ram_blk->data = vmalloc(ram_blk->size * 512);
	if (ram_blk->data == NULL) {
		printk ("Cannot allocate data for %s \n", BLKDEV_NAME);
		ret=-ENOMEM;
        goto out;
	}

    gd = alloc_disk(blk_minor);
    if (!gd){
        ret=-ENOMEM;
        printk("Cannot allocate gendisk for %s \n", BLKDEV_NAME);
        goto out;
    }
    ram_blk->gd = gd;
    gd->private_data = ram_blk;
    gd->queue = ram_blk->queue;
    gd->major = blk_major;
    gd->first_minor = blk_minor;
    blk_minor++;
    gd->fops = &ram_blk_fops;
    snprintf(gd->disk_name, DISK_NAME_LEN, "%s", ram_blk->name);
    set_capacity(gd, ram_blk->size);

    add_disk(gd);
    printk("Device %s added\n", BLKDEV_NAME);
    return 0;

out:
    destroy_ram_dev();
    return ret;
}

static int __init ram_blk_init(void)
{
    int ret = 0;
    blk_major = register_blkdev(0, BLKDEV_NAME);
    if(blk_major<0){
        printk("Cannot allocate major number");
        ret=-EINVAL;
    }
    create_ram_dev();
    return ret;
}

static void __exit ram_blk_exit(void){
    if(ram_blk != NULL){
        destroy_ram_dev();
    }

    unregister_blkdev(blk_major, BLKDEV_NAME);
    printk("%s unregistered, exit.\n", BLKDEV_NAME);
}

module_init(ram_blk_init);
module_exit(ram_blk_exit);