obj-m += src/iceDriver.o

iceDriver-objs := src/charDevice.o

KERNEL_DIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Include files directory
INC_DIR := include

# Extra flags for the kernel module
EXTRA_CFLAGS := -I$(PWD)/$(INC_DIR)

default:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
