obj-m += my_module.o

all:
	make -C /home/pacahon/linux/WSL2-Linux-Kernel M=$(shell pwd) modules

clean:
	make -C /home/pacahon/linux/WSL2-Linux-Kernel M=$(shell pwd) clean

load:
	sudo insmod my_module.ko
	sudo dmesg -c

unload:
	sudo rmmod my_module.ko
	sudo dmesg -c