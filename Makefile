KERNEL_SOURCE := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m += hello.o

all:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
ifneq ($(KERNELRELEASE),)

obj-m += demo.o

else

KDIR ?= "/lib/modules/`uname -r`/build"
all:
		$(MAKE) -C "$(KDIR)" M=`pwd` modules
clean:
		$(MAKE) -C "$(KDIR)" M=`pwd` clean
endif