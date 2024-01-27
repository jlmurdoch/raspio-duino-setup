sudo apt update
sudo apt upgrade

Easy way to install XFCE4 with LightDM:
sudo tasksel
[*] Xfce

This will configure lightdm to boot into a graphical mode:
sudo systemctl set-default graphical.target

Apple Aluminium (ISO)
=====================

wget arduino-ide....
unzip arduino-ide...
chmod +x arduino-ide
sudo ln -s /usr/lib/aarch64-linux-gnu/libz.so.1.2.13 /usr/lib/aarch64-linux-gnu/libz.so
sudo apt install fuse avrdude
./arduino-ide_2.2.1_Linux_arm64.AppImage
