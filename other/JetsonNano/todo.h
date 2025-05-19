//
// Sublime-text
//
wget -qO - https://download.sublimetext.com/sublimehq-pub.gpg | sudo gpg --dearmor -o /usr/share/keyrings/sublimehq-pub.gpg
echo "deb [signed-by=/usr/share/keyrings/sublimehq-pub.gpg] https://download.sublimetext.com/ apt/stable/" | sudo tee /etc/apt/sources.list.d/sublime-text.list
sudo apt update
sudo apt install sublime-text -y

    "trim_trailing_white_space_on_save": true,
    "ensure_newline_at_eof_on_save": true,
    "translate_tabs_to_spaces": true,
    "tab_size": 4,
    "auto_indent": true,
    "detect_indentation": true,
    "trim_only_modified_white_space": true,
    "ensure_single_trailing_newline": true,

sudo apt-get install sublime-merge -y

//
// Cmake + Ninja
//
sudo apt-get install cmake ninja-build -y

//
// Pip
//
sudo apt install python3-pip -y

//
// jtop
//
sudo -H pip3 install --no-cache-dir -U jetson-stats

//
// Git
//
git config --global user.email "ice.defcon@yahoo.com"
git config --global user.name "IceDefcon"
git config --global credential.helper 'store --file ~/.git-credentials'

//
// Qt5
//
sudo apt-get install qt5-qmake qtbase5-dev libqt5serialport5-dev -y

//
// Install
//
apt-get install guake nano -y

//
// Bash
//
mkdir /tmp/runtime-root
chmod 777 /tmp/runtime-root -R
cd /home/ice/code.lab/IceNET.Platform
alias push_droneAI="git push origin droneAI"
export XDG_RUNTIME_DIR="/tmp/runtime-root"
export PATH=/usr/lib/qt5/bin:$PATH

//
// Enable SPI trough device tree
//
cp nano-spi.dtb /boot/

//
// Then Modify
//
/boot/extlinux/extlinux.conf

LABEL primary
      MENU LABEL primary kernel
      LINUX /boot/Image
      INITRD /boot/initrd
      FDT /boot/nano-spi.dtb
      APPEND ${cbootargs} quiet root=/dev/mmcblk0p1 rw rootwait rootfstype=ext4 console=ttyS0,115200n8 console=tty0 fbcon=map:0 net.ifnames=0 sdhci_tegra.en_boot_part_access=1

//
// Install openCV with CUDA support
//
sudo apt-get update
sudo apt-get install -y cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install -y python3-dev python3-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev
sudo apt-get install -y libdc1394-22-dev libv4l-dev v4l-utils libopenblas-dev libatlas-base-dev libblas-dev
sudo apt-get install -y libxvidcore-dev libx264-dev libgtk-3-dev libcanberra-gtk* libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev

mkdir ~/opencv_build && cd ~/opencv_build
git clone https://github.com/opencv/opencv.git
git clone https://github.com/opencv/opencv_contrib.git
cd opencv
git checkout 4.5.4
cd ../opencv_contrib
git checkout 4.5.4

cd ~/opencv_build/opencv
mkdir build && cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
      -D CMAKE_INSTALL_PREFIX=/usr/local \
      -D OPENCV_EXTRA_MODULES_PATH=~/opencv_build/opencv_contrib/modules \
      -D WITH_CUDA=ON \
      -D ENABLE_FAST_MATH=1 \
      -D CUDA_FAST_MATH=1 \
      -D WITH_CUBLAS=1 \
      -D WITH_LIBV4L=ON \
      -D BUILD_opencv_python3=ON \
      -D WITH_GSTREAMER=ON \
      -D BUILD_EXAMPLES=OFF ..

make -j4
sudo make install
sudo ldconfig

//
// JetPack 4.6.1 -> L4T 32.7.1
//
-> CUDA 10.2
-> cuDNN 8.2.1
-> TensorRT 8.2.1
-> VPI 1.2
-> VisionWorks 1.6
-> OpenCV 4.1.1

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// cmake with OpenSSL
//
sudo apt remove cmake

sudo apt update
sudo apt install -y build-essential libssl-dev

cd ~
wget https://github.com/Kitware/CMake/releases/download/v3.13.4/cmake-3.13.4.tar.gz
tar -zxvf cmake-3.13.4.tar.gz
cd cmake-3.13.4

./bootstrap
make -j$(nproc)
sudo make install

//
// TensorRT
//
git clone -b release/8.2 https://github.com/NVIDIA/TensorRT.git
cd TensorRT

sudo apt update
sudo apt install -y \
    cmake \
    libprotobuf-dev protobuf-compiler \
    libnvinfer-dev libnvonnxparsers-dev libnvparsers-dev \
    python3-dev python3-pip \
    swig libopencv-dev

pip3 install onnx

cd TensorRT
mkdir build && cd build
cmake .. -DGPU_ARCHS="53" -DCUDA_VERSION="10.2" -DCUDNN_VERSION="8.2"
make -j$(nproc)

sudo make install

//
// Env
//
setenv boot_targets mmc0
setenv boot_targets mmc1
setenv boot_targets usb0
setenv boot_targets "usb0 mmc1"

//
// x86 OpenCV + GStream
//
sudo apt update
sudo apt install -y build-essential cmake git pkg-config \
    libgtk-3-dev libavcodec-dev libavformat-dev libswscale-dev \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    python3-dev python3-numpy libtbb2 libtbb-dev libjpeg-dev \
    libpng-dev libtiff-dev libdc1394-22-dev

