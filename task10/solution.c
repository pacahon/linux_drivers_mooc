#include <linux/init.h> /* Необходим для макросов */
#include <linux/kernel.h>  /* pr_info() */
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/list_sort.h>


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
static char my_sys[MODULE_NAME_LEN] = "";

static int cmp(void *priv, const struct list_head *a, const struct list_head *b)
{
	struct module *ela, *elb;

	ela = container_of(a, struct module, list);
	elb = container_of(b, struct module, list);

	return strcmp(ela->name, elb->name);
}

static ssize_t my_sys_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	ssize_t nbytes = 0;
	struct list_head *i;
	struct module *mod;
	//sprintf(THIS_MODULE->name, "%s", "solution");

	list_for_each(i, &THIS_MODULE->list) {
	    mod = list_entry(i, struct module, list);
	}

	// TODO: https://stackoverflow.com/questions/35694862/sorting-a-linux-list-h-linked-list-with-list-sort
	// https://stackoverflow.com/questions/29571210/list-first-entry-array-of-linked-list-linux-kernel

	list_sort(NULL, &mod->list, cmp);

	list_for_each(i, &mod->list) {
	    struct module *next_module = list_entry(i, struct module, list);
		sprintf(buf + nbytes, "%s\n", next_module->name);
		nbytes += strlen(buf + nbytes);
	    pr_info("[kernel_mooc] Write module name %s len = %d", next_module->name, (int)strlen(next_module->name));
	}

	
	return nbytes;
}


/* Sysfs attributes cannot be world-writable. */
static struct kobj_attribute my_sys_attribute =
	__ATTR_RO(my_sys);



static struct kobject *my_kobject;


static int __init solution_init(void)
{
	int retval;
	pr_info("Loading solution.ko\n");

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