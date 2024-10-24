/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "CP210xManufacturing.h"
#include "DeviceList.h"
#include "CP210xDevice.h"
#include "CP2103Device.h"
#include "OsDep.h"

/////////////////////////////////////////////////////////////////////////////
// Global Variables
/////////////////////////////////////////////////////////////////////////////

// Each time Construct()/Destruct() are called
// DeviceList will be updated to track the use
// of heap memory (stores object pointers)
//
// The CDeviceList destructor will automatically
// free any remaining devices
static CDeviceList<CCP210xDevice> DeviceList;

/////////////////////////////////////////////////////////////////////////////
// Exported Library Functions
/////////////////////////////////////////////////////////////////////////////

CP210x_STATUS CP210x_GetNumDevices(
        uint32_t * lpdwNumDevices
        ) {
    CP210x_STATUS status;

    // Check parameters
    if (lpdwNumDevices) {
        status = CCP210xDevice::GetNumDevices(lpdwNumDevices);
    } else {
        status = CP210x_INVALID_PARAMETER;
    }

    return status;
}

CP210x_STATUS CP210x_Open(
        uint32_t dwDevice,
        HANDLE* cyHandle
        ) {
    CP210x_STATUS status;

    // Check parameters
    if (cyHandle) {
        *cyHandle = NULL;

        // Create a new device object and add it to the device list
        CCP210xDevice* dev = NULL;

        status = CCP210xDevice::Open(dwDevice, &dev);

        if (status == CP210x_SUCCESS) {
            DeviceList.Add(dev);
            *cyHandle = dev->GetHandle();
        } else {
            if (dev) {
                // Close the handle
                dev->Close();

                // Delete the device object and
                // remove the device reference from the device list
                DeviceList.Destruct(dev);
            }
        }
    } else {
        status = CP210x_INVALID_PARAMETER;
    }

    return status;
}

