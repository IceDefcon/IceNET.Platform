# IceNET.NetworkAnalyser
Detect and Report Security Anomalies inside Computer Network
----------------------------------------------------------------

Current Hardware configuration:

	- EP2C5T144C8N Cyclone II FPGA
	- FRDM-K64F Platform ---> https://github.com/IceDefcon/K64F.FreeRTOS
	- x64 Linux Terminal ---> TCP Server for SQL control

Intel Altera provide a support for Cyclon II FPGA in Quartus 13.0ps1

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

# PIN CONTROL

	//
	// K64 Sub-Family Reference Manual ---> Example
	//
	//                                          //                      (page number)
	#define SIM_SCGC5 (*(int *)0x40048038u)     // Clock gate 5                 (314)
	#define SIM_SCGC5_PORTB 10                  // Open gate PORTB              (314)
	#define PORTB_PCR21 (*(int *)0x4004A054u)   // Pin Control Register         (277)
	#define PORTB_PCR21_MUX 8                   // Mux "001"                    (282)
	#define GPIOB_PDDR (*(int *)0x400FF054u)    // Port Data Direction Register (1760)
	#define GPIOB_PDOR (*(int *)0x400FF040u)    // Port Data Output Register    (1759)
	#define PIN_N 21                            // PTB21 --> Blue LED  			(1761)

# ICE
