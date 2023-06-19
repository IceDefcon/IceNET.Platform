# IceNET.NetworkAnalyser

Hardware firewall to detect potential threats on Ethernet Wire
-

# TODO List

1. Rquire JTAG socket to be soldered to PCB
2. Boot Linux Kernel via U-Boot

# Current Hardware configuration

	Quartus :: 22.1std.1.917 :: SC Lite Edition
	Cyclone IV :: EP4CE15F23C8N
	Cortex-A8 :: Kernel space driver
	Cortex-A8 :: User space Application
	x86 :: Network Stack Comms

# U-Boot Development

Get

	git clone git://git.denx.de/u-boot.git u-boot/

Defaut config :: For BBB

	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- am335x_boneblack_vboot_defconfig

Compile

	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-

# Linux Development

Get

	git clone git://github.com/beagleboard/linux.git

Checkout

	git checkout 6.1.26-ti-rt-r3

Default BBB Config

	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- bb.org_defconfig

Menuconfig

	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
	
Install modules on the RFS

	sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_MOD_PATH=/media/ice/RFS/ modules_install

Compile Kernel :: Using all CPUs

	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- LOADADDR=0x80000000 uImage dtbs -j$(nproc)

Compile kernel modules

	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules

Deploy new Kernel

	cp /ice/code.lab/linux/arch/arm/boot/zImage boot/vmlinuz-6.1.26-ti-rt-r3

Install Kernel modules

	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_MOD_STRIP=1 INSTALL_MOD_PATH=/media/ice/rootfs modules_install

# MODELSIM SIMULATION

	1. Create new project
	2. Add testing module + tb
	3. Compile
	4. Launch new simutaion usign tb

# ICE