CP210x_STATUS CP210x_Close(
        HANDLE cyHandle
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Close the device
        status = dev->Close();

        // Deallocate the device object, remove the device reference
        // from the device list
        DeviceList.Destruct(dev);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetPartNumber(
        HANDLE cyHandle,
        LPBYTE lpbPartNum
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (lpbPartNum) {
            status = dev->GetPartNumber(lpbPartNum);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetProductString(
        const uint32_t dwDeviceIndex,
        void * lpvDeviceString,
        const uint32_t dwFlags
        ) {
  (void)(dwFlags);
    //TODO - fill out all flagged strings
    HANDLE h;

  CP210x_STATUS status;

    if (CP210x_Open(dwDeviceIndex, &h) == CP210x_SUCCESS) {
        BYTE length;
      switch (dwFlags) {
        case CP210x_RETURN_SERIAL_NUMBER:
          status = CP210x_GetDeviceSerialNumber(h, lpvDeviceString, &length, true);
          break;
        case CP210x_RETURN_DESCRIPTION:
          status = CP210x_GetDeviceProductString(h, lpvDeviceString, &length, true);
          break;
        case CP210x_RETURN_FULL_PATH:
        default:
          status = CP210x_INVALID_PARAMETER;
          break;
      }

      CP210x_Close(h);
      return status;
    }
    
    return CP210x_DEVICE_NOT_FOUND;
}

CP210x_STATUS
CP210x_SetVid(
        HANDLE cyHandle,
        WORD wVid
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetVid(wVid);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_SetPid(
        HANDLE cyHandle,
        WORD wPid
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetPid(wPid);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_SetManufacturerString(
        HANDLE cyHandle,
        LPVOID lpvManufacturer,
        BYTE bLength,
        BOOL bConvertToUnicode
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetManufacturerString(lpvManufacturer, bLength, bConvertToUnicode);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_SetProductString(
        HANDLE cyHandle,
        LPVOID lpvProduct,
        BYTE bLength,
        BOOL bConvertToUnicode
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetProductString(lpvProduct, bLength, bConvertToUnicode);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_SetInterfaceString(
        HANDLE cyHandle,
        BYTE bInterfaceNumber,
        LPVOID lpvInterface,
        BYTE bLength,
        BOOL bConvertToUnicode
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetInterfaceString(bInterfaceNumber, lpvInterface, bLength, bConvertToUnicode);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_SetSerialNumber(
        HANDLE cyHandle,
        LPVOID lpvSerialNumber,
        BYTE bLength,
        BOOL bConvertToUnicode
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetSerialNumber(lpvSerialNumber, bLength, bConvertToUnicode);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_SetSelfPower(
        HANDLE cyHandle,
        BOOL bSelfPower
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetSelfPower(bSelfPower);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_SetMaxPower(
        HANDLE cyHandle,
        BYTE bMaxPower
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetMaxPower(bMaxPower);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_SetFlushBufferConfig(
        HANDLE cyHandle,
        WORD wFlushBufferConfig
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetFlushBufferConfig(wFlushBufferConfig);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_SetDeviceMode(
        HANDLE cyHandle,
        BYTE bDeviceModeECI,
        BYTE bDeviceModeSCI
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetDeviceMode(bDeviceModeECI, bDeviceModeSCI);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_SetDeviceVersion(
        HANDLE cyHandle,
        WORD wVersion
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetDeviceVersion(wVersion);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_SetBaudRateConfig(
        HANDLE cyHandle,
        BAUD_CONFIG* baudConfigData
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetBaudRateConfig(baudConfigData);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_SetPortConfig(
        HANDLE cyHandle,
        PORT_CONFIG* PortConfig
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetPortConfig(PortConfig);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_SetDualPortConfig(
        HANDLE cyHandle,
        DUAL_PORT_CONFIG* DualPortConfig
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetDualPortConfig(DualPortConfig);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_SetQuadPortConfig(
        HANDLE cyHandle,
        QUAD_PORT_CONFIG* QuadPortConfig
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetQuadPortConfig(QuadPortConfig);
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_SetLockValue(
        HANDLE cyHandle
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->SetLockValue();
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetDeviceVid(
        HANDLE cyHandle,
        LPWORD lpwVid
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (lpwVid) {
            status = dev->GetVid(lpwVid);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetDevicePid(
        HANDLE cyHandle,
        LPWORD lpwPid
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (lpwPid) {
            status = dev->GetPid(lpwPid);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetDeviceManufacturerString(
        HANDLE cyHandle,
        LPVOID lpManufacturer,
        LPBYTE lpbLength,
        BOOL bConvertToASCII
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (lpManufacturer) {
            status = dev->GetDeviceManufacturerString(lpManufacturer, lpbLength, bConvertToASCII);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetDeviceProductString(
        HANDLE cyHandle,
        LPVOID lpProduct,
        LPBYTE lpbLength,
        BOOL bConvertToASCII
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (lpProduct) {
            status = dev->GetDeviceProductString(lpProduct, lpbLength, bConvertToASCII);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetDeviceInterfaceString(
        HANDLE cyHandle,
        BYTE bInterfaceNumber,
        LPVOID lpInterface,
        LPBYTE lpbLength,
        BOOL bConvertToASCII
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (lpInterface) {
            status = dev->GetDeviceInterfaceString(bInterfaceNumber, lpInterface, lpbLength, bConvertToASCII);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetDeviceSerialNumber(
        HANDLE cyHandle,
        LPVOID lpSerialNumber,
        LPBYTE lpbLength,
        BOOL bConvertToASCII
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (lpSerialNumber) {
            status = dev->GetDeviceSerialNumber(lpSerialNumber, lpbLength, bConvertToASCII);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetDeviceAddress(
                        _In_ _Pre_defensive_ const HANDLE cyHandle
                        ,LPBYTE lpbAddress)
{
  CP210x_STATUS status;
  CCP210xDevice* dev = (CCP210xDevice*) cyHandle;
    // Check device object
  if (DeviceList.Validate(dev)) {
    if (lpbAddress) {
      status = dev->GetDeviceAddress(lpbAddress);
    } else {
      status = CP210x_INVALID_PARAMETER;
    }
  } else {
    status = CP210x_INVALID_HANDLE;
  }

  return status;
}

CP210x_STATUS
CP210x_GetSelfPower(
        HANDLE cyHandle,
        LPBOOL lpbSelfPower
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (lpbSelfPower) {
            status = dev->GetSelfPower(lpbSelfPower);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetMaxPower(
        HANDLE cyHandle,
        LPBYTE lpbPower
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (lpbPower) {
            status = dev->GetMaxPower(lpbPower);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetFlushBufferConfig(
        HANDLE cyHandle,
        LPWORD lpwFlushBufferConfig
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (lpwFlushBufferConfig) {
            status = dev->GetFlushBufferConfig(lpwFlushBufferConfig);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetDeviceMode(
        HANDLE cyHandle,
        LPBYTE lpbDeviceModeECI,
        LPBYTE lpbDeviceModeSCI
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (lpbDeviceModeECI && lpbDeviceModeSCI) {
            status = dev->GetDeviceMode(lpbDeviceModeECI, lpbDeviceModeSCI);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetDeviceVersion(
        HANDLE cyHandle,
        LPWORD lpwVersion
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (lpwVersion) {
            status = dev->GetDeviceVersion(lpwVersion);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetBaudRateConfig(
        HANDLE cyHandle,
        BAUD_CONFIG* baudConfigData
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (baudConfigData) {
            status = dev->GetBaudRateConfig(baudConfigData);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetPortConfig(
        HANDLE cyHandle,
        PORT_CONFIG* PortConfig
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (PortConfig) {
            status = dev->GetPortConfig(PortConfig);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetDualPortConfig(
        HANDLE cyHandle,
        DUAL_PORT_CONFIG* DualPortConfig
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (DualPortConfig) {
            status = dev->GetDualPortConfig(DualPortConfig);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetQuadPortConfig(
        HANDLE cyHandle,
        QUAD_PORT_CONFIG* QuadPortConfig
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (QuadPortConfig) {
            status = dev->GetQuadPortConfig(QuadPortConfig);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_GetLockValue(
        HANDLE cyHandle,
        LPBYTE lpbLockValue
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        // Check pointers
        if (lpbLockValue) {
            status = dev->GetLockValue(lpbLockValue);
        } else {
            status = CP210x_INVALID_PARAMETER;
        }
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_Reset(
        HANDLE cyHandle
        ) {
    CP210x_STATUS status;
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;

    // Check device object
    if (DeviceList.Validate(dev)) {
        status = dev->Reset();
    } else {
        status = CP210x_INVALID_HANDLE;
    }

    return status;
}

CP210x_STATUS
CP210x_CreateHexFile(
        HANDLE cyHandle,
        LPCSTR lpvFileName
        ) {
    //TODO
  UNREFERENCED_PARAMETER(cyHandle);
  UNREFERENCED_PARAMETER(lpvFileName);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

//------------------------------------------------------------------------
// CP210x_GetFirmwareVersion()
//
// Retrieves the on-board firmware version.
// 
//------------------------------------------------------------------------

CP210x_STATUS 
CP210x_GetFirmwareVersion(	HANDLE cyHandle,
						pFirmware_t	lpVersion)
{
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;
    if (!DeviceList.Validate(dev)) {
        return CP210x_INVALID_HANDLE;
    }
    if( !lpVersion) {
        return CP210x_INVALID_PARAMETER;
    }
    return dev->GetFirmwareVersion( lpVersion);
}

//------------------------------------------------------------------------
// CP210x_GetConfig()
//
// Retrieves the on-board config block from the device.  Caller must read first 2 bytes 
// to determine size of config block, then request the full config.
//
//------------------------------------------------------------------------
CP210x_STATUS 
CP210x_GetConfig(	HANDLE cyHandle,
						LPBYTE	lpbConfig,
						WORD	bLength)
{
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;
    if (!DeviceList.Validate(dev)) {
        return CP210x_INVALID_HANDLE;
    }
    if( !lpbConfig) {
        return CP210x_INVALID_PARAMETER;
    }
    return dev->GetConfig( lpbConfig, bLength);
}

//------------------------------------------------------------------------
// CP210x_SetConfig()
//
// Writes the on-board config block from the device.  Caller must verify any checksum
// before calling.
//
//------------------------------------------------------------------------
CP210x_STATUS 
CP210x_SetConfig(	HANDLE cyHandle,
						LPBYTE	lpbConfig,
						WORD	bLength)
{
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;
    if (!DeviceList.Validate(dev)) {
        return CP210x_INVALID_HANDLE;
    }
    if( !lpbConfig) {
        return CP210x_INVALID_PARAMETER;
    }
    return dev->SetConfig( lpbConfig, bLength);
}

//------------------------------------------------------------------------
// CP210x_updateFirmware()
//
// Requests device enter Bootloader mode.  Separate application will upload firmware.
// Unsure what will happen with USB interface when device resets. Specific to CP2102N.
//
//------------------------------------------------------------------------

CP210x_STATUS 
CP210x_UpdateFirmware(	HANDLE cyHandle)
{
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;
    if (!DeviceList.Validate(dev)) {
        return CP210x_INVALID_HANDLE;
    }
    return dev->UpdateFirmware();
}

// This function allows the caller to create a generic USB command to read.
// For this function, lpbGeneric and bLength must include the 8 bytes
// of the setup packet.  Use with caution.  Overwrites lpbGeneric with
// retrieved data, including 8 bytes of setup packet.  NOT intended for long term use
// - once a new feature has been debugged, add it to the DLL!
CP210x_STATUS 
CP210x_GetGeneric(	HANDLE cyHandle,
						LPBYTE	lpbGeneric,
						WORD	bLength)
{
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;
    if (!DeviceList.Validate(dev)) {
        return CP210x_INVALID_HANDLE;
    }
    if( !lpbGeneric) {
        return CP210x_INVALID_PARAMETER;
    }
    return dev->GetGeneric( lpbGeneric, bLength);
}

// This function allows the caller to create a generic USB command to write
// For this function, lpbGeneric and bLength must include the 8 bytes
// of the setup packet.  Use with caution.  NOT intended for long term use
// - once a new feature has been debugged, add it to the DLL!
CP210x_STATUS 
CP210x_SetGeneric(	HANDLE cyHandle,
						LPBYTE	lpbGeneric,
						WORD	bLength)
{
    CCP210xDevice* dev = (CCP210xDevice*) cyHandle;
    if (!DeviceList.Validate(dev)) {
        return CP210x_INVALID_HANDLE;
    }
    if( !lpbGeneric) {
        return CP210x_INVALID_PARAMETER;
    }
    return dev->SetGeneric( lpbGeneric, bLength);
}
