SLABHIDtoUART Library for CP2110/CP2114
---------------------------------------

Silicon Labs provides a source package for building
the SLABHIDtoUART library for linux.


SLABHIDtoUART linux library release contents
--------------------------------------------

- /common/			- Shared/common source
- /Makefile			- builds & installs the slabhidtouart library.
- /slabhidtouart/	        - Silicon Labs HIDtoUART API source
- /doc/				- library documentation and rules file.
                              

- /doc/FAQ.txt                  - Frequently asked questions regarding building and using
                          qthe SLABHIDtoUART library and HidUartExample.
- /doc/ReadMe.txt               - Explains how to build and use the SLABHIDtoUART library
- /doc/SharedObjectLibraryUsage.txt - Explains how to use libslabhidtouart.so
- /doc/SiliconLabs.rules        - udev rules file for Silicon Labs USB devices
                          (VID = 0x10c4)
- /HidUartExample/	        - An example program that uses the SLABHIDtoUART library.
				  This program also requires the SLABHIDDevice library.

Dependencies
------------

1. libusb-1.0

On Ubuntu:

  $ sudo apt-get install libusb-1.0-0-dev

2. g++

On Ubuntu:

  $ sudo apt-get install g++

Build SLABHIDtoUART
-------------------

To create the SLABHIDtoUART library for the current architecture (either x86_32 or x86_64):
	$ make

To install it at the default location (/usr/local):
	$ sudo make install

Configure Your System to Allow libusb to Access Your Device
-----------------------------------------------------------

SLABHIDtoUART uses HIDAPI to communicate with the CP2110/CP2114 over HID USB.
HIDAPI requires libusb-1.0 in order to communicate with USB devices. In order for
SLABHIDtoUART to recognize your device, you must first copy a udev file to
/etc/udev/rules.d/. This udev file will automatically enable read and write
permissions for a device with the specified USB vendor ID. Unplug and replug your
device after copying a udev file for the permissions to take effect. On some
systems, it may be necessary to reboot the machine before the new rules are
applied.

CP2110 (VID: 0x10c4 PID: 0xea80):
CP2114 (VID: 0x10c4 PID: 0xeab0):
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

Note on CP2114/SLABHIDtoUART Behavior Under Linux and Android
-------------------------------------------------------------

The CP2114 device uses a single HID interface for multiple purposes:

-	Vendor-specific reports (declared in CP2114_Common.h).

-	HID UART transmit and receive operations.

-	HID consumer controls for audio playback volume increment, decrement 
and mute.

On Windows, the in-box system drivers allow the CP2114's UART I/O and vendor-
specific HID reports to be accessed by the SLABHIDtoUART interface library, 
while the HID consumer-control volume and mute buttons remain associated with 
the system and therefore remain operational.

Linux systems do not provide the ability to handle UART I/O and vendor-specific 
HID reports concurrently with the HID consumer-control volume and mute buttons. 
When the SLABHIDtoUART library opens the device the kernel driver is detached 
from the CP2114 device handle, which results in the volume and mute controls 
ceasing to function while the device is opened. 

The current version of the SLABHIDtoUART library calls the 
libusb_set_auto_detach_kernel_driver() function prior to claiming an interface, 
which results in the kernel driver being automatically re-attached when the 
interface is released, so the volume and mute controls will again work when the 
CP2114 device is closed.
