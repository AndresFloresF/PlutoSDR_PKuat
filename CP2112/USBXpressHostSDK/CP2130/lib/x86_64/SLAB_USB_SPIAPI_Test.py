#!/usr/bin/env python2

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
# USER GUIDE
# Tests all found CP213x devices
# Returns 0 to shell on success, non-zero on error

from SLAB_USB_SPI import *

# Control variables
WriteRead_size      = 1024
WriteRead_timeout   = 2000
verify_writeRead    = False


class CByteGenerator:
    def __init__(self):
        self.m_Next = 0
    def Next(self):
        RetByte = self.m_Next
        if self.m_Next < 0xff:
            self.m_Next = self.m_Next + 1
        else:
            self.m_Next = 0
        return RetByte

#------------------------------------------------------
def LoopbackSetCP213x(device):
    # Set SPI control bytes and chip selects
    channel = 0
    controlByte = 0x00  # Leading edge, idle low, open-drain, 12 MHz
    cs_mode = 1         # 0: Idle, 1: Active, 2: Active, all other channels idle
    status = device.CP213x_SetSpiControlByte(channel, controlByte )
    if status != USB_SPI_ERRCODE_SUCCESS:
        PRINTV("CP213x_SetSpiControlByte returned 0x%x\n" % status)    
    
    status = device.CP213x_SetChipSelect(channel, cs_mode )
    if status != USB_SPI_ERRCODE_SUCCESS:
        PRINTV("CP213x_SetChipSelect returned 0x%x\n" % status)


#------------------------------------------------------
def LoopbackTestCP213x(device, DevIndex):
    PRINTV("Connect")
    if device.CP213x_Open(DevIndex) != USB_SPI_ERRCODE_SUCCESS:
        return -1

    PRINTV("-- Loopback test")

    ReadByteGen   = CByteGenerator()
    WriteByteGen  = CByteGenerator()
        
    rc = 0  # Assume success

    LoopbackSetCP213x(device)

    if sys.platform.startswith('linux') or sys.platform == 'win32':
        # Write-read a chunk
        WriteBuf  = ctypes.create_string_buffer(WriteRead_size)

        CbToLoop = len(WriteBuf)
        for i in range(0, CbToLoop):
            WriteBuf[i] = WriteByteGen.Next() 

        ReadBuf  = ctypes.create_string_buffer( CbToLoop)
        for i in range(0, CbToLoop):
            ReadBuf[i] = 0
        
        (status, CbLooped) = device.CP213x_TransferWriteRead(WriteBuf, ReadBuf, CbToLoop, False, WriteRead_timeout)
        if status != USB_SPI_ERRCODE_SUCCESS:
            PRINTV("CP213x_TransferWriteRead returned 0x%x\n" % status)
            rc = -1 
            if device.CP213x_Close() != USB_SPI_ERRCODE_SUCCESS:
                return -1    
            return rc        
        
        if (CbLooped == CbToLoop) and (verify_writeRead):
            PRINTV("Verifying %d bytes" % CbLooped)
            for i in range(0, CbLooped):
                ### No need to compute expected value; just compare ReadBuf to WriteBuf
                #if ReadBuf.raw[i] != ReadByteGen.Next():
                #    print("ERROR: Data corrupt at index %x: %x" % (i, ReadBuf.raw[i]))
                #    rc = -1
                #    break
                if ReadBuf.raw[i] != WriteBuf.raw[i]:
                    print("ERROR: Data corrupt at index %d: Wrote 0x%x, Read 0x%x" % (i, WriteBuf.raw[i], ReadBuf.raw[i]))
                    rc = -1
                    break
            else: # Did not break
                PRINTV("Verified %d bytes" % CbLooped)
                rc = 0
        if CbLooped != CbToLoop:
            print("ERROR: CbLooped (%d) not equal to CbToLoop (%d)\n" % (CbLooped, CbToLoop))
            rc = -1

    if device.CP213x_Close() != USB_SPI_ERRCODE_SUCCESS:
        return -1
    return rc

def LoopbackTestAll():
    import sys
    errorlevel = 1
    lib = SLAB_USB_SPI()
    try:
        SuccessCnt = 0
        (status, NumDevices) = lib.CP213x_GetNumDevices()
        if status == USB_SPI_ERRCODE_SUCCESS:
            print("* NumDevices = %d" % NumDevices)
            if TestInvalDevIndex( lib, NumDevices) == 0:
                for i in range(0, NumDevices):
                    if LoopbackTestCP213x(lib, i) == 0:
                        SuccessCnt = SuccessCnt + 1
                if NumDevices == SuccessCnt:
                    errorlevel = 0 # let shell know that test PASSED
    except:
        print("SLAB_USB_SPIAPI_Test: Unhandled exception")
    finally:
        if 0 == errorlevel: print("* Test result: PASS\n")
        else: print("* Test result: FAIL\n")
        sys.exit(errorlevel)

if __name__ == "__main__":
    LoopbackTestAll()
