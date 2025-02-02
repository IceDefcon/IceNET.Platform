obj-m += iceDriver.o

iceDriver-objs := src/charDevice.o src/charDeviceCfg.o src/isrCtrl.o src/main.o src/spiDmaCtrl.o src/spiWork.o src/stateMachine.o src/watchdog.o src/console.o src/ramDisk.o src/ramAxis.o src/scheduler.o

EXTRA_CFLAGS=-I$(PWD)/include

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
