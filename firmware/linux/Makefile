# Kernel module name
obj-m += iceDriver.o

# Source files directory
SRC_DIR := src

# Include files directory
INC_DIR := include

# Extra flags for the kernel module
EXTRA_CFLAGS := -I$(PWD)/$(INC_DIR)

# Kernel source directory
KDIR := /lib/modules/$(shell uname -r)/build

# Main rule to build the kernel module
all:
	make -C $(KDIR) M=$(PWD) modules

# Clean rule
clean:
	make -C $(KDIR) M=$(PWD) clean

.PHONY: all clean
