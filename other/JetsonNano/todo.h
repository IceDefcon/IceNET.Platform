//
// Sublime-text
//
wget -qO - https://download.sublimetext.com/sublimehq-pub.gpg | sudo gpg --dearmor -o /usr/share/keyrings/sublimehq-pub.gpg
echo "deb [signed-by=/usr/share/keyrings/sublimehq-pub.gpg] https://download.sublimetext.com/ apt/stable/" | sudo tee /etc/apt/sources.list.d/sublime-text.list
sudo apt update
sudo apt install sublime-text

    "trim_trailing_white_space_on_save": true,
    "ensure_newline_at_eof_on_save": true,
    "translate_tabs_to_spaces": true,
    "tab_size": 4,
    "auto_indent": true,
    "detect_indentation": true,
    "trim_only_modified_white_space": true,
    "ensure_single_trailing_newline": true,

sudo apt-get install sublime-merge

//
// Cmake + Ninja
//
sudo apt update
sudo apt-get install cmake ninja-build -y

//
// Pip
//
sudo apt update
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
sudo apt update
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
