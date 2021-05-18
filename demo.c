#define pr_fmt(fmt) "demo: " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

static int __init demo_init(void) {
    void *buf;

    buf = kmalloc(1000, GFP_KERNEL);
    pr_info("kmalloc returned %p\n", buf);
    kfree(buf);
    return 0;
}

static void __exit demo_exit(void) {
}

module_init(demo_init);
module_exit(demo_exit);