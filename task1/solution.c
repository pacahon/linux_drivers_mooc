#include <linux/init.h> /* Необходим для макросов */
#include <linux/kernel.h>  /* pr_info() */
#include <linux/module.h>

static int __init hello_init(void)
{
	pr_info( "HELLO, WTF, loading\n");
	return 0;
}

static void __exit hello_exit(void)
{
	pr_info( "WTF, clean\n");
}


module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");