obj-m += iceDriver.o

iceDriver-objs := \
    src/charDevice.o \
    src/charDeviceCfg.o \
    src/isrCtrl.o \
    src/main.o \
    src/spiDmaCtrl.o \
    src/spiWork.o \
    src/stateMachine.o \
    src/watchdog.o \
    src/scheduler.o \
    src/ramDisk.o \
    src/ramAxis.o \
    src/console.o \
    src/memory.o \
    src/network.o \
    ../common/src/crypto.o \
    ../common/src/receiver.o \
    ../common/src/transmitter.o

EXTRA_CFLAGS = -I$(PWD)/include -I$(PWD)/../common/include

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/../common/src clean