cd ~
git clone https://github.com/opencv/opencv.git
git clone https://github.com/opencv/opencv_contrib.git

cd ~/opencv
mkdir build
cd build

cmake -D CMAKE_BUILD_TYPE=Release \
      -D CMAKE_INSTALL_PREFIX=/usr/local \
      -D WITH_GSTREAMER=ON \
      -D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib/modules \
      ..

make -j$(nproc)
sudo make install
sudo ldconfig

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Load System from SD card
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Prepare SD card
//
mount /dev/sdb1 /mnt
sudo mount bootloader/system.img.raw ./tmp_system
sudo rsync -axHAWX --numeric-ids --info=progress2 --exclude=/proc ./tmp_system/ /mnt
sudo mkdir -p /mnt/{proc,sys,dev,tmp,run}
sudo chmod 755 /mnt/{proc,sys,dev,tmp,run}
sudo umount /mnt
sudo umount ./tmp_system

//
// Bad
//
[    0.000000] Booting Linux on physical CPU 0x0
[    0.000000] Linux version 4.9.253-tegra (buildbrain@mobile-u64-5494-d8000) (gcc version 7.3.1 20180425 [linaro-7.3-2018.05 revision d29120a424ecfbc167ef90065c0eeb7f91977701] (Linaro GCC 7.3-2018.05) ) #1 SMP PREEMPT Wed Apr 20 14:25:12 PDT 2022
[    0.000000] Boot CPU: AArch64 Processor [411fd071]
[    0.000000] OF: fdt:memory scan node memory@80000000, reg size 32,
[    0.000000] OF: fdt: - 80000000 ,  3f200000
[    0.000000] OF: fdt: - c0000000 ,  3ee00000
[    0.000000] Found tegra_fbmem: 00800000@92cb4000
[    0.000000] earlycon: uart8250 at MMIO32 0x0000000070006000 (options '')
[    0.000000] bootconsole [uart8250] enabled

//
// Good
//
[    0.000000] Booting Linux on physical CPU 0x0
[    0.000000] Linux version 4.9.253-tegra (buildbrain@mobile-u64-5494-d8000) (gcc version 7.3.1 20180425 [linaro-7.3-2018.05 revision d29120a424ecfbc167ef90065c0eeb7f91977701] (Linaro GCC 7.3-2018.05) ) #1 SMP PREEMPT Wed Apr 20 14:25:12 PDT 2022
[    0.000000] Boot CPU: AArch64 Processor [411fd071]
[    0.000000] OF: fdt:memory scan node memory@80000000, reg size 32,
[    0.000000] OF: fdt: - 80000000 ,  7ee00000
[    0.000000] OF: fdt: - 100000000 ,  7f200000
[    0.000000] Found tegra_fbmem: 00800000@92cb4000
[    0.000000] earlycon: uart8250 at MMIO32 0x0000000070006000 (options '')
[    0.000000] bootconsole [uart8250] enabled
[    1.162457] tegradc tegradc.1: dpd enable lo

//
// Nvidia Download ->
//
https://developer.nvidia.com/embedded/jetson-linux-archive
https://developer.nvidia.com/embedded/linux-tegra-r3276
https://docs.nvidia.com/jetson/archives/l4t-archived/l4t-3275/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/quick_start.html#

https://www.waveshare.com/wiki/JETSON-NANO-DEV-KIT#System_Environment_.26_EMMC_System_Programming

//
//
//
sudo tar -xjf Jetson-210_Linux_R32.7.2_aarch64.tbz2
cd Linux_for_Tegra/rootfs/
sudo tar -xjf ../../Tegra_Linux_Sample-Root-Filesystem_R32.7.2_aarch64.tbz2
cd ../
sudo ./apply_binaries.sh (If an error occurs, follow the prompts and re-enter the instruction).

cd ..
wget https://developer.nvidia.com/downloads/embedded/L4T/r32_Release_v7.5/overlay_32.7.5_PCN211181.tbz2
sudo tar -xjf overlay_32.7.5_PCN211181.tbz2

sudo ./flash.sh jetson-nano-emmc mmcblk0p1

//
// More ideas
//
TIMEOUT 30
DEFAULT FLASH

MENU TITLE L4T boot options

LABEL FLASH
      MENU LABEL FLASH
      LINUX /boot/Image
      INITRD /boot/initrd
      APPEND ${cbootargs} quiet root=/dev/mmcblk0p1 rw rootwait rootfstype=ext4 console=ttyS0,115200n8 console=tty0 fbcon=map:0 net.ifnames=0 sdhci_tegra.en_boot_part_access=1

LABEL SD
      MENU LABEL SD
      LINUX /boot/Image
      INITRD /boot/initrd
      FDT /boot/nano-spi.dtb
      APPEND ${cbootargs} quiet root=UUID=fe3d7b93-91e9-469a-a245-3a0a52fe179c rw rootwait rootfstype=ext4 console=ttyS0,115200n8 console=tty0 fbcon=map:0 net.ifnames=0 sdhci_tegra.en_boot_part_access=1

//
// Update @ SD -> UUID in fstab
//
UUID=da0c2cea-40e1-4b54-914d-63b79432351a / ext4 defaults 0 1
