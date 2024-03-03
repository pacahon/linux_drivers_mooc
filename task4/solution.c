#include <linux/init.h> /* Необходим для макросов */
#include <linux/kernel.h>  /* pr_info() */
#include <linux/module.h>
#include <checker.h>
#include <linux/slab.h> /* kmalloc */

/*
 * 1. Указатель произвольного типа
 * ssize_t get_void_size(void); // вернет сгенерированный размер в байтах, которое необходимо выделить. 
 * void submit_void_ptr(void *p);
 * 2. Указатель на целочисленный массив
 * ssize_t get_int_array_size(void); // вернет количество элементов, для которых требуется выделить память
 * void submit_int_array_ptr(int *p);
 * 3. Указатель на структуру
 * void submit_struct_ptr(struct device *p);
 *
 * void checker_kfree(void *p); // Для освобождения выделенной для указателя памяти, при выгрузке модуля,
 * вместо стандартной void kfree(const void *);
 */

void *ptr1;
int *ptr2;
struct device *ptr3;

static int __init solution_init(void)
{
	ssize_t size;

	size = get_void_size();
	ptr1 = kmalloc(size, GFP_KERNEL);
	submit_void_ptr(ptr1);

	size = get_int_array_size();
	ptr2 = kmalloc(size * sizeof (int), GFP_KERNEL);
	submit_int_array_ptr(ptr2);

	ptr3 = kmalloc(sizeof (struct device), GFP_KERNEL);
	submit_struct_ptr(ptr3);
	return 0;
}

static void __exit solution_exit(void)
{
	checker_kfree(ptr1);
	checker_kfree(ptr2);
	checker_kfree(ptr3);
}


module_init(solution_init);
module_exit(solution_exit);

MODULE_LICENSE("GPL");