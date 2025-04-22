obj-m += iceControler.o

iceControler-objs := \
    src/crypto.o \
    src/main.o \
    src/mainThread.o \
    src/networkControl.o \

EXTRA_CFLAGS = -I$(PWD)/include

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
