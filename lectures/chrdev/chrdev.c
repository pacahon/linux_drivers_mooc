/**
 * make
 * sudo mknod /dev/chrdrv c 500 0, `c` - символьный, `500 0` - major manor
 * sudo chmod a+rw /dev/chrdrv
 * sudo insmod chrdev.ko
 * sudo lsmod | grep chrdev
 * cat /proc/modules | grep chrdev
 * echo '1' > /dev/chrdrv
 * sudo mknod /dev/chrdrv c 500 0 && sudo chmod a+rw /dev/chrdrv && sudo insmod chrdev.ko
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
static int my_major = 500, my_minor = 0;
static struct cdev *my_cdev;

#define DEVICE_NAME "mychrdev"
#define KBUF_SIZE (size_t) ((10) * PAGE_SIZE)


/* Вызывается, когда процесс пытается открыть файл устройства, например
 * "cat /dev/chrdrv"
 */
static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;
	char *kbuf = kmalloc(KBUF_SIZE, GFP_KERNEL);
	file->private_data = kbuf;
	pr_info( "Opening device: %s\n", DEVICE_NAME);
	counter++;
	pr_info("Counter: %d\n", counter);
	pr_info("Module refcounter: %d\n\n", module_refcount(THIS_MODULE));
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	char *kbuf;
	pr_info( "Closing device: %s\n", DEVICE_NAME);
	kbuf = file->private_data;
	if (kbuf) {
		pr_info("Free buffer\n\n");
		kfree(kbuf);
	}
	kbuf = NULL;
	file->private_data = NULL;
	return 0;
}

static ssize_t device_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos)
{
	static int counter = 0;
	char *kbuf = file->private_data;
	int nbytes = lbuf - copy_to_user(buf, kbuf + *ppos, lbuf); /* copy_to_user returns number of bytes that could not be copied. On success, it will be zero. */
	*ppos += nbytes;

	pr_info( "Read device %s nbytes = %d, ppos = %d:\n\n", DEVICE_NAME, nbytes, (int)*ppos);
	counter++;
	if (counter > 2) {
		return 0;
	}
	return nbytes;
}

static ssize_t device_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos)
{
	char *kbuf = file->private_data;
	int nbytes = lbuf - copy_from_user(kbuf + *ppos, buf, lbuf);
	*ppos += nbytes;

	pr_info( "Write device: %s nbytes = %d, ppos = %d\n\n", DEVICE_NAME, nbytes, (int)*ppos);
	return nbytes;
}

static const struct file_operations mycdev_fops = {
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

static int __init solution_init(void)
{
	pr_info( "HELLO, WTF, loading\n");
	first = MKDEV (my_major, my_minor); /* dev_t, MKDEV комбинирует major and minor в одно 32 битное число */
	register_chrdev_region(first, count, DEVICE_NAME); /* register a range of device numbers */
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
