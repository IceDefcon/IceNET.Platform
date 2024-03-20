obj-m := iceDriver_out.o
klm_out-objs : iceDriver.o charDevice.o

modules:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
