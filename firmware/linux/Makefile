obj-m += mymodule.o

mymodule-objs := ./src/mymodule.o ./src/mymodule_func.o

all:
	make -C $(KERNEL_PATH) M=$(PWD) modules

clean:
	make -C $(KERNEL_PATH) M=$(PWD) clean