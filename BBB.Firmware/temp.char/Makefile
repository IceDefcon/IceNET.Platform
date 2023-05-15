#
# Author: Ice.Marek
# 2023 IceNET Technology
#

obj-m+=iceCOM.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm $(APP)

