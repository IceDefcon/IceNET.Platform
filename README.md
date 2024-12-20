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

	00. Order Nvidia Jetson Nano :: GPU Development
	01. Port BBB Project into Nvidia board @ Quad core Cortex-A57
	01. Attach JTAG Header to the PCB
	02. Boot Kernel trough the JTAG via U-Boot
	03. Customize Root File System
	04. Order J-Link Cable :: SEGGER
	05. Attach to CPU @ Ozone Debuger
	06. Design Powered PCB for bus control

# TODO Software

	00. Block device for chip configuration in FPGA
	01. 8 byte transfer mode for the Platform IO over SPI
	02. Fix up Header in kernel space
	03. Advance parametrization of FIFO
	04. Fix up parametriation of I2C Controller
	05. I2C burst mode
	06. SPI Driver in FPGA with burst mode
	07. UART driver in FPGA
	08. CAN driver in FPGA
	09. Checksum in GUI Application to be send in last byte
	10. Checksum in LNX Module to be send in last byte
	11. Checksum in RTL Code to be send in last byte
	12. UART Console (DMA controller buffer)
	13. Upgrade Console for the functions with arguments
	14. Solve 120s wait mutex lock warning
	15. Upgrade TCP/IP server to DMA Server
	16. Architecture Upgrade ---> charDevice Commander + ramDisk DMA Engine

# Testing Hardware Modules

	HC-12 - Radio Module 433MHz
	RTC DS1307 - Real time clock
	SYN115 - Radio Module 433MHz
	L3G4200D - Gyroscope Module

# ICE
