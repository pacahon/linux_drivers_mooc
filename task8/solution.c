/**
 * make
 * sudo insmod chrdev.ko
 * sudo lsmod | grep chrdev
 * cat /proc/devices | grep chrdev
 * echo '1' > /dev/chrdrv
 * sudo mknod /dev/chrdrv c 500 0 && sudo chmod a+rw /dev/chrdrv && sudo insmod chrdev.ko
 */
#include <linux/init.h> /* Необходим для макросов */
#include <linux/kernel.h>  /* pr_info() */
#include <linux/device.h> 
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/cdev.h>

static struct class *device_class;
static char *node_name = "chrdrv";
static int major_num = 0;
static int counter = 0;

module_param(node_name, charp, 0000);
MODULE_PARM_DESC(node_name, "Device file name");


#define DEVICE_NAME "mychrdev"
#define KBUF_SIZE (size_t) ((10) * PAGE_SIZE)


/* Вызывается, когда процесс пытается открыть файл устройства, например "cat /dev/chrdrv" */
static int device_open(struct inode *inode, struct file *file)
{
	char *kbuf = kmalloc(KBUF_SIZE, GFP_KERNEL);
	file->private_data = kbuf;
	pr_info( "Opening device: %s\n", DEVICE_NAME);
	counter = 0;
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
	int nbytes;
	ssize_t output_len;
	char *kbuf = file->private_data;
	sprintf(kbuf, "%d\n", major_num);
	output_len = strlen(kbuf);
	nbytes = output_len - copy_to_user(buf, kbuf, output_len);
	*ppos += nbytes;
	counter++;
	if (counter > 1) {
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

static loff_t device_llseek(struct file *file, loff_t offset, int orig)
{
	loff_t testpos;

	switch (orig)
	{
	case SEEK_SET:
		testpos = offset;
		break;
	case SEEK_CUR:
		testpos = file->f_pos + offset;
		break;
	case SEEK_END:
		testpos = KBUF_SIZE + offset;
		break;
	default:
		return -EINVAL;
	}

	testpos = testpos < KBUF_SIZE ? testpos : KBUF_SIZE;
	testpos = testpos >= 0 ? testpos : 0;
	file->f_pos = testpos;

	pr_info("Seeking to %ld position\n", (long)testpos);

	return testpos;
}

static const struct file_operations mycdev_fops = {
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
	.llseek = device_llseek
};

static int __init chrdev_init(void)
{
	int ret_val;
	pr_info( "Loading module for device %s\n", DEVICE_NAME);

	ret_val = register_chrdev(0, DEVICE_NAME, &mycdev_fops);
	if (ret_val < 0) { 
        pr_alert("Sorry, registering the character device failed with %d\n", ret_val); 
        return ret_val; 
    }

    major_num = ret_val;

	/* Динамически создаём ноду */
	device_class = class_create(THIS_MODULE, "myclass");
	device_create(device_class, NULL,  MKDEV(major_num, 0), "%s", node_name);
	pr_info("Device created on /dev/%s\n", node_name);

	return 0;
}

static void __exit chrdev_exit(void)
{
	pr_info( "Exiting module for device %s\n", DEVICE_NAME);
	device_destroy(device_class, MKDEV(major_num, 0));
	class_destroy(device_class);

	unregister_chrdev(major_num, DEVICE_NAME); 
}


module_init(chrdev_init);
module_exit(chrdev_exit);

MODULE_LICENSE("GPL");
