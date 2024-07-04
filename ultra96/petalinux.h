1. petalinux-create -t project -s u96v2_sbc_base_2022_2.bsp
2. Open Vivado and build
3. Export bitstream
4. petalinux-config --get-hw-description hardware/u96v2_sbc_base_2022_2/u96v2_sbc_base.xsa





petalinux-config -c u-boot
petalinux-config -c kernel
petalinux-config -c rootfs

petalinux-build



1. defconfig

System Memory base address 	:: 0x0
System Memory 				:: 0x80000000
Kernel base address 		:: 0x0
u-boot text base address 	:: 0x100000


psu_uart_1
FSBL
AFT
DGT
115200

xilinx_zynqmp_virt_defconfig