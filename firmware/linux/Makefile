obj-m += iceDriver.o

mymodule-objs := ./src/iceDriver.o ./src/charDevice.o

all:
	make -C $(KERNEL_PATH) M=$(PWD) modules

clean:
	make -C $(KERNEL_PATH) M=$(PWD) clean