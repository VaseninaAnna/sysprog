KERNELDIR ?= /lib/modules/$(shell uname -r)/build

obj-m += drive_blk.o

all:
	make -Wall -C $(KERNELDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean