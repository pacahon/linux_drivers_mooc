#include <linux/init.h> /* Необходим для макросов */
#include <linux/kernel.h>  /* pr_info() */
#include <linux/module.h>
#include <linux/random.h>
#include <checker.h>  /* CHECKER_MACRO, array_sum, generate_output */

/*
 * int array_sum(short *arr, size_t n);
 * ssize_t generate_output(int sum, short *arr, size_t size, char *buf);
 */

static int __init hello_init(void)
{
	int i;
	for (i = 0; i < 11; ++i)
	{
		int j, arrSize, arrSum, checkerSum, outputSize;
		unsigned int randInt;
		short int arr[10];
		char buf[1000];

		get_random_bytes(&randInt, sizeof(randInt));
		arrSize = (randInt % 10) + 1; /* 1 - 10 */
		pr_info("[kernel_mooc] arrSize = %d\n", arrSize);
		
		arrSum = 0;
		for (j = 0; j < arrSize; ++j)
		{
			int randInt;
			get_random_bytes(&randInt, sizeof(randInt));
			arr[j] = randInt % 100;
			arrSum += arr[j];
		}

		pr_info("[kernel_mooc] arrSum = %d\n", arrSum);

		checkerSum = array_sum(arr, arrSize);
		outputSize = generate_output(checkerSum, arr, arrSize, buf);
		if (checkerSum == arrSum) {
			pr_info("%.*s\n", outputSize, buf);
		} else {
			pr_err("%.*s\n", outputSize, buf);
		}
	}
	CHECKER_MACRO;
	return 0;
}

static void __exit hello_exit(void)
{
	CHECKER_MACRO;
}




module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");