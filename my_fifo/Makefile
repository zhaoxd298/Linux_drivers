ifneq ($(KERNELRELEASE),)
	obj-m := my_fifo.o
else
	KDIR := /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(shell pwd) modules
	-rmmod my_fifo
	insmod my_fifo.ko
app:
	gcc app_read.c -o read
	gcc app_write.c -o write
clean:
	rm -rf *.ko *.o *.mod.o *.mod.c *.symvers  modul* .*.cmd .tmp_versions/ read write
endif       


