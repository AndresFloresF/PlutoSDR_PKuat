Customizer for CP2110/CP2114
---------------------------------------

To run the tool on Linux 32 bit, we need to install enough dependencies.

Dependencies
------------

1. libusb-1.0

On Ubuntu:

  $ sudo apt-get install libusb-1.0-0-dev

2. java 1.8

  $ sudo apt-get install openjdk-8-jre

Check java version by:

  $ java -version   

3. Copy SiliconLabs.rules to /etc/udev/rules.d/

To have effect, restart PC after copying SiliconLabs.rules to /etc/udev/rules.d/

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


