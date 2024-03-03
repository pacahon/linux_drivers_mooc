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
#include <linux/ioctl.h>

#define SUCCESS 0
#define MAJOR_NUM 240
#define MINOR_NUM 0
#define DEVICE_NAME "mychrdev"
#define DEVICE_COUNT 1
#define KDATA_SIZE (size_t) ((10) * PAGE_SIZE)
#define KBUF_SIZE (KDATA_SIZE - (2 * sizeof(int)))
#define BUF_LEN 20

#define IOC_MAGIC 'k'
#define SUM_LENGTH _IOWR(IOC_MAGIC, 1, char*)
#define SUM_CONTENT _IOWR(IOC_MAGIC, 2, char*)
#define IOCTL_GET_MSG _IOR(IOC_MAGIC, 2, char*)


typedef struct {
    int size;
    int counter;
    char kbuf[KBUF_SIZE];
} file_private_data;


static struct cdev *my_cdev;
static int sum_length = 0;
static int sum_content = 0;


/* Вызывается, когда процесс пытается открыть файл устройства, например "cat /dev/chrdrv" */
static int device_open(struct inode *inode, struct file *flip)
{
	file_private_data * data = kmalloc(KDATA_SIZE, GFP_KERNEL);
	data->size = 0;
	data->counter = 0;
	flip->private_data = data;
	memset(data->kbuf, 0, KBUF_SIZE);
	pr_info("[kernel_mooc] Opening device: %s\n", DEVICE_NAME);
	return 0;
}

static int device_release(struct inode *inode, struct file *filp)
{
	file_private_data * pdata = filp->private_data;
	pr_info("[kernel_mooc] Closing device: %s\n", DEVICE_NAME);
	if (pdata) {
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

	if ((int)*ppos > pr_data->size) {
		pr_data->counter = 0;
		return 0;
	}

	sprintf(pr_data->kbuf, "%d %d\n", sum_length, sum_content);
	output_len = strlen(pr_data->kbuf);
	nbytes = output_len - copy_to_user(buf, pr_data->kbuf, output_len);
	*ppos += nbytes;

	pr_data->counter++;

	if (nbytes == 0) {
		pr_data->counter = 0;
	}


	pr_info("[kernel_mooc] Device read sum_length = %d sum_content = %d\n", sum_length, sum_content);

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

	pr_info("[kernel_mooc] Write device[%s]: nbytes = %d, ppos = %d size = %d\n\n", DEVICE_NAME, nbytes, (int)*ppos, pr_data->size);
	return nbytes;
}

static loff_t device_llseek(struct file *filp, loff_t offset, int whence)
{
	loff_t newpos;
	file_private_data * pr_data = filp->private_data;

	pr_info("[kernel_mooc] Seek device[%s]: seek(%d, %d) requested\n", DEVICE_NAME, (int)offset, whence);

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
	if (newpos < 0) {
		return -EINVAL;
	}
	filp->f_pos = newpos;

	pr_info("[kernel_mooc] Seeking to %ld position\n", (long)newpos);

	return newpos;
}



/* Эта функция вызывается, когда процесс пытается выполнить ioctl для 
 * файла устройства. Мы получаем два дополнительных параметра 
 * (дополнительных для структур inode и file, которые получают все  
 * функции устройств): номер ioctl и параметр, заданный для этой ioctl. 
 * 
 * Если ioctl реализует запись или запись/чтение (то есть ее вывод 
 * возвращается вызывающему процессу), вызов ioctl возвращает вывод 
 * этой функции.
 */ 
static long device_ioctl(struct file *filp, unsigned int ioctl_num, /* Число и параметр для ioctl */ unsigned long ioctl_param) 
{
    int i;
    long ret = SUCCESS;
 
    /* Переключение согласно вызванной ioctl. */ 
    switch (ioctl_num) { 
	    case SUM_LENGTH: { 
	        /* Получение указателя на сообщение (в пользовательском  
	         * пространстве) и установка его как сообщения устройства. 
	         * Получение параметра, передаваемого ioctl процессом. 
	         */ 
	        char __user *tmp = (char __user *)ioctl_param; 
	        char ch; 
	 
	        /* Определение длины сообщения. */ 
	        get_user(ch, tmp); 
	        for (i = 0; ch && i < BUF_LEN; i++, tmp++) 
	            get_user(ch, tmp);

	        sum_length += i - 1;
	        ret = sum_length;
	        break; 
	    }
	   case SUM_CONTENT: { 
	        /* Получение указателя на сообщение (в пользовательском  
	         * пространстве) и установка его как сообщения устройства. 
	         * Получение параметра, передаваемого ioctl процессом. 
	         */ 
	        char __user *tmp = (char __user *)ioctl_param; 
	 		
	 		ret = kstrtoint_from_user(tmp, BUF_LEN, 0, &i);
	 		if (ret < 0) {
	 			return ret;
	 		}
	        sum_content += i;
	        ret = sum_content;
	        break; 
	    } 
	    case IOCTL_GET_MSG: { 
	        loff_t offset = 0; 
	 
	        /* Передача текущего сообщения вызывающему процессу. Получаемый 
	         * параметр является указателем, который мы заполняем. 
	         */ 
	        i = device_read(filp, (char __user *)ioctl_param, 99, &offset); 
	 
	        /* Помещаем в конец буфера нуль, чтобы он правильно завершился. 
	         */ 
	        put_user('\0', (char __user *)ioctl_param + i); 
	        break; 
	    }
	}

	pr_info("[kernel_mooc] Return %ld sum_length = %d sum_content = %d\n", ret, sum_length, sum_content);
 
    return ret; 
}



static const struct file_operations mycdev_fops = {
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
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
