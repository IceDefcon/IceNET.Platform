# IceNET.Platform :: CPU & FPGA Computer Platform

# Active :: Sub Applications

	1. Artificial Inteligence Drone :: Manual Override
	2. Network Protocol Analyser 	:: Defence Protocol

# Current Hardware configuration

	Quartus :: 22.1std.1.917 :: SC Lite Edition
	Cyclone IV :: EP4CE15F23C8N
	Cortex-A8 :: User space for console control
	Cortex A8 :: Kernel space for FPGA driver
	x86 :: Network Stack Comms

# TODO Debug

	1. Solder wires
	2. Order FTR-110-03-G-D-06 :: JTAG Pin Header
	3. Attach JTAG Header to the PCB
	4. Boot Kernel trough the JTAG via U-Boot
	5. Customize Root File System
	6. Order J-Link Cable :: SEGGER
	7. Attach to CPU @ Ozone Debuger

# TODO Software

	1. 8 byte transfer mode tor the Platform IO over SPI
	2. I2C burst mode
	3. SPI Dma controller transfers
	4. SPI Driver in FPGA with burst mode
	5. UART driver in FPGA
	6. CAN driver in FPGA
	7. Checksum for SPI Kerent<->FPGA communication
	8. UART Console (DMA controller buffer)

# Testing Hardware Modules

	HC-12 - Radio Module 433MHz
	RTC DS1307 - Real time clock
	SYN115 - Radio Module 433MHz
	L3G4200D - Gyroscope Module

# ICE
