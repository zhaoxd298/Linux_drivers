ifneq ($(KERNELRELEASE),)
	obj-m := proc.o
else
	KDIR := /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(shell pwd) modules
	-rmmod proc
	insmod proc.ko

clean:
	rm -rf *.ko *.o *.mod.o *.mod.c *.symvers  modul* .*.cmd .tmp_versions/ *.mod
endif       


