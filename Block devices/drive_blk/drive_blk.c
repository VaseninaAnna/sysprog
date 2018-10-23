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

char BLKDEV_NAME[32] = "disk_blk";

char *bdev_path = "/dev/sdb" ;
module_param(bdev_path, charp, 0000);
MODULE_PARM_DESC(bdev_path, "Path to main storage block device");

static int blk_major;
static int blk_minor=1;
static struct mutex lock;

struct disk_blk {
    char *name;
    int size;
    struct block_device *bdev;
    struct request_queue *queue;
    struct gendisk *gd;
};
struct disk_blk *disk_blk;

static int disk_blk_open(struct block_device *bdev, fmode_t mode){
	return 0;
}

static void  disk_blk_release(struct gendisk *disk, fmode_t mode){
}

static const struct block_device_operations disk_blk_fops = {
    .owner = THIS_MODULE,
    .open = disk_blk_open,
	.release = disk_blk_release,
};

blk_qc_t disk_blk_make_request (struct request_queue *q, struct bio *bio){
    struct disk_blk *mydev = q->queuedata;
    
    if (bio_sectors(bio)==0){
        bio -> bi_error = 0;
        bio_endio(bio);
        return BLK_QC_T_NONE;
    }
    bio->bi_bdev = mydev->bdev;
    submit_bio(bio);
    return BLK_QC_T_NONE;
}

static void destroy_disk_dev(void){
    printk("Destroying device %s \n", BLKDEV_NAME);
    if(disk_blk==NULL){
        printk("disk_blk is NULL, device destroyed\n");
        return;
    }
    if(disk_blk->gd){
        del_gendisk(disk_blk->gd);
        put_disk(disk_blk->gd);
        printk("For dev %s gendisk destroyed\n", BLKDEV_NAME);
    }

    if(disk_blk->queue){
        blk_cleanup_queue(disk_blk->queue);
        printk("For dev %s queue cleaned\n", BLKDEV_NAME);
    }
    if(!IS_ERR(disk_blk->bdev) && disk_blk->bdev != NULL){
		blkdev_put(disk_blk->bdev, FMODE_READ | FMODE_WRITE);
		printk("For dev %s put bdev\n", BLKDEV_NAME);
	}
    if(disk_blk->name){
        kfree(disk_blk->name);
    }

    kfree(disk_blk);
    disk_blk=NULL;
    printk("Device destroyed\n");
    return;
}

static int create_disk_dev(void){
    struct gendisk *gd;
    int ret = 0;

    disk_blk = kzalloc(sizeof(struct disk_blk), GFP_KERNEL);
    if (!disk_blk){
        ret=-ENOMEM;
        printk("Not enough memory for allocating disk_blk \n");
        goto out;        
    } 

    disk_blk->name = kstrdup(BLKDEV_NAME, GFP_KERNEL);
    if(!disk_blk->name){
        ret = -ENOMEM;
        printk("Canot allocate name %s \n", BLKDEV_NAME);
        goto out;          
    }

    disk_blk->queue = blk_alloc_queue(GFP_KERNEL);
    if(!disk_blk->queue){
        ret = -ENOMEM;
        printk("Canot allocate queue for %s \n", BLKDEV_NAME);
        goto out;
    }

    blk_queue_make_request(disk_blk->queue, disk_blk_make_request); //passing function to process requests
    disk_blk->queue->queuedata = disk_blk;    //we can use this field as we like, it;s convinient to access the blk dev

    disk_blk->bdev = blkdev_get_by_path(bdev_path, FMODE_READ | FMODE_WRITE, disk_blk);
	if(IS_ERR(disk_blk->bdev)){
		printk("Cannot find bdev: %s \n", bdev_path);
		ret = -EINVAL;
		goto out;
	}

    disk_blk->size = get_capacity(disk_blk->bdev->bd_disk);

    gd = alloc_disk(blk_minor);
    if(!gd){
        ret=-ENOMEM;
        printk("Cannot allocate gendisk for %s\n", BLKDEV_NAME);
        goto out;
    }
    disk_blk->gd = gd;
    gd->private_data = disk_blk;
    gd->queue = disk_blk->queue;
    gd->major = blk_major;
    gd->first_minor = blk_minor;
    blk_minor++;
    gd->fops = &disk_blk_fops;
    snprintf(gd->disk_name, DISK_NAME_LEN, "%s", disk_blk->name);
    set_capacity(gd, disk_blk->size);

    add_disk(gd);
    printk("Device %s added\n", BLKDEV_NAME);
    return 0;

out:
    destroy_disk_dev();
    return ret;
}

static int __init disk_blk_init(void){
    int ret = 0;
   	mutex_init(&lock);

    blk_major = register_blkdev(0, BLKDEV_NAME);
    if(blk_major<0){
        printk("Cannot allocate major number");
        ret=-EINVAL;
    }
    return ret;
}

static void __exit disk_blk_exit(void){
    if(disk_blk != NULL){
        destroy_disk_dev();
    }

    unregister_blkdev(blk_major, BLKDEV_NAME);
    printk("%s is unregistered, exit.\n", BLKDEV_NAME);
}

int __set_cur_cmd(const char *str, struct kernel_param *kp){
	printk("Got command \"%s\"  control\n", str);
	if(!strcmp(str, "create\n")){
		create_disk_dev();
		return 0;
	}
	if(!strcmp(str, "destroy\n")){
		destroy_disk_dev();
		return 0;
	}
	strcpy(bdev_path, str);
    printk("bdev path set to %s", bdev_path);
	return 0;      
}
module_param_call(control, __set_cur_cmd, NULL, NULL, S_IRUGO | S_IWUSR);

module_exit(disk_blk_exit);
module_init(disk_blk_init);