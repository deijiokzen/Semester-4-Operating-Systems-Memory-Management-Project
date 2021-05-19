#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>


struct birthday {
  int day;
  int month;
  int year;
  struct list_head list;
};

static LIST_HEAD(birthday_list);

int pa2_init(void) {
  struct birthday *person = NULL;

  person = kmalloc(sizeof(*person), GFP_KERNEL);
  person->day = 28;
  person->month = 2;
  person->year = 1997;
  INIT_LIST_HEAD(&person->list);
  list_add_tail(&person->list, &birthday_list);

  person = kmalloc(sizeof(*person), GFP_KERNEL);
  person->day = 19;
  person->month = 3;
  person->year = 1997;
  INIT_LIST_HEAD(&person->list);
  list_add_tail(&person->list, &birthday_list);

  struct birthday *ptr;

  list_for_each_entry(ptr, &birthday_list, list) {
    printk(KERN_INFO "Birthday: %4d/%02d/%02d\n", ptr->year, ptr->month, ptr->day);
  }

  return 0;
}

void pa2_exit(void) {
  struct birthday *ptr, *next;
  long i;

  i = 0;
  list_for_each_entry_safe(ptr, next, &birthday_list, list) {
    printk(KERN_INFO "Successful del%ld.\n", i);
    i = i + 1;
    list_del(&ptr->list);
    kfree(ptr);
  }
}

module_init(pa2_init);
module_exit(pa2_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("PA2");
MODULE_AUTHOR("AW");