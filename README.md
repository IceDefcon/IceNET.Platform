# IceNET.NetworkAnalyser

Hardware firewall to detect, report and neutralise security threats inside computer networks
-

# U-Boot Development

//////////////////////////
// 						//
// 						//
// 						//
// 	Get and Prepare 	//
// 						//
// 						//
// 						//
//////////////////////////
//
// Get
//
git clone git://git.denx.de/u-boot.git u-boot/
//
// Defaut config :: For BBB
//
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- am335x_boneblack_vboot_defconfig
//
// Compile
//
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-

# Linux Development

//////////////////////////
// 						//
// 						//
// 						//
// 	Get and Prepare 	//
// 						//
// 						//
// 						//
//////////////////////////
//
// Download
//
git clone git://github.com/beagleboard/linux.git

//
// Checkout
//
git checkout 6.1.26-ti-rt-r3

//
// Default BBB Config
//
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- bb.org_defconfig

//
// Menuconfig
//
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig

sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_MOD_PATH=/media/ice/RFS/ modules_install

//////////////////////////
// 						//
// 						//
// 						//
// 	Compile Kernel 		//
// 	Compile Modules 	//
// 	And Device Tree 	//
// 						//
// 						//
// 						//
//////////////////////////
//
// Using all CPUs
// 
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- LOADADDR=0x80000000 uImage dtbs -j$(nproc)

//
// Compile kernel modules
//
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules

//////////////////////////
// 						//
// 						//
// 						//
// 	Deploy new Kernel 	//
// 						//
// 						//
// 						//
//////////////////////////
//
// Copy new image to SD
//
cp /ice/code.lab/linux/arch/arm/boot/zImage boot/vmlinuz-6.1.26-ti-rt-r3
//
// Install Kernel modules
//
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_MOD_STRIP=1 INSTALL_MOD_PATH=/media/ice/rootfs modules_install

# FPGA

Current Hardware configuration:

	- EP2C5T144C8N Cyclone II FPGA
	- BBB driven by Cortex-A8 :: Linux kernel
	- Overall master controll application

Intel Altera provide a support for Cyclon II FPGA in Quartus 13.0ps1

To create symbolic link

	cd /ice/q13//quartus/bin
	ln -s $(pwd)/quartus /usr/local/bin/quartus

To install Quartus 13.0sp1 you need

	sudo apt-get install gcc-multilib g++-multilib

To launch quartus in 64-bit mode you need to download libpng12 library

	wget -q http://mirrors.kernel.org/ubuntu/pool/main/libp/libpng/libpng12-0_1.2.54-1ubuntu1_amd64.deb
	dpkg -i /tmp/libpng12.deb

	Or this one:

	sudo add-apt-repository ppa:linuxuprising/libpng12
	sudo apt update
	sudo apt install libpng12-0

And to launch QSYS

	sudo apt-get install libpng-dev
	sudo apt-get install libxtst-dev
	sudo apt-get install libxi6 libgconf-2-4
	sudo apt-get install libxtst6:i386 
	sudo apt install libxi6:i386
	sudo apt install libz-dev:i386

QUARTUS Execution links for 64-bit: 

	cd ../quartus/bin
	./quartus --64bit

To check if QSYS is working or require some dependant libraries you can execute

	cd ../quartus/sopc_builder/bin
	./qsys-edit

MODELSIM Application

	cd ../modelsim_ase/bin
	./vsim

Possibly need these

	sudo apt install libxft-dev:i386
	sudo apt install libncurses-dev:i386
	
	sudo add-apt-repository universe
	sudo apt-get install libncurses5 libncurses5:i386

For older kernels with:: Read failure in vlm process (0,0)

	1. For 13.1ps1 Download ---> freetype-2.4.12
	2. Configure
	   ./configure --build=i686-pc-linux-gnu "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32"
	3. If everything good ---> make
	   make -j8

	4. Modify vsim ---> put after this line another one
	   dir=`dirname $arg0`
	   export LD_LIBRARY_PATH=${dir}/lib32

	5. Also check your kernrel ---> uname -r
	   then for example ---> 5.4.0-146-generic
	   modfy vsim intoadditional 5*

		case $utype in
		2.4.[7-9]*)       vco="linux" ;;
		2.4.[1-9][0-9]*)  vco="linux" ;;
		2.[5-9]*)         vco="linux" ;;
		2.[1-9][0-9]*)    vco="linux" ;;
		3.[0-9]*)    		vco="linux" ;;
		5.[0-9]*)     vco="linux" ;;
		*)                vco="linux_rh60" ;;

	6. Then copy this so library into credetd "lib32" folder in the modelsim_ase folder ! 
	   cp objs/.libs/libfreetype.so* ../../../altera/13.1/modelsim_ase/lib32/

	7. This should be if it should work now !!! enjoy

If there are JTAG problems:

Check if Altera USB Blaser is present in the system: 

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

When: unable to lock chain(Insufficient port permissions)

	sudo killall -9 jtag 
	cd /altera/13.0sp1/quartus/bin/
	./jtagd 
	./jtagconfig

# MODELSIM SIMULATION

	1. Create new project
	2. Add testing module + tb
	3. Compile
	4. Launch new simutaion usign tb

# ICE
