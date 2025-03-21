# CPU & FPGA Embedded Computer Platform

	Artificial Inteligence Drone :: Manual Override

# Current Hardware configuration

	Quartus :: 22.1std.1.917 :: SC Lite Edition
	Cyclone IV :: EP4CE15F23C8N
	Cortex-A57 :: 4.9.253-tegra Kernel Module
	Cortex-A57 :: Qt5 GUI & System Application for Master Control

# TODO Hardware

	00. Order LTE module for high speed radio troughput :: Video transmission
	01. Order Nvidia Jetson Nano :: GPU Development
	02. Port BBB Project into Nvidia board @ Quad core Cortex-A57
	03. Attach JTAG Header to the PCB
	04. Boot Kernel trough the JTAG via U-Boot
	05. Customize Root File System
	06. Order J-Link Cable :: SEGGER
	07. Attach to CPU @ Ozone Debuger
	08. Design Powered PCB for bus control

# TODO Software

	00. Advance parametrization of FIFO
	01. Fix up parametriation of I2C Controller
	02. I2C burst mode
	03. SPI Driver in FPGA with burst mode
	04. CAN driver in FPGA
	05. Checksum in RTL Code to be send in last byte
	06. UART Console (DMA controller buffer)
	07. Upgrade Console for the functions with arguments
	08. Upgrade TCP/IP server to DMA Server
	09. Add Diagnostic object in DroneCtrl to monitor memory allocation
	10. Add a Number of Drone Arm Instances in the DroneCtrl unit
	11. Synchronize lengths of feedback interrupts :: I2C, SPI, PWM
	12. Adjust feedback preambles @ SPI transmissions between Kernel<-->FPGA
	13. Remove concatenation of bytes :: Use single Sector 0 for SPI/DMI configuration
	14. New concept of Ram Disk Commander :: Command ---> Sector number + data size + operation + etc...

# Testing Hardware Modules

	HC-12 - Radio Module 433MHz
	RTC DS1307 - Real time clock
	SYN115 - Radio Module 433MHz
	L3G4200D - Gyroscope Module

# ICE
