obj-m += iceControler.o

iceControler-objs := \
    src/transmiter.o \
    src/main.o

EXTRA_CFLAGS = -I$(PWD)/include

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
