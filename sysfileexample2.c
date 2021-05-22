#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/string.h>

static struct kobject *register_kobj;
static char *param_buf;

// function for many symbol data enter
static ssize_t __used store_value(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){ 
    printk(KERN_ALERT "you entered %s\n", buf);
    strncpy(param_buf, buf, PAGE_SIZE - 1);
    return count;
}

// register function to attribute
static struct kobj_attribute store_val_attribute = __ATTR( put_parameters, 0220, NULL, store_value);

// put attribute to attribute group
static struct attribute *register_attrs[] = {
    &store_val_attribute.attr,
    NULL,   /* NULL terminate the list*/
};
static struct attribute_group  reg_attr_group = {
    .attrs = register_attrs
};

static int hello_init(void){
    param_buf = kzalloc(PAGE_SIZE, GFP_KERNEL);
    // create sysfs object ( /sys/kernel/test_1025_sym directory )
    register_kobj = kobject_create_and_add("test_1025_sym", kernel_kobj);
    if (!register_kobj)
    return -ENOMEM;

    //create attributes (files)
    if(sysfs_create_group(register_kobj, &reg_attr_group)){
        kobject_put(register_kobj);
        return -ENOMEM;
    }

    return 0;
}

static void hello_exit(void){
    printk(KERN_ALERT "last value was %s\n", param_buf);
    kfree(param_buf);
    kobject_put(register_kobj);
}

MODULE_LICENSE("Dual BSD/GPL");
module_init(hello_init);
module_exit(hello_exit);