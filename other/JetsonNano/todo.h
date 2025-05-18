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
// Software Versions in JetPack 4.6.2 (L4T R32.7.2)
//

Component   Version

-> CUDA         10.2
-> cuDNN        8.2.1
-> TensorRT     8.2.1
-> VPI          1.1
-> VisionWorks  1.6
-> OpenCV       4.5.4
-> Vulkan       1.2 (via Mesa)
-> L4T (Linux)  R32.7.2
-> Ubuntu       18.04
-> Python       3.6 (default)

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
