#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 
#include<linux/uaccess.h>             
#include<linux/sysfs.h> 
#include<linux/kobject.h> 
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/workqueue.h>            
 
 
#define IRQ_NO 11
 
char etx_value[250];
 
dev_t dev = 0;
/*
*************DEV_T FUNCTIONS****************
    ddi_create_minor_node()
    Create a minor node for a device

    ddi_getiminor()
    Get kernel internal minor number from an external dev_t

    ddi_remove_minor_node()
    Remove a minor mode for a device

    getmajor()
    Get major device number

    getminor()
    Get minor device number

    makedevice()
    Make device number from major and minor numbers
*/
static struct class *dev_class; //will be initiated by create_class later, necessary for device creation
static struct cdev character_device; //cdev = character device
struct kobject *kobject_for_sysfile;

/*FUNCTIONS FOR INITIALIZING THE DEVICE AND DETACHING THE DEVICE */ 
static int __init __driver_initializer(void);
static void __exit __driver_exit(void);
 
static struct workqueue_struct *own_workqueue;
static void workqueue_fn(struct work_struct *work); 
 
static DECLARE_WORK(work, workqueue_fn);
 
/*Linked List Node*/
struct data_collection_structure
{
     struct list_head list;    /*function from kernel link implementation*/
     char data[250];
};
 
/*Initialization of List head Node*/
LIST_HEAD(NodeHead);
 

/* Driver Fuctions */
static int driver_open(struct inode *inode, struct file *file);
static int driver_release(struct inode *inode, struct file *file);
static ssize_t driver_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t driver_write(struct file *filp, const char *buf, size_t len, loff_t * off);
 
/*Sysfs Fuctions */
static ssize_t sys_fs_function_for_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t sys_fs_function_for_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count);
struct kobj_attribute driver_attribute = __ATTR(etx_value, 0660, sys_fs_function_for_show, sys_fs_function_for_store);

/*Workqueue Function*/
static void workqueue_fn(struct work_struct *work)
{
        struct data_collection_structure *temp= NULL; 
        printk(KERN_INFO "Executing Workqueue Function\n");        
        /*Creating Node*/
        temp = kmalloc(sizeof(struct data_collection_structure), GFP_KERNEL);
        /*Data Assignment*/
        copy_to_user(temp->data,etx_value,250);
        /*Initializing List*/
        INIT_LIST_HEAD(&temp->list);
        /*Addition to linked list*/
        list_add_tail(&temp->list, &NodeHead);
}
 
 
//Interrupt handler for IRQ 11. 
static irqreturn_t irq_handler(int irq,void *dev_id) {
        printk(KERN_INFO "Shared IRQ: Interrupt Occurred\n");
        /*Allocating work to queue*/
        queue_work(own_workqueue, &work);
        
        return IRQ_HANDLED;
}
/*File operation structure to connect driver's intended open method*/ 
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = driver_read,
        .write          = driver_write,
        .open           = driver_open,
        .release        = driver_release,
};

/* sysfs file reading*/  
static ssize_t sys_fs_function_for_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        printk(KERN_INFO "Sysfs - Read!!!\n");
        copy_to_user(buf,etx_value,250);
        return sprintf(buf, "%s", etx_value);
}
/* sysfs file writing*/   
static ssize_t sys_fs_function_for_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
        printk(KERN_INFO "Sysfs - Write!!!\n");
        return count;
}






/*Module Initialization Functions, Constructor for Device*/  
static int __init __driver_initializer(void)
{
        /*Creating Device Number - Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0)
        {
                printk(KERN_INFO "Cannot allocate major number\n");
                return -1;
        }
        printk(KERN_INFO "Major Number Allotted to Device= %d \nMinor Number Allotted to Device= %d \n",MAJOR(dev), MINOR(dev));
 
        /*Initializing a Virtual File System Registration*/
        cdev_init(&character_device,&fops);
 
        /*Adding character device to the Virtual File System*/
        if((cdev_add(&character_device,dev,1)) < 0)
        {
            printk(KERN_INFO "Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL)
        {
            printk(KERN_INFO "Struct Class was not able to be made\n");
            goto r_class;
        }
 
        /*Finally, Device Creation*/
        if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL)
        {
            printk(KERN_INFO "Cannot create the Device \n");
            goto r_device;
        }
 
        /*Directory made in /sys/kernel/ */
        kobject_for_sysfile = kobject_create_and_add("etx_sysfs",kernel_kobj);
 
        /*Sysfs Creation*/
        if(sysfs_create_file(kobject_for_sysfile,&driver_attribute.attr))
        {
            printk(KERN_INFO"Cannot create sysfs file......\n");
            goto r_sysfs;
        }
        if (request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "etx_device", (void *)(irq_handler))) 
        {
            printk(KERN_INFO "my_device: cannot register IRQ \n");
            goto irq;
        }
 
        /*Creating workqueue */
        own_workqueue = create_workqueue("own_wq");
        
        printk(KERN_INFO "Device Driver Insert Successful\n");
        return 0;
 
irq:
        free_irq(IRQ_NO,(void *)(irq_handler));
 
r_sysfs:
        kobject_put(kobject_for_sysfile); 
        sysfs_remove_file(kernel_kobj, &driver_attribute.attr);
 
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        cdev_del(&character_device);
        return -1;
}
/*Module Exit function, Device Destructor*/ 
static void __exit __driver_exit(void)
{
 
        /* Go through the list and free the memory. */
        struct data_collection_structure *cursor, *temp;
        list_for_each_entry_safe(cursor, temp, &NodeHead, list) 
        {
            list_del(&cursor->list);
            kfree(cursor);
        }
 
        /* Delete workqueue */
        destroy_workqueue(own_workqueue);
        free_irq(IRQ_NO,(void *)(irq_handler));
        kobject_put(kobject_for_sysfile); 
        sysfs_remove_file(kernel_kobj, &driver_attribute.attr);
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&character_device);
        unregister_chrdev_region(dev, 1);
        printk(KERN_INFO "Device Driver Removal Complete!\n");
}
















/*
** This fuction will be called when we open the Device file
*/ 

static int driver_open(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Device File Opened...!!!\n");
        return 0;
}
/*
** This fuction will be called when we close the Device file
*/   
static int driver_release(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Device File Closed...!!!\n");
        return 0;
}
/*
** This fuction will be called when we read the Device file
*/ 
static ssize_t driver_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        struct data_collection_structure *temp;
        int count = 0;
        printk(KERN_INFO "Read function\n");
 
        /*Traversing Linked List and Print its Members*/
        list_for_each_entry(temp, &NodeHead, list) 
        {
            copy_from_user(buf,temp->data,250);
            printk(KERN_INFO "Node %d data = %s\n", count++, temp->data);
        }
 
        printk(KERN_INFO "Total Nodes = %d\n", count);
        return 0;
}
/*
** This fuction will be called when we write the Device file
*/
static ssize_t driver_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Write Function\n");
        /*Copying data from user space*/
        // sscanf(buf,"%d",&etx_value);
        struct data_collection_structure *temp= kmalloc(sizeof(struct data_collection_structure), GFP_KERNEL); 
        copy_from_user(temp->data,buf,250);
        list_add(&temp->list, &NodeHead);
        if( copy_from_user(&etx_value,buf,len) )
        {
                pr_err("Data Write : Err!\n");
        }
        printk(KERN_INFO "The File was written with the string: %.*s", len, temp->data);
        
        /* Triggering Interrupt */
        asm("int $0x3B");  // Corresponding to irq 11
        return len;
}
 
module_init(__driver_initializer);
module_exit(__driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Character Device With Interrupt Handling and Sysfs!");
