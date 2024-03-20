obj-m += iceDriver.o

# Source files
iceDriver-objs := charDevice.o spiDevice.o

all:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules -j

clean:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean -j
