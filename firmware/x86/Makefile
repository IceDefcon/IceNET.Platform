obj-m += iceControler.o

iceControler-objs := \
    src/main.o \
    src/mainThread.o \
    src/x86network.o \
    src/diagnostics.o \
    src/kernelComms.o \
    ../common/src/memory.o \
    ../common/src/crypto.o \
    ../common/src/receiver.o \
    ../common/src/transmitter.o

EXTRA_CFLAGS = -I$(PWD)/include -I$(PWD)/../common/include

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f ../common/src/*.o ../common/src/.*.o.cmd
