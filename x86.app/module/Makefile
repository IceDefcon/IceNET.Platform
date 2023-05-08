#
# Author: Ice.Marek
# 2023 IceNET Technology
#
TARGET 	= ice
APP 	= app

obj-m+=ice.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) $(APP).c -o $(APP)

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm $(TARGET) $(APP)

