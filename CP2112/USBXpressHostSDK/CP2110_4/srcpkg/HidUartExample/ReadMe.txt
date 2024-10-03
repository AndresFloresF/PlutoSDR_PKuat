HID UART Example Command Line Application
=========================================

This application demonstrates basic UART and GPIO
functionality for CP2110 and CP2114 devices.

Release packages contain binaries built using Ubuntu 11.10 Desktop 32-bit and
Ubuntu 12.04.1 Desktop 64-bit. These binaries may not be compatible with your
system, in which case it may be necessary to build the SLABHIDtoUART and
HidUartExample binaries using make.

SLABHIDtoUART linux library release contents
--------------------------------------------

- /Common/        - Shared/common source
- /HIDAPI/        - HIDAPI source (HID library that uses libusb-1.0)
- /nbproject/     - Netbeans project that uses the library and test main app
                    makefiles
- /Release/       - SLABHIDtoUART library header files
- /Release/Linux/ - Output directory for main and libslabhidtouart.so.1.0
                    (released with prebuilt 32-bit Ubuntu 11.10 binaries)
- /Release/Linux/x86_64 - Contains 64-bit binaries (released with prebuilt
                          64-bit Ubuntu 12.04.1 binaries)
- /SLABHIDDevice/ - Silicon Labs HID API source
- /SLABHIDtoUART/ - Silicon Labs HIDtoUART API source
- /main.cpp       - Basic test app that compiles HIDtoUART source w/o using
                    libslabhidtouart.so
- /Makefile-lib-linux   - libslabhidtouart.so.1.0 makefile
- /Makefile-main-linux  - main makefile
- /FAQ.txt              - Frequently asked questions regarding building and using
                          the SLABHIDtoUART library and HidUartExample.
- /ReadMe.txt           - Explains how to build and use the SLABHIDtoUART library
- /SharedObjectLibraryUsage.txt - Explains how to use libslabhidtouart.so
- /SiliconLabs.rules    - udev rules file for Silicon Labs USB devices
                          (VID = 0x10c4)

HidUartExample release contents
-------------------------------

- /nbproject/     - Netbeans project that uses the makefile
- /Release/Linux/ - Output directory for hidUartExample
                    (released with prebuilt 32-bit Ubuntu 11.10 hidUartExample)
- /main.cpp       - The main source file for hidUartExample
- /Makefile       - The makefile for hidUartExample
- /ReadMe.txt     - Explains how to build and run hidUartExample
- /SharedObjectLibraryUsage.txt - Explains how to use libslabhidtouart.so
- /SiliconLabs.rules    - udev rules file for Silicon Labs USB devices
                          (VID = 0x10c4)
- /SLABHIDtoUart.h - libslabhidtouart library include file
- /Types.h         - shared types header file
- /Terminal.cpp    - Linux terminal helper functions
- /Terminal.h      - Linux terminal helper function prototypes

Building the application
------------------------

1. Install libslabhidtouart.so to /opt/lib (see SharedObjectLibraryUsage.txt)

2. Open the provided project using Netbeans

- or -

3. Build the project using make:

  $ make

  Builds /Release/Linux/hidUartExample

Running the application
-----------------------

1. Install the appropriate udev rule (ie SiliconLabs.rules)
   by copying the file to /etc/udev/rules.d

  $ cp SiliconLabs.rules /etc/udev/rules.d/

- or -

2. Create your own .rules file by modifying an existing
   rules file and specifying your own VID/PID and copy
   this modified file to /etc/udev/rules.d

3. It may be necessary to unplug/replug your USB device
   and/or reboot your machine before the udev rule will
   take affect.

4. Add the location of libslabhidtouart.so to your
   LD_LIBRARY_PATH. By default, this could be /opt/lib

  $ export LD_LIBRARY_PATH=/opt/lib:$LD_LIBRARY_PATH

5. Run the example application:

  $ ./hidUartExample

Command Line Arguments
----------------------

hidUartExample [-l] [-vid VID] [-pid PID]
hidUartExample [-s SERIAL] [-vid VID] [-pid PID]

positional arguments:
-l               Display a list of all CP211x serial strings
-vid VID         Specify the USB Vendor ID of the device (default 0x10c4)
-pid PID         Specify the USB Product ID of the device (default 0xea80)
-s SERIAL        Connect to a CP211x device by serial string

Example Command Line Arguments
------------------------------

1. To display a list of HID serial strings using the
   default VID/PID for CP2110:

  $ ./hidUartExample -l

2. To display a list of HID serial strings using your own VID/PID
   for a CP2110 or CP2114 (ex: VID = 0x1234, PID = 0x4321):

  $ ./hidUartExample -l -vid 0x1234 -pid 0x4321

3. To run the example application using the default VID/PID for a CP2110
   (0x10c4/0xea80) and connect to the first CP2110:

  $ ./hidUartExample

4. To run the example application and connect to a device with the specified
   VID, PID, and serial string (ex: VID = 0x10c4, PID = 0xeab0, Serial = 0005F5C5)

  $ ./hidUartExample -s 0005F5C5 -vid 0x10c4 -pid 0xeab0

5. To display help for the command line arguments

  $ ./hidUartExample -?
