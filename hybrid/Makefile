obj-m += iceNET.o

ARCH := arm64
CROSS_COMPILE := /usr/bin/aarch64-linux-gnu-

all:
	# Make sure this line is indented with a tab, not spaces
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	# Make sure this line is indented with a tab, not spaces
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean

