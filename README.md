# Lounge Lizard
This guide serves to help install Lounge Lizard in Linux (Ubuntu 22.10) and install a Windows 10 or 11* environment to install and develop Linux GUI Apps, using a windowing system for bitmap displays. 
###### **Note:The installation process has not been tested on Windows 11, but it should work on it by just following this guide, if you desired you can skip the windowing system since it's been integrated into WSL2 in Windows 11* 

### Prerequisites
#### For Windows 10 or 11*
- WSL2 with Ubuntu 22.10
- A Windowing system (Preferably VcXsrv, you can download it [HERE](https://sourceforge.net/projects/vcxsrv/))
- PowerShell with root privileges
#### For Linux
- Packages names can have different names for different Linux distros, if there is any package that is not available in your distro please use [pkgs.org](https://pkgs.org/) to find the package name for your distro
### Setting up a Windowing system for Windows 10
We will use VcXsrv for our windowing system, but you are free to use any alternative. 
Install VcXsrv there is no special instruction needed to reach the final. After installation, create a new desktop shortcut, and use the following command. Add the following command in the properties → shortcut →target
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
#### Clone the Repository:
1. In your desired directory run the following commands:
 ``` 
sudo apt-get install gettext libjson-glib-dev libgtk-3-dev libgirepository1.0-dev libgtk-4-dev mercurial libgumbo-dev libcmark-dev help2man valac libxml2-dev libgupnp-1.2-dev gupnp-igd-1.0 libgstreamer1.0- dev libgstreamer-plugins-base1.0-dev liblua5.4-dev lua-lgi liblua5.3-dev libperl-dev libglib-object-introspection-perl python-gi-dev libsasl2-dev libcanberra-dev libidn11-dev meson python3-pip libgadu-dev gmime-3.0 libkf5wallet-dev gettext libxml2-dev cmake libsecret-1-dev gi-docgen libavahi-glib-dev libavahi-client-dev sassc libadwaita-1-dev libmeanwhiledev qttools5-dev-tools meson libgirepository1.0-dev
 ```
