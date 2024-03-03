obj-m += solution.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
	cat /proc/modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

a:
	make -C /home/pacahon/linux/WSL2-Linux-Kernel M=$(shell pwd) modules	

cl:
	make -C /home/pacahon/linux/WSL2-Linux-Kernel M=$(shell pwd) clean

test: a unload load
	cat /sys/kernel/my_kobject/my_sys
	sudo dmesg -c

load:
	sudo insmod solution.ko
	sudo dmesg -c

unload:
	sudo rmmod solution.ko
	sudo dmesg -c