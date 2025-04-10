obj-m += iceNET.o

ARCH := arm64
CROSS_COMPILE := aarch64-linux-gnu-
KDIR := /ice/petalinux/linux-xlnx
INCLUDE := /ice/petalinux/linux-xlnx-headers

all:
	make -C $(KDIR) $(INCLUDE) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	make -C $(KDIR) $(INCLUDE) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean

