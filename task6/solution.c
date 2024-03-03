#include <linux/init.h> /* Необходим для макросов */
#include <linux/kernel.h>  /* pr_info() */
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/moduleparam.h>


/**
 * make
 * sudo insmod solution.ko a=2 b=3 c=1,2,3,4,5
 * sudo lsmod | grep solution
 * cat /sys/kernel/my_kobject/my_sys
 * sudo rmmod solution.ko
 */ 

static int a = 0;
static int b = 0;
static int c[5];
static int arr_argc;

module_param(a, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(b, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param_array(c, int, &arr_argc, 0000);


/*
 * The "my_sys" file where a static variable is read from
 */
static int my_sys = 0;

static ssize_t my_sys_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", my_sys);
}


/* Sysfs attributes cannot be world-writable. */
static struct kobj_attribute my_sys_attribute =
	__ATTR_RO(my_sys);



static struct kobject *my_kobject;


static int __init solution_init(void)
{
	int retval;
	int sum, i;
	pr_info("HELLO, I'm loading\n");

	my_kobject = kobject_create_and_add("my_kobject", kernel_kobj); /* kernel_kobj - global kobject under /sys/kernel/ */
	if (!my_kobject)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	retval = sysfs_create_file(my_kobject, &my_sys_attribute.attr);
	if (retval) {
		kobject_put(my_kobject);
		pr_err("Failed to create my_sys file in /sys/kernel/my_kobject/\n");
	}

	sum = a + b;
	for (i = 0; i < 5; i++) sum += c[i];
	my_sys = sum;

	return retval;
}

static void __exit solution_exit(void)
{
	kobject_put(my_kobject);
}


module_init(solution_init);
module_exit(solution_exit);

MODULE_LICENSE("GPL");