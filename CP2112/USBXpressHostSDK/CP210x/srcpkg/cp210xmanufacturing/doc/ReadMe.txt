CP210xManufacturing Library for CP210x
--------------------------------------

Silicon Labs provides a source package for building
the CP210xManufacturing library for linux.


Uncompressing the source package
--------------------------------

$ tar -xf cp210xmanufacturing_1.0.tar.gz

Source Package Contents
-----------------------

- /common/						- OS abstraction
- /cp210xmanufacturing/include/	- Header files that declare the interface for the CP210xManufacturing library
- /cp210xmanufacturing/src/		- Source files for the manufacturing library
- /Makefile/					- Used to create the library.
- /doc/							- library documentation and rules file.

Dependencies
------------

1. libusb-1.0

On Ubuntu:

  $ sudo apt-get install libusb-1.0-0-dev

2. g++

On Ubuntu:

  $ sudo apt-get install g++

Building libcp210xmanufacturing
-------------------------------

To create the CP210x manufacturing library for the current architecture (either x86_32 or x86_64):
	$ make

To install it at the default location (/usr/local):
	$ sudo make install

Configure Your System to Allow libusb to Access Your Device
-----------------------------------------------------------

CP210xManufacturing uses libusb-1.0 to communicate with the CP210x devices over
USB. In order for CP210xManufacturing to recognize your device, you must first
copy a udev file to /etc/udev/rules.d/. This udev file will automatically
enable read and write permissions for a device with the specified USB vendor
ID. Unplug and replug your device after copying a udev file for the permissions
to take effect. On some systems, it may be necessary to reboot the machine
before the new rules are applied.

CP2101 (VID: 0x10c4 PID: 0xea60):
- Copy SiliconLabs.rules to /etc/udev/rules.d/

Other:
- Modify an existing udev rules file for the required vendor ID

  SUBSYSTEM=="usb", ATTRS{idVendor}=="10c4", MODE="0666"
  SUBSYSTEM=="usb_device", ATTRS{idVendor}=="10c4", MODE="0666"
  
  - or - (optionally add a required product ID)

  SUBSYSTEM=="usb", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea80", MODE="0666"
  SUBSYSTEM=="usb_device", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea80", MODE="0666"
  
- SiliconLabs.rules does not specify the product ID and will therefore allow
  read/write access to all Silicon Labs USB devices with the Silicon Labs VID (0x10c4).
  The product ID is optional and will further restrict which devices the rules file
  will affect.

