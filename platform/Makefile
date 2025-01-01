ifneq ($(KERNELRELEASE),)
	obj-m := pltdev.o pltdrv.o
else
	KDIR := /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(shell pwd) modules
	-rmmod pltdev
	-rmmod pltdrv
	insmod pltdev.ko
	insmod pltdrv.ko

clean:
	rm -rf *.ko *.o *.mod.o *.mod.c *.symvers  modul* .*.cmd .tmp_versions/ *.mod app
	-rmmod pltdev
	-rmmod pltdrv
endif       


