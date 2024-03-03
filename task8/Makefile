obj-m += solution.o

all:
	# make -C /home/pacahon/linux/WSL2-Linux-Kernel M=$(shell pwd) modules
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	# make -C /home/pacahon/linux/WSL2-Linux-Kernel M=$(shell pwd) clean
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

load:
	sudo insmod solution.ko
	sudo dmesg -c

unload:
	sudo rmmod solution.ko
	sudo dmesg -c