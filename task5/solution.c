#include <linux/init.h> /* Необходим для макросов */
#include <linux/kernel.h>  /* pr_info() */
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/cdev.h>

/**
 * make
 * sudo insmod solution.ko
 * sudo lsmod | grep solution
 * cat /sys/kernel/my_kobject/my_sys
 * sudo rmmod solution.ko
 */ 


/*
 * The "my_sys" file where a static variable is read from
 */
static int my_sys = 0;

static ssize_t my_sys_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	my_sys += 1;
	return sprintf(buf, "%d\n", my_sys);
}


/* Sysfs attributes cannot be world-writable. */
static struct kobj_attribute my_sys_attribute =
	__ATTR_RO(my_sys);



static struct kobject *my_kobject;


static int __init solution_init(void)
{
	int retval;
	pr_info("HELLO, I'm loading\n");

	my_kobject = kobject_create_and_add("my_kobject", kernel_kobj); /* kernel_kobj - global kobject under /sys/kernel/ */
	if (!my_kobject)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	retval = sysfs_create_file(my_kobject, &my_sys_attribute.attr);
	if (retval) {
		kobject_put(my_kobject);
		pr_err("Failed to create my_sys file in /sys/kernel/my_kobject\n");
	}

	return retval;
}

static void __exit solution_exit(void)
{
	kobject_put(my_kobject);
}


module_init(solution_init);
module_exit(solution_exit);

MODULE_LICENSE("GPL");