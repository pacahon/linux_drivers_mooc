/**
 * make
 * sudo mknod /dev/solution_node c 240 0
 * sudo chmod a+rw /dev/solution_node
 * sudo insmod solution.ko
 * sudo lsmod | grep chrdev
 * cat /proc/devices | grep chrdev
 * echo '1' > /dev/chrdrv
 * sudo mknod /dev/solution_node c 240 0 && sudo chmod a+rw /dev/solution_node && sudo insmod solution.ko
 */
#include <linux/init.h> /* Необходим для макросов */
#include <linux/kernel.h>  /* pr_info() */
#include <linux/device.h> 
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/cdev.h> /* cdev_alloc, cdev_init, cdev_add, cdev_del */
#include <linux/interrupt.h>


#define DEVICE_NAME "solution"
#define IRQ_NO 8

static int sum_interrupts = 0;


//Interrupt handler for IRQ 8. 
static irqreturn_t irq_handler(int irq, void *dev_id) {
  	pr_info("[kernel_mooc] Shared IRQ: Interrupt Occurred");

  	if (irq == 8) {
  		sum_interrupts += 1;
  	}
  	return IRQ_HANDLED;
}


static ssize_t my_sys_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", sum_interrupts);
}

/* Sysfs attributes cannot be world-writable. */
static struct kobj_attribute my_sys_attribute =
	__ATTR_RO(my_sys);

static struct kobject *my_kobject;


static int __init chrdev_init(void)
{
	int retval;
	pr_info("[kernel_mooc] Loading solution.ko\n");

    retval = request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "rtc8", (void *)(&irq_handler));
	if (retval) {
        pr_err("[kernel_mooc] Cannot register IRQ 8\n");
        return -EIO;
    }

	my_kobject = kobject_create_and_add("my_kobject", kernel_kobj); /* kernel_kobj - global kobject under /sys/kernel/ */
	if (!my_kobject) {
		pr_err("[kernel_mooc] Cannot register /sys/kernel/my_kobject\n");
		return -ENOMEM;
	}

	/* Create the files associated with this kobject */
	retval = sysfs_create_file(my_kobject, &my_sys_attribute.attr);
	if (retval) {
		kobject_put(my_kobject);
		pr_err("[kernel_mooc] Failed to create my_sys file in /sys/kernel/my_kobject\n");
	}

	return retval;
}


static void __exit chrdev_exit(void)
{
	free_irq(IRQ_NO, (void *)(&irq_handler));
	kobject_put(my_kobject);
}


module_init(chrdev_init);
module_exit(chrdev_exit);

MODULE_LICENSE("GPL");
