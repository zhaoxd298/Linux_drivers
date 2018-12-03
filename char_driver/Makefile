ifneq ($(KERNELRELEASE),)
	obj-m := char_driver.o
else
	KDIR := /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(shell pwd) modules
clean:
	rm -rf *.ko *.o *.mod.o *.mod.c *.symvers  modul* .*.cmd .tmp_versions/
endif       


