ifneq ($(KERNELRELEASE),)
	obj-m := poll_select.o
else
	KDIR := /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(shell pwd) modules
	rmmod poll_select
	insmod poll_select.ko
app:
	gcc app_read.c -o read
	gcc app_write.c -o write
clean:
	rm -rf *.ko *.o *.mod.o *.mod.c *.symvers  modul* .*.cmd .tmp_versions/ *.mod read write
endif       


