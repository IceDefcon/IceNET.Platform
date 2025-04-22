# CPU & FPGA Embedded Computer Platform

	Artificial Inteligence Drone :: Manual Override

# Current Hardware configuration

	Quartus :: 22.1std.1 Build 917 02/14/2023 SC Lite Edition
	Cyclone IV :: EP4CE15F23C8N
	Cortex-A57 :: 4.9.253-tegra Kernel Module
	Cortex-A57 :: Qt5 GUI & System Application for Master Control

# TODO Hardware

	00. Order LTE module for high speed radio troughput :: Video transmission
	01. Order J-Link Cable :: SEGGER
	02. Attach to CPU @ Ozone Debuger
	03. Design Powered PCB for bus control

# TODO FPGA

	00. Reanalyze Reset Network in FPGA
	01. Reset modules :: Done !
	02. Empty FIFO's
	03. Reconfigure Peripherals :: Reset them through registers

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

# NEXT Task :: Network Stack -> R&D

	-----===[Top Required]===-----

	00. Order Jetson Nano 4K Camera (or two Cameras)
	01. Order SD Card -> 128GB for Accelerated Libraries (Additional @ GPU Support)
	02. Develop Network Stack Controllers for Video Transportation (Investigation of Network Stack Layers)
	03. Send 4K Video from Jetson Nano over Wi-Fi to x86_64 Machines (Optionally use LTE/5G Modules)
	04. x86_64 Machines connected to the same Network receive Drone 4K Video
	05. Develop Backtrack x86_64 Network Stack Video Receive Controller (Again, All Network Stack Layers :: Both Kernel/User Space)
	06. Display Drone Video on x86_64 Authenticated Machines

# CURRENT Task :: Gravitational Assist -> Try 10MHz @ SPI (Check the Noise)

	Already Done!!!

	00. Initialize system threads
	01. Send sensors configuration to FPGA
	02. Send Offload Vector from CPU to FIFO (Then wait for ACK from FPGA)
	03. Enable Pulse Controllers to begin acquisition of data from SPI sensors

	Need to be Computed

	04. Double-check if SPI Controller has burst all required data from Sensors
	05. Store data from all sensors in FIFO (Due to overlapping interrupt signals to Kernel)
	06. Reconfigure SPI/DMA on the secondary side (concatenated sensor data)
	07. Initiate SPI/DMA Transfer :: Possible IRQ Vector @ FPGA->CPU Direction
	08. Initiate SPI/DMA Transfer :: Optional Kernel ISR triggered by GPIO
	09. Send Data from Kernel to User space
	10. Display data @ GUI
	11. Develop Transfer Function for the PID Controllers based on acquisition from sensors
	12. Accelerate Transfer Function using 128 CUDA Core GPU
	13. Compute feedback to FPGA for PWM Motor controllers
	14. Extremely Fast Feedback for each arm of the HEX-Drone ???
	15. Are we up and flying???

# Testing Hardware Modules

	HC-12 - Radio Module 433MHz
	RTC DS1307 - Real time clock
	SYN115 - Radio Module 433MHz
	L3G4200D - Gyroscope Module

# Most Recent TODO

	00. Finish Sensor calibration
	01. Network stack architecture
	02. Char device for Master Controller
	03. Ram Disk Block device for Master Controller

# ICE
