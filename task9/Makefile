obj-m += solution.o

all:
	#make -C /home/pacahon/linux/WSL2-Linux-Kernel M=$(shell pwd) modules
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	#make -C /home/pacahon/linux/WSL2-Linux-Kernel M=$(shell pwd) clean
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

a: cl
	make -C /home/pacahon/linux/WSL2-Linux-Kernel M=$(shell pwd) modules	

cl:
	make -C /home/pacahon/linux/WSL2-Linux-Kernel M=$(shell pwd) clean

re: cl a
	sudo rm -f /dev/solution_node && sudo mknod /dev/solution_node c 240 0 && sudo chmod a+rw /dev/solution_node

test: unload load
	python3 test_10546.py
	sudo dmesg -c

load:
	sudo insmod solution.ko
	sudo dmesg -c

unload:
	sudo rmmod solution.ko
	sudo dmesg -c