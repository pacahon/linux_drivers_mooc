obj-m += chrdev.o

all:
	make -C /home/pacahon/linux/WSL2-Linux-Kernel M=$(shell pwd) modules

clean:
	make -C /home/pacahon/linux/WSL2-Linux-Kernel M=$(shell pwd) clean

load:
	sudo insmod chrdev.ko
	sudo dmesg -c

unload:
	sudo rmmod chrdev.ko
	sudo dmesg -c