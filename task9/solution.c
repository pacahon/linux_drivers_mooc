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

#define MAJOR_NUM 240
#define MINOR_NUM 0
#define DEVICE_NAME "mychrdev"
#define DEVICE_COUNT 1
#define KDATA_SIZE (size_t) ((10) * PAGE_SIZE)
#define KBUF_SIZE (KDATA_SIZE - (4 * sizeof(int)))

typedef struct {
	int session_num;
    int size;
    int counter;
    int reads_count;
    char kbuf[KBUF_SIZE];
} file_private_data;


static struct cdev *my_cdev;
static int session_num = 0;


/* Вызывается, когда процесс пытается открыть файл устройства, например "cat /dev/chrdrv" */
static int device_open(struct inode *inode, struct file *flip)
{
	file_private_data * data = kmalloc(KDATA_SIZE, GFP_KERNEL);
	data->session_num = session_num;
	data->size = 0;
	data->counter = 0;
	data->reads_count = 0;
	flip->private_data = data;
	memset(data->kbuf, 0, KBUF_SIZE);
	pr_info("[kernel_mooc] Opening device: %s session_num=%d\n", DEVICE_NAME, session_num);
	pr_info("[kernel_mooc] Module refcounter: %d session_num=%d\n\n", module_refcount(THIS_MODULE), session_num);
	session_num++;
	return 0;
}

static int device_release(struct inode *inode, struct file *filp)
{
	file_private_data * pdata = filp->private_data;
	pr_info("[kernel_mooc] Closing device: %s\n", DEVICE_NAME);
	if (pdata) {
		pr_info("[kernel_mooc] Free memory allocated for private data session_num=%d\n\n", pdata->session_num);
		kfree(pdata);
	}
	filp->private_data = NULL;
	return 0;
}

static ssize_t device_read(struct file *filp, char __user *buf, size_t lbuf, loff_t *ppos)
{
	int nbytes = 0;
	ssize_t output_len;
	file_private_data * pr_data = filp->private_data;

	if (pr_data->counter > 0) {
		pr_data->counter = 0;
		return 0;
	}

	pr_info("[kernel_mooc] Read device[%s]: Read from session = %d ppos = %d file_size=%d counter=%d\n", DEVICE_NAME, pr_data->session_num, (int)*ppos, pr_data->size, pr_data->counter);

	if (pr_data->reads_count == 0) {
		// FIXME: Should I move ppos in that case?
		pr_info("[kernel_mooc] The first read -> return session number = %d\n", pr_data->session_num);
		sprintf(pr_data->kbuf + *ppos, "%d", pr_data->session_num);
		pr_data->size += strlen(pr_data->kbuf + *ppos);
	}

	if ((int)*ppos > pr_data->size) {
		pr_data->counter = 0;
		return 0;
	}

	output_len = strlen(pr_data->kbuf + *ppos);
	nbytes = output_len - copy_to_user(buf, pr_data->kbuf + *ppos, output_len);
	*ppos += nbytes;

	pr_data->reads_count++;
	pr_data->counter++;

	pr_info("[kernel_mooc] Read device[%s]: session = %d, str_len = %d, new_ppos = %d file_size = %d counter = %d reads_count = %d\n", DEVICE_NAME, pr_data->session_num, nbytes, (int)*ppos, pr_data->size, pr_data->counter, pr_data->reads_count);
	if (nbytes == 0) {
		pr_data->counter = 0;
	}
	return nbytes;
}

static ssize_t device_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos)
{
	file_private_data * pr_data = file->private_data;
	int nbytes = lbuf - copy_from_user(pr_data->kbuf + *ppos, buf, lbuf);
	*ppos += nbytes;

	if (pr_data->size < (int)*ppos) {
		pr_data->size = *ppos;
	}

	pr_info("[kernel_mooc] Write device[%s]: session = %d, nbytes = %d, ppos = %d size = %d\n\n", DEVICE_NAME, pr_data->session_num, nbytes, (int)*ppos, pr_data->size);
	return nbytes;
}

static loff_t device_llseek(struct file *filp, loff_t offset, int whence)
{
	loff_t newpos;
	file_private_data * pr_data = filp->private_data;

	pr_info("[kernel_mooc] Seek device[%s]: session = %d seek(%d, %d) requested\n", DEVICE_NAME, pr_data->session_num, (int)offset, whence);

	switch (whence)
	{
	case SEEK_SET:
		newpos = offset;
		break;
	case SEEK_CUR:
		newpos = filp->f_pos + offset;
		break;
	case SEEK_END:
		newpos = pr_data->size + offset;
		break;
	default:
		return -EINVAL;
	}
	pr_info("[kernel_mooc] Calculated newpos=%ld file_size=%d\n", (long)newpos, pr_data->size);
	// newpos = newpos < pr_data->size ? newpos : pr_data->size;
	// newpos = newpos >= 0 ? newpos : 0;
	if (newpos < 0) {
		return -EINVAL;
	}
	filp->f_pos = newpos;

	pr_info("[kernel_mooc] Seeking to %ld position session = %d\n", (long)newpos, pr_data->session_num);

	return newpos;
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
	int err;
	dev_t first = MKDEV(MAJOR_NUM, MINOR_NUM); /* MKDEV комбинирует major and minor в одно 32 битное число */
	pr_info("[kernel_mooc] Register the character device %s\n", DEVICE_NAME);
	register_chrdev_region(first, DEVICE_COUNT, DEVICE_NAME); /* register a range of device numbers */
	my_cdev = cdev_alloc();
	cdev_init(my_cdev, &mycdev_fops);
	err = cdev_add(my_cdev, first, DEVICE_COUNT);
	
	if (err) { 
        pr_alert("[kernel_mooc] Sorry, registering the character device failed with %d\n", err); 
        return err; 
    }

	return 0;
}

static void __exit chrdev_exit(void)
{
	pr_info("[kernel_mooc] Unregister the character device %s\n", DEVICE_NAME);
	if (my_cdev) {
		cdev_del(my_cdev);
	}
	unregister_chrdev_region(MKDEV(MAJOR_NUM, MINOR_NUM), DEVICE_COUNT);
}


module_init(chrdev_init);
module_exit(chrdev_exit);

MODULE_LICENSE("GPL");
