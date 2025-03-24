# CPU & FPGA Embedded Computer Platform

	Artificial Inteligence Drone :: Manual Override

# Current Hardware configuration

	Quartus :: 22.1std.1.917 :: SC Lite Edition
	Cyclone IV :: EP4CE15F23C8N
	Cortex-A57 :: 4.9.253-tegra Kernel Module
	Cortex-A57 :: Qt5 GUI & System Application for Master Control

# TODO Hardware

	00. Order LTE module for high speed radio troughput :: Video transmission
	01. Order J-Link Cable :: SEGGER
	02. Attach to CPU @ Ozone Debuger
	03. Design Powered PCB for bus control

# TODO Software

	00. Fix up parametriation of I2C Controller
	01. I2C burst mode
	02. SPI Driver in FPGA with burst mode
	03. CAN driver in FPGA
	04. Checksum in RTL Code to be send in last byte
	05. UART Console (DMA controller buffer)
	06. Upgrade Console for the functions with arguments
	07. Add a Number of Drone Arm Instances in the DroneCtrl unit
	08. Synchronize lengths of feedback interrupts :: I2C, SPI, PWM
	09. Adjust feedback preambles @ SPI transmissions between Kernel<-->FPGA
	10. Remove concatenation of bytes :: Use single Sector 0 for SPI/DMI configuration
	11. New concept of Ram Disk Commander :: Command ---> Sector number + data size + operation + etc...

# Testing Hardware Modules

	HC-12 - Radio Module 433MHz
	RTC DS1307 - Real time clock
	SYN115 - Radio Module 433MHz
	L3G4200D - Gyroscope Module

# ICE
