/**
 * make
 * sudo mknod /dev/solution_node c 240 0, `c` - символьный, `240 0` - major minor
 * sudo chmod a+rw /dev/solution_node
 * sudo insmod solution.ko
 * sudo lsmod | grep solution
 * cat /proc/modules | grep solution
 * https://it.wikireading.ru/1789?ysclid=lgjs510ty7660977460
 */
#include <linux/init.h> /* Необходим для макросов */
#include <linux/kernel.h>  /* pr_info() */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/cdev.h>

static dev_t first;
static unsigned int count = 1; /* the number of consecutive device numbers required */
static int my_major = 240, my_minor = 0;
static struct cdev *my_cdev;
static int open_count = 0;
static int write_count = 0;
static int counter = 0;

#define MYDEV_NAME "solution"
#define KBUF_SIZE (size_t) ((10) * PAGE_SIZE)


static int mychrdev_open(struct inode *inode, struct file *file)
{
	char *kbuf = kmalloc(KBUF_SIZE, GFP_KERNEL);
	file->private_data = kbuf;
	pr_info( "Opening device %s:\n\n", MYDEV_NAME);
	open_count++;
	counter = 0;
	pr_info("Counter: %d\n", open_count);
	pr_info("Module refcounter: %d\n", module_refcount(THIS_MODULE));
	return 0;
}


static int mychrdev_release(struct inode *inode, struct file *file)
{
	char *kbuf;
	pr_info( "Closing device %s:\n\n", MYDEV_NAME);
	kbuf = file->private_data;
	pr_info("Free buffer");
	if (kbuf) {
		kfree(kbuf);
	}
	kbuf = NULL;
	file->private_data = NULL;
	return 0;
}

static ssize_t mychrdev_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos)
{
	int nbytes;
	ssize_t output_len;
	char *kbuf = file->private_data;
	sprintf(kbuf, "%d %d\n", open_count, write_count);
	output_len = strlen(kbuf);
	nbytes = output_len - copy_to_user(buf, kbuf, output_len);
	*ppos += nbytes;
	counter++;
	pr_info( "[kernel_mooc] Read device %s nbytes = %d, count = %d:\n\n", MYDEV_NAME, write_count, open_count);
	if (counter > 1) {
		return 0;
	}
	return nbytes;
}

static ssize_t mychrdev_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos)
{
	char *kbuf = file->private_data;
	int nbytes = lbuf - copy_from_user(kbuf + *ppos, buf, lbuf);
	*ppos += nbytes;
	write_count += nbytes;
	pr_info( "[kernel_mooc] Write device %s nbytes = %d, ppos = %d, total= %d:\n\n", MYDEV_NAME, nbytes, (int)*ppos, write_count);
	return nbytes;
}

static const struct file_operations mycdev_fops = {
	.owner = THIS_MODULE,
	.read = mychrdev_read,
	.write = mychrdev_write,
	.open = mychrdev_open,
	.release = mychrdev_release
};

static int __init solution_init(void)
{
	pr_info( "Loading module\n");
	first = MKDEV (my_major, my_minor); /* macro MKDEV комбинирует major and minor в одно 32 битное число */
	register_chrdev_region(first, count, MYDEV_NAME); /* register a range of device numbers */
	my_cdev = cdev_alloc();
	cdev_init(my_cdev, &mycdev_fops);
	cdev_add(my_cdev, first, count);
	return 0;	
}

static void __exit solution_exit(void)
{
	pr_info( "WTF, clean\n");
	if (my_cdev) {
		cdev_del(my_cdev);
	}
	unregister_chrdev_region(first, count);
}


module_init(solution_init);
module_exit(solution_exit);

MODULE_LICENSE("GPL");