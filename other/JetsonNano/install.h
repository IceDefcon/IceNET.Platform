sudo apt update
sudo apt install gitk guake picocom make git net-tools nasm g++ curl dirmngr gnupg htop guake libncurses-dev

sudo apt install wireshark-qt
sudo apt install -y aircrack-ng

apt-transport-https ca-certificates software-properties-common sublime-text -y
curl -fsSL https://download.sublimetext.com/sublimehq-pub.gpg | sudo apt-key add -
sudo add-apt-repository "deb https://download.sublimetext.com/ apt/stable/"

sudo apt install sublime-text

echo "deb https://download.sublimetext.com/ apt/stable/" | sudo tee /etc/apt/sources.list.d/sublime-text.list
sudo apt-get update
sudo apt-get install sublime-merge

git config --global user.email "ice.defcon@yahoo.com"
git config --global user.name "IceDefcon"
git config --global credential.helper 'store --file ~/.git-credentials'

sudo apt-get install python3-tk
sudo apt install python3-pip
sudo pip3 install pyserial
