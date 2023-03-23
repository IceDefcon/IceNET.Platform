# IceNET.NetworkAnalyser
Detect and Report Security Anomalies inside Computer Network
----------------------------------------------------------------

Current Hardware configuration:

	- EP2C5T144C8N Cyclone II FPGA
	- FRDM-K64F Platform ---> https://github.com/IceDefcon/K64F.FreeRTOS
	- x64 Linux Terminal ---> TCP Server for SQL control

Intel Altera provide a support for Cyclon II FPGA in Quartus 13.0ps1

System is running without any problems on Machine:

	Architecture:        x86_64
	CPU op-mode(s):      32-bit, 64-bit
	Byte Order:          Little Endian
	CPU(s):              4
	On-line CPU(s) list: 0-3
	Thread(s) per core:  2
	Core(s) per socket:  2
	Socket(s):           1
	NUMA node(s):        1
	Vendor ID:           GenuineIntel
	CPU family:          6
	Model:               142
	Model name:          Intel(R) Core(TM) i7-7500U CPU @ 2.70GHz
	Stepping:            9
	CPU MHz:             882.018
	CPU max MHz:         3500,0000
	CPU min MHz:         400,0000
	BogoMIPS:            5799.77
	Virtualization:      VT-x
	L1d cache:           32K
	L1i cache:           32K
	L2 cache:            256K
	L3 cache:            4096K
	NUMA node0 CPU(s):   0-3

And operating system:

	NAME="Ubuntu"
	VERSION="18.04.6 LTS (Bionic Beaver)"
	ID=ubuntu
	ID_LIKE=debian
	PRETTY_NAME="Ubuntu 18.04.6 LTS"
	VERSION_ID="18.04"

To install Quartus 13.0sp1 you need

	sudo apt-get install gcc-multilib g++-multilib plocate

To launch quartus in 64-bit mode you need to download libpng12 library

	wget -q http://mirrors.kernel.org/ubuntu/pool/main/libp/libpng/libpng12-0_1.2.54-1ubuntu1_amd64.deb
	dpkg -i /tmp/libpng12.deb

And to launch QSYS more grphical libraries are required

	sudo apt-get install libpng-dev
	sudo apt-get install libxtst-dev
	sudo apt-get install libxi6 libgconf-2-4

Possible those as well:

	sudo apt-get install libxtst6:i386 
	sudo apt install libxi6:i386
	sudo apt install libz-dev:i386

Execution links for 64-bit: 

	cd /home/ice/altera/13.0sp1/quartus/bin
	./quartus --64bit

If there are JTAG problems:

1.Check if Altera USB Blaser is present in the system: 

	$dmesg|tail
	usb 1-2: Product: USB-Blaster
	usb 1-2: Manufacturer: Altera
	usb 1-2: SerialNumber: 00000000
	usb 1-2: USB disconnect, device number 61
	usb 1-2: new full-speed USB device number 62 using xhci_hcd
	usb 1-2: New USB device found, idVendor=09fb, idProduct=6001, bcdDevice= 4.00
	usb 1-2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
	usb 1-2: Product: USB-Blaster
	usb 1-2: Manufacturer: Altera
	usb 1-2: SerialNumber: 00000000

2.When: unable to lock chain(Insufficient port permissions)

	sudo killall -9 jtag 
	cd /altera/13.0sp1/quartus/bin/
	./jtagd 
	./jtagconfig

Bare metal drivers and low level sources:

	https://mcuxpresso.nxp.com/en/welcome

3. To drive SPI inside FPGA ---> K64F.FreeRTOS Project must be modified to allow pins of the K64F Board to comunicate with with the Cyclon Board

	https://github.com/IceDefcon/K64F.FreeRTOS

	Inside the Project ---> Threre is an example how to control particular K64F Board LED PIN 

	This configuration can be modified to allow bare metal NXP SPI driver to comunicate with the Cyclon Board ! 

# ICE
