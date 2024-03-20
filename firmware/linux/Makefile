# Source files directory
SRC_DIR := src

# Kernel module name
obj-m += $(SRC_DIR)/iceDriver.o

# Include files directory
INC_DIR := include

# Extra flags for the kernel module
EXTRA_CFLAGS := -I$(PWD)/$(INC_DIR)

# Kernel source directory
KDIR := /lib/modules/$(shell uname -r)/build

# Main rule to build the kernel module
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean rule
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean
