// Deps
sudo apt update
sudo apt install gawk wget git-core diffstat unzip texinfo gcc-multilib build-essential chrpath socat cpio python3 python3-pip python3-pexpect xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev xterm
// Dir
mkdir ~/yocto.bbb
cd ~/yocto.bbb
// Clone
git clone -b kirkstone git://git.yoctoproject.org/poky.git

cd poky
source oe-init-build-env

nano conf/local.conf

MACHINE = "beaglebone-yocto"
DISTRO = "poky"
PARALLEL_MAKE = "-j4"

nano conf/bblayers.conf

BBLAYERS ?= " \
  /home/ice/yocto.bbb/poky/meta \
  /home/ice/yocto.bbb/poky/meta-poky \
  /home/ice/yocto.bbb/poky/meta-yocto-bsp \
  "

// Build
bitbake core-image-minimal
// Output
tmp/deploy/images/beaglebone-yocto/

// Format partitions
sudo mkfs.vfat -F 32 -n BOOT /dev/sdc1
sudo mkfs.ext4 -L ROOTFS /dev/sdc2

// Load boot partition
mkdir -p /mnt/sdboot
sudo mount /dev/sdc1 /mnt/sdboot

sudo cp /home/ice/yocto.bbb/poky/build/tmp/deploy/images/beaglebone-yocto/MLO /mnt/sdboot/
sudo cp /home/ice/yocto.bbb/poky/build/tmp/deploy/images/beaglebone-yocto/u-boot.img /mnt/sdboot/
sudo cp /home/ice/yocto.bbb/poky/build/tmp/deploy/images/beaglebone-yocto/zImage /mnt/sdboot/
sudo cp /home/ice/yocto.bbb/poky/build/tmp/deploy/images/beaglebone-yocto/*.dtb /mnt/sdboot/

sudo umount /mnt/sdboot

// Extract root file system
sudo mkdir -p /mnt/sdroot
sudo mount /dev/sdc2 /mnt/sdroot

sudo tar -xvjf /home/ice/yocto.bbb/poky/build/tmp/deploy/images/beaglebone-yocto/core-image-minimal-beaglebone-yocto.tar.bz2 -C /mnt/sdroot

sudo umount /mnt/sdroot

// uEnv.txt file

bootdir=/
bootfile=zImage

# Set device tree file
fdtfile=am335x-boneblack.dtb
fdtaddr=0x88000000

# Kernel command line arguments
console=ttyO0,115200n8
root=/dev/mmcblk0p2 rw
rootfstype=ext4
rootwait

# Load kernel and device tree
loadzimage=load mmc 0:1 ${loadaddr} ${bootdir}${bootfile}
loadfdt=load mmc 0:1 ${fdtaddr} ${bootdir}${fdtfile}

# Boot command
uenvcmd=run loadzimage; run loadfdt; bootz ${loadaddr} - ${fdtaddr}

