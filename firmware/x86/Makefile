obj-m += iceControler.o

iceControler-objs := \
    src/main.o \
    src/mainThread.o \
    src/networkControl.o \
    ../common/src/crypto.o \
    ../common/src/receiver.o \
    ../common/src/transmitter.o

EXTRA_CFLAGS = -I$(PWD)/include -I$(PWD)/../common/include

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f ../common/src/*.o ../common/src/.*.o.cmd
