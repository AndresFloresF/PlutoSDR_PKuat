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

# This module is imported by the following modules in different directories:
#   SLABHIDtoUART.py
#   SLABHIDtoUARTAPI_Test.py
#   SLAB_USB_SPIAPI_Test.py
#   SiUSBXp.py
#   SiUSBXpAPI_Test.py
# They pass an instance of their object as comport to Test(). The object is assumed
# to have implemented (in its own way) all the functions Test() calls.

#-------------------------------------------------------------------------------
# Windows COM API definitions copied from winbase.h
ONESTOPBIT   = 0
ONE5STOPBITS = 1
TWOSTOPBITS  = 2

NOPARITY     = 0
ODDPARITY    = 1
EVENPARITY   = 2
MARKPARITY   = 3
SPACEPARITY  = 4

#-------------------------------------------------------------------------------
# Constant definitions copied from TestSuite
COM_NO_FLOW_CONTROL        = 0
COM_HARDWARE_FLOW_CONTROL  = 1
COM_SOFTWARE_FLOW_CONTROL  = 2

import ctypes as ct

def PRINTV(*arg):
#    print(arg)
    pass

# Note: avoids generating 0, 0 is used to fill the read buffer before reading.
class CByteGenerator:
    def __init__(self):
        self.m_Next = 1
    def Next(self):
        RetByte = self.m_Next
        if self.m_Next < 0xff:
            self.m_Next = self.m_Next + 1
        else:
            self.m_Next = 1
        return RetByte

class TestSuite:

    def Test(self, comport, DevIndex):
        PRINTV("--- TestSuite Test DevIndex %d ---" % DevIndex)
        PRINTV("Connect")
        comport.Connect( DevIndex)
        PRINTV("SetComConfig")
        if comport.SetComConfig( 9600, 8, NOPARITY, ONESTOPBIT, COM_NO_FLOW_CONTROL) != 0:
            print("SetComConfig failed")
            comport.Disconnect()
            return -1

        #-----------------------------
        # XXX bug MCUFW-709: SiUSBXp Connect calls purge but it doesn't work, so read until nothing is left
        # Right now isn't seen much because the purge is added to Disconnect as a workaround
        CbStray = 0
        while True:
            ReadBuf  = ct.create_string_buffer(512)
            CbToRead = len( ReadBuf)
            CbRead = comport.Read( ReadBuf, CbToRead)
            if CbRead != 0:
                print("Stray bytes:\n")
                for i in range(0, CbRead):
                    print(" %x\n" % ReadBuf.raw[ i])
                CbStray = CbStray + CbRead
            else:
                break;
        PRINTV("%d stray bytes read" % CbStray)

        #-----------------------------
        # Loopback test
        PRINTV("-- Loopback test")

        ReadByteGen   = CByteGenerator()
        WriteByteGen  = CByteGenerator()

        rc = 0

        # Write a chunk
        WriteBuf  = ct.create_string_buffer( 256)
        CbToWrite = len( WriteBuf)
        for i in range(0, CbToWrite):
            WriteBuf[ i] = WriteByteGen.Next()
        CbWritten = comport.Write( WriteBuf, CbToWrite)
        PRINTV("CbWritten %d" % CbWritten)
    
        # Read and verify the written data
        CbTotalRead = 0
        CbToRead = CbWritten
        while CbTotalRead < CbWritten: 
            ReadBuf  = ct.create_string_buffer( CbToRead)
            # Fill the read buffer with 0 just in case
            for i in range(0, len( ReadBuf)):
                ReadBuf[ i] = 0

            CbRead = comport.Read( ReadBuf, CbToRead)
            PRINTV("CbToRead %d CbRead %d" % (CbToRead, CbRead))

            if CbRead != 0:
                CbTotalRead = CbTotalRead + CbRead
                CbToRead = CbToRead - CbRead
                for i in range(0, CbRead):
                    if ReadBuf.raw[ i] != ReadByteGen.Next():
                        print(" data corrupt at index %x\n" % i)
                        rc = -1
                        break
            else:
                print(" not enough data read %d\n" % CbTotalRead)
                rc = -1
                break;
        PRINTV("CbTotalRead %d" % CbTotalRead)

        PRINTV("Disconnect")
        comport.Disconnect()
        if rc:
            print("TestSuite Test DevIndex %d - FAIL" % DevIndex)
        return rc
