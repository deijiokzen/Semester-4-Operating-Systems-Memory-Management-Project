KERNEL_SOURCE := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# obj-m += hello.o
# obj-m += demo.o
# obj-m += list.o
# obj-m += pa2.o

# obj-m += InputList.o
obj-m += list.o
all:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
