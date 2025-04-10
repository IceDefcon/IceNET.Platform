//
// Beagle bone fresh installation
//

sudo apt update
sudo apt install gawk wget git-core diffstat unzip texinfo gcc-multilib build-essential chrpath socat cpio python3 python3-pip python3-pexpect xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev xterm

mkdir ~/yocto.bbb
cd ~/yocto.bbb

git clone -b kirkstone git://git.yoctoproject.org/poky.git

cd poky

source oe-init-build-env

nano conf/local.conf

// Verion 1
MACHINE = "beaglebone-yocto"
BB_NUMBER_THREADS = "4"
PARALLEL_MAKE = "-j4"
IMAGE_FSTYPES = "tar.xz ext4"
// Version 2
MACHINE = "beaglebone-yocto"
DISTRO = "poky"
PARALLEL_MAKE = "-j4"

nano conf/bblayers.conf
// Version 1
BBLAYERS ?= " \
  ${TOPDIR}/../poky/meta \
  ${TOPDIR}/../poky/meta-poky \
  ${TOPDIR}/../poky/meta-yocto-bsp \
  ${TOPDIR}/../meta-openembedded/meta-oe \
  ${TOPDIR}/../meta-openembedded/meta-networking \
  ${TOPDIR}/../meta-openembedded/meta-python \
  ${TOPDIR}/../meta-openembedded/meta-filesystems \
  ${TOPDIR}/../meta-ti \
  ${TOPDIR}/../meta-beagleboard \
"
// Version 2
BBLAYERS ?= " \
  /home/ice/yocto.bbb/poky/meta \
  /home/ice/yocto.bbb/poky/meta-poky \
  /home/ice/yocto.bbb/poky/meta-yocto-bsp \
  "
// Build
bitbake core-image-minimal
// Output
tmp/deploy/images/beaglebone-yocto/

//
// Prepare to load
//

200M W95 FAT32  :: sudo mkfs.vfat -F 32 -n "BOOT" /dev/sdc1
14.4G Linux     :: sudo mkfs.ext4 -L "ROOTFS" /dev/sdc2

[ice][ice] $ ls boot/
total 9200
-rw-r--r-- 1 ice ice   67160 kwi  6  2011 am335x-boneblack.dtb
-rw-r--r-- 1 ice ice   63710 kwi  6  2011 am335x-bone.dtb
-rw-r--r-- 1 ice ice   63974 kwi  6  2011 am335x-bonegreen.dtb
drwxr-xr-x 2 ice ice    2048 kwi  6  2011 extlinux
-rw-r--r-- 1 ice ice  107932 kwi  6  2011 MLO
-rw-r--r-- 1 ice ice 1224988 kwi  6  2011 u-boot.img
-rw-r--r-- 1 ice ice 7884728 kwi  6  2011 zImage

[ice][ice] $ ls root/
total 28
drwxr-xr-x  2 root root  3072 mar  9  2018 bin
drwxr-xr-x  2 root root  1024 mar  9  2018 boot
drwxr-xr-x  2 root root  1024 mar  9  2018 dev
drwxr-xr-x 18 root root  1024 mar  9  2018 etc
drwxr-xr-x  3 root root  1024 mar  9  2018 home
drwxr-xr-x  3 root root  1024 mar  9  2018 lib
drwx------  2 root root 12288 mar 12 02:29 lost+found
drwxr-xr-x  2 root root  1024 mar  9  2018 media
drwxr-xr-x  2 root root  1024 mar  9  2018 mnt
dr-xr-xr-x  2 root root  1024 mar  9  2018 proc
drwxr-xr-x  2 root root  1024 mar  9  2018 run
drwxr-xr-x  2 root root  1024 mar  9  2018 sbin
dr-xr-xr-x  2 root root  1024 mar  9  2018 sys
lrwxrwxrwx  1 root root     8 mar  9  2018 tmp -> /var/tmp
drwxr-xr-x  9 root root  1024 mar  9  2018 usr
drwxr-xr-x  8 root root  1024 mar  9  2018 var
