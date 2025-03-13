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
