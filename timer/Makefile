ifneq ($(KERNELRELEASE),)
	obj-m := timer.o
else
	KDIR := /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(shell pwd) modules
	-rmmod timer
	insmod timer.ko

	gcc app.c -o app
clean:
	rm -rf *.ko *.o *.mod.o *.mod.c *.symvers  modul* .*.cmd .tmp_versions/ *.mod app
endif       


