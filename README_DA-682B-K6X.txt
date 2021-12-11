Reference Zhaoxin's Linux list, this driver is development in Ubuntu version is 18.04.4.
The Ubuntu ISO, ubuntu-18.04.4-live-server-amd64.iso, is available from http://old-releases.ubuntu.com/releases/18.04.4/
Check the distribution-specific information.

	root@k6:~/# lsb_release -a
	No LSB modules are available.
	Distributor ID:	Ubuntu
	Description:	Ubuntu 18.04.4 LTS
	Release:	18.04
	Codename:	bionic

Check the kernel version information.

	root@k6:~/# uname -r
	4.15.0-128-generic

1. Install the development tools in Ubuntu.

	For Ubuntu 18.04
	root@k6:~/# apt-get install build-essential linux-headers-4.15.0-128 gcc-multilib binutils-multiarch git 

	Note: The linux-headers version is dependent on the kernel you have patched.

2. Get the source from git-hub

	root@k6:~/# git clone git://github.com/Moxa-Linux/x86_v3.git x86_k6

3. Build and install the device drivers

	The device drivers for DA-682B-K6 are located in these path.
	3.1, 3.2, 3.3 and 3.4 describes the steps to build and install the device driver in the system.

	x86_k6/modules/moxa_misc
	x86_k6/modules/mxser
	x86_k6/DA-720/src/modules/moxa_IRIG-B
	x86_k6/DA-720/src/modules/moxa_it87_wdt
	x86_k6/DA-720/src/user/irigb
	x86_k6/DA-682B-K6X/romfs/etc/udev/rules.d/70-da-682b-k6x-ubuntu1104-persistent-net.rules

3.1. Build and install the relay, programmable LED device driver

	Set the CONFIG_PRODUCT=da682bk6 in Makefile

	root@k6:~/# cd x86_k6/modules/moxa_misc
	root@k6:~/# vi Makefile
	
	#CONFIG_PRODUCT?=mpc21xx
	#CONFIG_PRODUCT?=mc1100
	#CONFIG_PRODUCT?=expc1519
	#CONFIG_PRODUCT?=da720
	CONFIG_PRODUCT?=da682bk6
	#CONFIG_PRODUCT?=mc7200
	#CONFIG_PRODUCT?=mc7400

	Build the device driver

	root@k6:~/x86_k6/modules/moxa_misc# make

	Install the device driver

	root@k6:~/x86_k6/modules/moxa_misc# make install


3.2. Build and install the Moxa serial port device driver

	Build the Moxa serial port device driver
	
	root@k6:~/x86_k6/modules/moxa_misc# cd ../mxser/
	root@k6:~/x86_k6/modules/mxser# make

	Install the Moxa serial port device driver

	root@k6:~/x86_k6/modules/mxser# make install

3.3. Build and install the Moxa IRIG-B device driver and utility

	Build the Moxa IRIG-B device driver
	
	root@k6:~/x86_k6/modules/mxser# cd ../../DA-720/src/modules/moxa_IRIG-B/
	root@k6:~/x86_k6/DA-720/src/modules/moxa_IRIG-B# make

	Install the Moxa IRIG-B device driver

	root@k6:~/x86_k6/DA-720/src/modules/moxa_IRIG-B# make install

	Build the IRIG-B utility

	root@k6:~/x86_k6/DA-720/src/modules/moxa_IRIG-B# cd ../../user/irigb/
	root@k6:~/x86_k6/DA-720/src/user/irigb# make

	Install the IRIG-B utility

	root@k6:~/x86_k6/DA-720/src/user/irigb# make install
	
3.4. Build and install the watchdog device driver (Optional)

	If the Linux system doesn't support it8786_wdt.ko, you can install the device driver for this system.

	root@k6:~/x86_k6/DA-720/src/modules/moxa_it87_wdt# make

	Install the watchdog device driver

	root@k6:~/x86_k6/DA-720/src/modules/moxa_it87_wdt# make install

	Install the watchdogd from the repository.

	root@k6:~/x86_k6/DA-720/src/modules/moxa_it87_wdt# apt-get install watchdog

	Configure the watchdog.conf

	root@k6:~ vi /etc/watchdog.conf
	...
	repair-timeout		= 29
	...
	priority		= 1
	...
	watchdog-device	= /dev/watchdog

	
	Reboot or restart the watchdog service

	root@k6:~# service  watchdog stop
	root@k6:~# service  watchdog start

3.5. Remapping the order of LAN ports

	The UDEV rule is used to remap the LAN order as the label on the case. You can put it to /etc/udev/rules.d/.

	root@k6:~# cp -a x86_k6/DA-682B-K6X/romfs/etc/udev/rules.d/70-da-682b-k6x-ubuntu1104-persistent-net.rules /etc/udev/rules.d/

	Then reboot to apply the udev rule

	root@k6:~# /sbin/reboot
