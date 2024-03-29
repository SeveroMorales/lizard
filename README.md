# Lounge Lizard
This guide serves to help install Lounge Lizard in Linux (Ubuntu 22.10) and install a Windows 10 or 11* environment to develop and use Linux GUI Apps, using a windowing system for bitmap displays. 
###### **Note:The installation process has not been tested on Windows 11, but it should work on it by just following this guide, if you desired you can skip the windowing system since it's been integrated into WSL2 in Windows 11* 

### Prerequisites
#### For Windows 10 or 11*
- WSL2 with Ubuntu 22.10
- A Windowing system (Preferably VcXsrv, you can download it [HERE](https://sourceforge.net/projects/vcxsrv/))
- PowerShell with root privileges
#### For Linux
- Packages names can have different names for different Linux distros, if there is any package that is not available in your distro please use [pkgs.org](https://pkgs.org/) to find them
### Setting up a Windowing system for Windows 10
We will use VcXsrv for our windowing system, but you are free to use any alternative. 
Install VcXsrv. create a new desktop shortcut, and use the following command. Add the following command in the properties → shortcut →target
```
"C:\Program Files\VcXsrv\vcxsrv.exe" :0 -ac -terminate -lesspointer -multiwindow -clipboard -wgl -dpi auto
```
Execute the shortcut and use the below command in the prompt to verify it. (execute it on a PowerShell with root privileges)
```
netstat -abno|findstr 6000
```
The output should look something like this:
```
PS C:\WINDOWS\system32> netstat -abno|findstr 6000
  TCP    0.0.0.0:6000           0.0.0.0:0              LISTENING       2372
  TCP    127.0.0.1:6000         127.0.0.1:56804        ESTABLISHED     2372
  TCP    127.0.0.1:6000         127.0.0.1:56805        ESTABLISHED     2372
  TCP    127.0.0.1:6000         127.0.0.1:56806        ESTABLISHED     2372
  TCP    127.0.0.1:56804        127.0.0.1:6000         ESTABLISHED     2372
  TCP    127.0.0.1:56805        127.0.0.1:6000         ESTABLISHED     2372
  TCP    127.0.0.1:56806        127.0.0.1:6000         ESTABLISHED     2372
  TCP    [::]:6000              [::]:0                 LISTENING       2372
  ```
  Now open up your WSL, and install Terminator using the following command:
  ```
sudo apt-get update  
sudo apt-get install terminator
```
Without closing VcXsrv, execute the following command in your WSL to open terminator
```
 DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0 terminator &
 ```
### Building Lounge Lizard in Linux, or Windows 10 or 11* with a windowing system
#### Clone and build the Repository:

1. Install meson as root with the following commands:
```
sudo -i
curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
python3 get-pip.py
python3 -m pip install meson
exit
```
##### After installing meson, restart your WSL or Linux machine
2. In your directory run the following commands:
###### **Note: These are packeges needed to build and develop in Ubuntu 22.10  
``` 
git clone https://github.com/SeveroMorales/lizard.git
cd lizard
sudo apt-get install gettext libjson-glib-dev libgtk-3-dev libgirepository1.0-dev libgtk-4-dev mercurial libgumbo-dev libcmark-dev help2man valac libxml2-dev libgupnp-1.2-dev gupnp-igd-1.0 libgstreamer1.0- dev libgstreamer-plugins-base1.0-dev liblua5.4-dev lua-lgi liblua5.3-dev libperl-dev libglib-object-introspection-perl python-gi-dev libsasl2-dev libcanberra-dev libidn11-dev meson python3-pip libgadu-dev gmime-3.0 libkf5wallet-dev gettext libxml2-dev cmake libsecret-1-dev gi-docgen libavahi-glib-dev libavahi-client-dev sassc libadwaita-1-dev libmeanwhiledev qttools5-dev-tools libgirepository1.0-dev libgtk-4-dev libadwaita-1-dev libgumbo-dev libcmark-dev help2man valac libxml++2.6-dev libsoup-3.0-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev gstreamer1.0-plugins-good libgstreamer-plugins-good1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-libav libidn-dev libsecret-1-dev
 ```
 3. Now run these commands:
 ```
 meson setup build
 cd build
 ninja
 sudo ninja install
 ```
 4. In order to run Lounge Lizard, run the following commands:
 ```
  cd build/pidgin/
 ./pidgin3
 ```
 
5. Now you need to set up and account and use Lounge Lizard

Also the ui-testing branch is for new feature, if you can switch to that branch and try our new features. Just remember to run ninja again
