#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "SLABHIDDevice.h"
#include "ReportQueue.h"
#include "OsDep.h"
#include <vector>
#include "CP2114_Common.h"

/////////////////////////////////////////////////////////////////////////////
// CHIDtoUART Class
/////////////////////////////////////////////////////////////////////////////

class CHIDtoUART
{
public:
    HID_DEVICE      hid;
    CReportQueue    queue;
    DWORD           readTimeout;
    DWORD           writeTimeout;
    BYTE            partNumber;
    BYTE            deviceVersion;
    BYTE            cp2114DeviceApiVersion;
    BYTE            cp2114FirmwareVersion;
    BYTE            cp2114ConfigFormat;

    CHIDtoUART()
    {
        hid             = NULL;
        readTimeout     = 1000;
        writeTimeout    = 1000;
        partNumber      = 0x00;
        deviceVersion   = 0x00;
        cp2114DeviceApiVersion  = 0x00;
        cp2114FirmwareVersion   = 0x00;
        cp2114ConfigFormat      = 0x00;
    }

    bool ramConfigSize( BYTE &configSize) const
    {
        if (cp2114ConfigFormat == CP2114_CONFIG_VERSION_B01)
        {
            configSize = RAM_CONFIG_SIZE_B01;
        }
        else if (cp2114ConfigFormat == CP2114_CONFIG_VERSION_B02)
        {
            configSize = RAM_CONFIG_SIZE_B02;
        }
        else
        {
            return false;
        }
        return true;
    }
};
