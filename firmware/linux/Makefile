obj-m += iceDriver.o

iceDriver-objs := src/iceDriver.o src/charDevice.o src/workLoad.o src/mutex.c

EXTRA_CFLAGS=-I$(PWD)/include

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules -j

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean -j
