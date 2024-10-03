################################################################################
## Copyright (c) 2015 by Silicon Laboratories Inc.  All rights reserved.
## The program contained in this listing is proprietary to Silicon Laboratories,
## headquartered in Austin, Texas, U.S.A. and is subject to worldwide copyright
## protection, including protection under the United States Copyright Act of 1976
## as an unpublished work, pursuant to Section 104 and Section 408 of Title XVII
## of the United States code.  Unauthorized copying, adaptation, distribution,
## use, or display is prohibited by this law.
################################################################################

# Python 3.4

#-------------------------------------------------------------------------------
# This program overwrites VID PID of a device.
# XXX It can only do that on a 1-interface device. This is probabl;y easy to fix, just need
# to studyu CP210xManufacturing.DLL a little more. Because each interface on a multi-interface
# device is seen as a separate device, it's not apparent how to change the ID - will the command
# sent to one of the interfaces overwrite the global ID?...

import sys
from CP210xManufacturing import *

def StrToUshort(s):
    retval = int(s, 16) # this too can raise ValueError
    if retval > 0xffff:
        raise ValueError
    if retval < 0:
        raise ValueError
    return retval

if __name__ == "__main__":
    import sys

    #------------------------------------------------------
    # Read 16-bit hex VID and PID from command line

    if len(sys.argv) != 3 :
        print("Specify VID PID in command line")
        sys.exit(1)

    try:
        argVID = StrToUshort(sys.argv[1])
        argPID = StrToUshort(sys.argv[2])
    except ValueError:
        print("Invalid arguments: %s" % str(sys.argv))
        sys.exit(1)

    #------------------------------------------------------
    # Make sure there it's the right device to change VID/PID on, in a dumb way -
    # there can be no mistake if there is just 1 device.

    DevIndex = 0

    device = CP210xManufacturing()

    result,NumDevices = device.CP210x_GetNumDevices()
    if result!= 0:
        sys.exit(1)

    if NumDevices != 1:
        print("Must have just 1 device but found %d" % NumDevices)
        sys.exit(1)

    if device.CP210x_Open(DevIndex) != 0:
        sys.exit(1)

    result,devVID = device.CP210x_GetDeviceVid()
    if result != 0:
        sys.exit(1)
    result,devPID = device.CP210x_GetDevicePid()
    if result != 0:
        sys.exit(1)
    print("Old VID %x PID %x" % (devVID, devPID))
    print("New VID %x PID %x" % (argVID, argPID))

    if device.CP210x_SetVid(argVID) != 0:
        sys.exit(1)
    if device.CP210x_SetPid(argPID) != 0:
        sys.exit(1)

    print("New VID PID programmed")

    print("Resetting device")
    if device.CP210x_Reset() != 0:
        sys.exit(1)

    if device.CP210x_Close() != 0:
        sys.exit(1)

    sys.exit(0)
