# IceNET.NetworkAnalyser
Detect and Report Security Anomalies inside Computer Network
----------------------------------------------------------------

Current Hardware configuration:
	- EP2C5T144C8N Cyclone II FPGA
	- FRDM-K64F Platform
	- x64 Linux Terminal

Intel Altera provide a support for Cyclon II FPGA in Quartus 13.0ps1

If the links are not set for the Quartus execution is has to be done manually: 

	cd /home/ice/altera/13.0sp1/quartus/bin
	./quartus --64bit

Also, if JTAG is no set it must be configured manually:

1. Check if Altera USB Blaser is present in the system ---> dmesg|tail

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

2. If we have a problem with ---> unable to lock chain(Insufficient port permissions)

	sudo killall -9 jtag 
	cd /altera/13.0sp1/quartus/bin/
	./jtagd 
	./jtagconfig


# ICE
