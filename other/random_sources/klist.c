#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/list.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("IceMare");
MODULE_DESCRIPTION("Linked list without macros");

struct point3d {
    int x, y, z;
    struct list_head list;
};

// Declare list head manually instead of using LIST_HEAD
static struct list_head measuremetList;

static int __init list_example_init(void)
{
    struct point3d *pt1, *pt2, *pt3, *pt4;

    // Manually initialize the list head instead of using LIST_HEAD_INIT
    measuremetList.next = &measuremetList;
    measuremetList.prev = &measuremetList;

    // pt1
    pt1 = kmalloc(sizeof(*pt1), GFP_KERNEL);
    if (!pt1) {
        printk(KERN_ERR "Failed to allocate memory for pt1\n");
        return -ENOMEM;
    }
    pt1->x = 1; pt1->y = 2; pt1->z = 3;
    pt1->list.next = NULL;
    pt1->list.prev = NULL;
    list_add_tail(&pt1->list, &measuremetList);

    // pt2
    pt2 = kmalloc(sizeof(*pt2), GFP_KERNEL);
    if (!pt2) {
        printk(KERN_ERR "Failed to allocate memory for pt2\n");
        kfree(pt1);
        return -ENOMEM;
    }
    pt2->x = 4; pt2->y = 5; pt2->z = 6;
    pt2->list.next = NULL;
    pt2->list.prev = NULL;
    list_add_tail(&pt2->list, &measuremetList);

    // pt3
    pt3 = kmalloc(sizeof(*pt3), GFP_KERNEL);
    if (!pt3) {
        printk(KERN_ERR "Failed to allocate memory for pt3\n");
        kfree(pt1); kfree(pt2);
        return -ENOMEM;
    }
    pt3->x = 7; pt3->y = 8; pt3->z = 9;
    pt3->list.next = NULL;
    pt3->list.prev = NULL;
    list_add_tail(&pt3->list, &measuremetList);

    // pt4
    pt4 = kmalloc(sizeof(*pt4), GFP_KERNEL);
    if (!pt4) {
        printk(KERN_ERR "Failed to allocate memory for pt4\n");
        kfree(pt1); kfree(pt2); kfree(pt3);
        return -ENOMEM;
    }
    pt4->x = 10; pt4->y = 11; pt4->z = 12;
    pt4->list.next = NULL;
    pt4->list.prev = NULL;
    list_add_tail(&pt4->list, &measuremetList);

    // Print list contents
    struct point3d *entry;
    list_for_each_entry(entry, &measuremetList, list) {
        printk(KERN_INFO "Point: x=%d, y=%d, z=%d\n", entry->x, entry->y, entry->z);
    }

    return 0;
}

static void __exit list_example_exit(void)
{
    struct point3d *entry, *tmp;

    list_for_each_entry_safe(entry, tmp, &measuremetList, list) {
        printk(KERN_INFO "Freeing Point: x=%d, y=%d, z=%d\n", entry->x, entry->y, entry->z);
        list_del(&entry->list);
        kfree(entry);
    }
}

module_init(list_example_init);
module_exit(list_example_exit);
