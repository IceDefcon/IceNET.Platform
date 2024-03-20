obj-m += iceDriver.o

iceDriver-objs := src/iceDriver.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules -j

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean -j
