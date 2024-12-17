# IceNET.Platform :: CPU & FPGA Computer Platform

# Active :: Sub Applications

	1. Artificial Inteligence Drone :: Manual Override
	2. Network Protocol Analyser 	:: Defence Protocol

# Current Hardware configuration

	Quartus :: 22.1std.1.917 :: SC Lite Edition
	Cyclone IV :: EP4CE15F23C8N
	Cortex-A8 :: User Space Controller
	Cortex A8 :: FPGA Kernel Driver
	x86 :: Master Control Application

# TODO Hardware

	1. Attach JTAG Header to the PCB
	2. Boot Kernel trough the JTAG via U-Boot
	3. Customize Root File System
	4. Order J-Link Cable :: SEGGER
	5. Attach to CPU @ Ozone Debuger
	6. Design Powered PCB for bus control

# TODO Software

	0. Block device for chip configuration in FPGA
	1. 8 byte transfer mode for the Platform IO over SPI
	2. Fix up Header in kernel space
	3. Advance parametrization of FIFO
	4. Fix up parametriation of I2C Controller
	5. I2C burst mode
	6. SPI Driver in FPGA with burst mode
	7. UART driver in FPGA
	8. CAN driver in FPGA
	9. Checksum in GUI Application to be send in last byte
	10. Checksum in LNX Module to be send in last byte
	11. Checksum in RTL Code to be send in last byte
	12. UART Console (DMA controller buffer)
	13. Upgrade Console for the functions with arguments

# Testing Hardware Modules

	HC-12 - Radio Module 433MHz
	RTC DS1307 - Real time clock
	SYN115 - Radio Module 433MHz
	L3G4200D - Gyroscope Module

# ICE
