obj-m += iceDriver.o

iceDriver-objs += src/charDevice.o
iceDriver-objs += src/gpioCtrl.o
iceDriver-objs := src/iceDriver.o
iceDriver-objs += src/spiCtrl.o
iceDriver-objs += src/spiWork.o
iceDriver-objs += src/stateMachine.o

EXTRA_CFLAGS=-I$(PWD)/include

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules -j

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean -j
