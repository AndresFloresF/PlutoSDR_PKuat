// SLAB_USB_SPI.cpp
// CP2130 Interface Library main implementation source file
//
/// @file SLAB_USB_SPI.cpp

// Include local headers
#if defined(__linux__)
#include <sys/eventfd.h>
#endif
#include <unistd.h>
#include "SLAB_USB_SPI.h"
#include "SLAB_USB_SPI_private.h"
#include "CP213xDevice.h"
#include "DeviceList.h"
#include "CP213xSupportFunctions.h"
#include "ReadThread.h"

//
// Global Variables
//

// DeviceList stores device object pointers.
// Each time CDeviceList::Construct()/Destruct()
// are called, DeviceList will be updated to track
// the use of heap memory.
static CDeviceList<CCP213xDevice> DeviceList;

//
// Maps that relate ChipSelect/GPIO index to bitmask and port byte offset (0 or 1)
//

WORD gpioWordMap[CP213x_NUM_GPIO] = {
    0x0008, // GPIO0
    0x0010, // GPIO1
    0x0020, // GPIO2
    0x0040, // GPIO3
    0x0080, // GPIO4
    0x0100, // GPIO5
    0x0400, // GPIO6
    0x0800, // GPIO7
    0x1000, // GPIO8
    0x2000, // GPIO9
    0x4000  // GPIO10
};

tGpioMap gpioByteMap[] =
{
// offset   mask      GPIO #
    0x00,   0x08,   // [0]
    0x00,   0x10,   // [1]
    0x00,   0x20,   // [2]
    0x00,   0x40,   // [3]
    0x00,   0x80,   // [4]
    0x01,   0x01,   // [5]
    0x01,   0x04,   // [6]
    0x01,   0x08,   // [7]
    0x01,   0x10,   // [8]
    0x01,   0x20,   // [9]
    0x01,   0x40    // [10]
};

tGpioMap gpioValuesByteMap[] =
{
// offset   mask      GPIO #
    0x01,   0x08,   // [0]
    0x01,   0x10,   // [1]
    0x01,   0x20,   // [2]
    0x01,   0x40,   // [3]
    0x01,   0x80,   // [4]
    0x00,   0x01,   // [5]
    0x00,   0x04,   // [6]
    0x00,   0x08,   // [7]
    0x00,   0x10,   // [8]
    0x00,   0x20,   // [9]
    0x00,   0x40    // [10]
};

//
// External API functions
//

// The DPRINT macro prints formatted text to the Visual Studio 'Output' window

USB_SPI_STATUS DPRINT(const char *format, ...)
#if _DEBUG
{
#define DPRINT_BUFSIZE 256
char    buf[DPRINT_BUFSIZE], *p = buf;
va_list args;
int     n;

    va_start(args, format);
    n = _vsnprintf_s(p, sizeof buf - 3, DPRINT_BUFSIZE, format, args); // buf-3 is room for CR/LF/NUL
    va_end(args);

    p += (n < 0) ? DPRINT_BUFSIZE - 3 : n;

    while ( p > buf  &&  isspace(p[-1]) )
            *--p = '\0';
    *p   = '\0';
    OutputDebugStringA(buf);
    return USB_SPI_ERRCODE_SUCCESS;
}
#else
{return USB_SPI_ERRCODE_SUCCESS;}
#endif


/*!
 * \brief Get the Interface Library version numbers.
 *
 * \details
 *
 * \param[out] pMajor Major release number
 * \param[out]  pMinor Minor release number
 * \param[out]  pIsRelease 0: Not released version,  1: Released version
 * \return USB_SPI_ERRCODE_SUCCESS
 */
USB_SPI_STATUS
CP213x_GetLibraryVersion(BYTE* pMajor, BYTE* pMinor, BOOL* pIsRelease)
{
    if (pMajor) *pMajor = LIBVERSION_MAJOR;
    if (pMinor) *pMinor = LIBVERSION_MINOR;
    if (pIsRelease) *pIsRelease = LIBVERSION_ISRELEASE;
    
    return USB_SPI_ERRCODE_SUCCESS;
}


//
//  Device Management Functions
//
/// @name Device Management Functions
/// @{

/*!
 * \details Returns the number of CP213x devices connected to
 * the system
 *
 * \param[in] numDevices A return value of 0 indicates that no devices are
 * available. Otherwise this parameter returns the number of connected
 * devices. When refering to a device by @c deviceIndex, the index may range
 * from 0 to (::CP213x_GetNumDevices() - 1).
 * \param[in] vid The vid for the CP2130 device to list in the return value
 * \param[in] pid The pid for the CP2130 device to list in the return value
 * 
 *
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_PARAMETER
 *
 * @remark This function returns the number of connected devices. This does
 * not imply that a device is not already in use. Users must call
 * CP213x_GetNumDevices() before calling any function that takes a device index
 * parameter in order to build an up to date device list. If a device is
 * installed or removed after calling ::CP213x_GetNumDevices(), then the device
 * list will be out of date, and the results may be unpredictable.
 *
 */
USB_SPI_STATUS
CP213x_GetNumDevices(DWORD* lpdwNumDevices, DWORD vid, DWORD pid)
{
    USB_SPI_STATUS status;
    
    // Check parameters
    if (lpdwNumDevices) {
        status = CCP213xDevice::GetNumDevices(lpdwNumDevices, vid, pid);
    } else {
        status = USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    return status;
}


/*!
 * \brief Opens a USB device using a device index and returns a
 * device object pointer which will be used for subsequent access.
 *
 * \details
 *
 * \param[in] deviceIndex Index of device to open
 * \param[out] device Returns a pointer to a device object used for subsequent
 * device access.
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT
 * \return USB_SPI_ERRCODE_INVALID_PARAMETER
 *
 * \note ::CP213x_GetNumDevices() must be called prior to calling this function
 * in order to initialize the library's internal device list.
 *
 * \sa ::CP213x_GetNumDevices()
 */
USB_SPI_STATUS CP213x_Open(DWORD dwDevice, CP213x_DEVICE* phDevice, DWORD vid, DWORD pid)
{
    USB_SPI_STATUS status;

    // Check parameters
    if (phDevice)
    {
        *phDevice = NULL;

        // Create a new device object and add it to the device list
        CCP213xDevice* dev = NULL;

        status = CCP213xDevice::Open(dwDevice, &dev, vid, pid);

        if (status == USB_SPI_ERRCODE_SUCCESS)
        {
            DeviceList.Add(dev);
            *phDevice = dev->GetHandle();
        } else
        {
            if (dev)
            {
                // Close the handle
                dev->Close();

                // Delete the device object and
                // remove the device reference from the device list
                DeviceList.Destruct(dev);
            }
        }
    }
    else
    {
        status = USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    return status;
}


/*!
 * \brief Close device handle
 *
 * \details
 *
 * \param[in] hDevice Pointer to device handle
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT
 */
USB_SPI_STATUS
CP213x_Close(CP213x_DEVICE hDevice)
{
    USB_SPI_STATUS code;

    CCP213xDevice* dev = (CCP213xDevice*)hDevice;

    // Check device object
    if (DeviceList.Validate(dev))
    {
        // Stop read thread if in progress
        if (dev->IsAsyncReadInProgress()) {
            dev->CancelAsyncRead();
        }
                
        // Close the device
        code = dev->Close();

        // Delete the device object and
        // remove the device reference from the device list
        DeviceList.Destruct(dev);
    }
    else
    {
        code = USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
    }
    return code;
}

/*
 * \details Returns the USB device opened state.
 *
 * \param[in] device A pointer to a device object returned by
 * CP213x_OpenByIndex() or CP213x_OpenByDevicePath().
 *
 * @return @c TRUE if a device is opened. @c FALSE if @c device is invalid or
 * if a device is not opened.
 *
 * @sa ::CP213x_OpenByIndex()
 * @sa ::CP213x_OpenByDevicePath()
*/
BOOL
CP213x_IsOpened(CP213x_DEVICE device)
{
    CCP213xDevice* dev = (CCP213xDevice*)device;

    // Check device object
    if (DeviceList.Validate(dev))
    {
        return dev->IsOpened();
    }
    else
    {
        return FALSE;
    }
}

/*!
 * \brief Get specified device descriptor
 *
 * \details
 *
 * \param[in] hDevice Interface Handle
 * \param[out] pDescriptor Pointer to device descriptor
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_ALLOC_FAILURE
 * \return USB_SPI_ERRCODE_UNKNOWN_ERROR
 */
USB_SPI_STATUS
CP213x_GetDeviceDescriptor(CP213x_DEVICE hDevice, PDEVICE_DESCRIPTOR pDescriptor)
{
    CCP213xDevice* dev = (CCP213xDevice*)hDevice;   
    
    if (dev == NULL) {
        return USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
    }
    
    if (pDescriptor == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    return dev->GetDeviceDescriptor(pDescriptor);
}

/*!
 * \brief Get specified string descriptor
 *
 * \details
 *
 * \param[in] hDevice Device Handle
 * \param[in] index String descriptor index
 * \param[out] stringDescriptor String descriptor
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_ALLOC_FAILURE
 * \return USB_SPI_ERRCODE_UNKNOWN_ERROR
 */
USB_SPI_STATUS
CP213x_GetStringDescriptor(CP213x_DEVICE hDevice, BYTE index, BYTE stringDescriptor[STRING_DESCRIPTOR_SIZE])
{
    CCP213xDevice* dev = (CCP213xDevice*)hDevice;
    
    if (dev == NULL) {
        return USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
    }
    
    if (stringDescriptor == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    return dev->GetStringDescriptor(index, stringDescriptor);
}

/// @}  (end of Device Management Functions)


/*!
 * \brief Reset device
 *
 * \details Send command to reset the device
 *
 * \param[in] hDevice USB Interface Handle
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_Reset(CP213x_DEVICE hDevice)
{
    USB_SPI_STATUS status;
    BYTE data[16];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length       = 0;
    SetupPacket.Request      = RESET_DEVICE;
    SetupPacket.Index        = 0;
    SetupPacket.RequestType  = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value        = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    return status;
}


/*!
 * \brief Stop 'Read with RTR' operation
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] stopRtr 0: No effect  1: Stop Read with RTR operation
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetRtrStop(CP213x_DEVICE hDevice, BYTE stopRtr)
{
    USB_SPI_STATUS status;
    BYTE data[16];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    stopRtr ? data[0] = 0x01 : data[0] = 0x00;
    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length       = 1;
    SetupPacket.Request      = SET_RTR_STOP;
    SetupPacket.Index        = 0;
    SetupPacket.RequestType  = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value        = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    return status;
}


/*!
 * \brief Read status of RTR operation
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] inRtrMode 0: Device is not in RTR mode  1: Device is in RTR mode
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetRtrState(CP213x_DEVICE hDevice, BYTE* inRtrMode)
{
    USB_SPI_STATUS status;
    BYTE data[16];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;
    
    if (inRtrMode == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length       = 1;
    SetupPacket.Request      = GET_RTR_STOP;
    SetupPacket.Index        = 0;
    SetupPacket.RequestType  = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value        = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    if(!status)
    {
        *inRtrMode = data[0];
    }
    return status;
}


/*!
 * \brief Get the read-only device version
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] majorVersion
 * \param[out] minorVersion
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetDeviceVersion(CP213x_DEVICE hDevice, BYTE* majorVersion, BYTE* minorVersion)
{
    USB_SPI_STATUS status;
    SETUP_PACKET SetupPacket;
    BYTE data[16];
    DWORD cbRead = 0;
    
    if (majorVersion == NULL || minorVersion == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 2;
    SetupPacket.Request     = GET_RO_VERSION;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    if(!status)
    {
        if (majorVersion) *majorVersion = data[0];
        if (minorVersion) *minorVersion = data[1];
    }
    return status;
}


/*!
 * \brief Set the SPI control word
 *
 * \details Set the SPI control byte for a given channel
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] channel Channel for which to set SPI control byte
 * \param[in] controlByte SPI control byte
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetSpiControlByte(CP213x_DEVICE hDevice, BYTE channel, BYTE controlByte)
{
    USB_SPI_STATUS status;
    BYTE data[EP_BUFSIZE];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    data[0] = channel;
    data[1] = controlByte;

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 2;
    SetupPacket.Request     = SET_SPI_CONTROL_BYTE;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    return status;
}


/*!
 * \brief Get the SPI control word
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] controlBytes Buffer that contains the SPI control bytes for each channel
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetSpiControlBytes(CP213x_DEVICE hDevice, BYTE controlBytes[CP213x_NUM_GPIO])
{
    USB_SPI_STATUS status;
    SETUP_PACKET SetupPacket;
    BYTE data[16];
    DWORD cbRead = 0;
    
    if (controlBytes == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = CP213x_NUM_GPIO;          // Number of bytes expected
    SetupPacket.Request     = GET_SPI_CONTROL_BYTES;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    if(!status)
    {
        memcpy(controlBytes, data, CP213x_NUM_GPIO);
    }
    return status;
}


/*!
 * \brief Set the SPI delay values
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] channel SPI channel for which to set delays
 * \param[in] delayMode
 * \param[in] interByteDelay
 * \param[in] postAssertDelay
 * \param[in] preDeassertDelay
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetSpiDelay ( CP213x_DEVICE hDevice,
                     BYTE channel,
                     BYTE delayMode,
                     WORD interByteDelay,
                     WORD postAssertDelay,
                     WORD preDeassertDelay )
{
    USB_SPI_STATUS status;
    BYTE data[16];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    data[0] = channel;         // Channel number (GPIO index) to configure;
    data[1] = delayMode;     // Delay control (b3: ToggleCS, b2: Enable Pre-CS-Deassert, b1: Enable Post-CS-Assert, b0: Enable Interbyte
    data[2] = (BYTE)(interByteDelay >> 8);      // Inter-byte delay (MSB);
    data[3] = (BYTE)(interByteDelay & 0xFF);    // Inter-byte delay (LSB);
    data[4] = (BYTE)(postAssertDelay >> 8);     // CS Post-CS-Assert delay (MSB);
    data[5] = (BYTE)(postAssertDelay & 0xFF);   // CS Post-CS-Assert delay (LSB);
    data[6] = (BYTE)(preDeassertDelay >> 8);    // CS Pre-CS-Deassert delay (MSB);
    data[7] = (BYTE)(preDeassertDelay & 0xFF);  // CS Pre-CS-Deassert delay (LSB);

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 8;
    SetupPacket.Request     = SET_SPI_DELAY;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    return status;
}


/*!
 * \brief Get the SPI delay values
 *
 * \details Pass the desired SPI channel number in buffer[0] 
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] channel SPI channel for which to get delays
 * \param[out] delayMode
 * \param[out] interByteDelay
 * \param[out] postAssertDelay
 * \param[out] preDeassertDelay
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetSpiDelay(CP213x_DEVICE hDevice,
                     BYTE channel,
                     BYTE* delayMode,
                     WORD* interByteDelay,
                     WORD* postAssertDelay,
                     WORD* preDeassertDelay )
{
    USB_SPI_STATUS status;
    SETUP_PACKET SetupPacket;
    BYTE data[16];
    DWORD cbRead = 0;

    if (channel >= CP213x_NUM_GPIO) {
        return USB_SPI_ERRCODE_INVALID_CHANNEL_INDEX;
    }
    
    if ((delayMode == NULL) || (interByteDelay == NULL) || (postAssertDelay == NULL) || (preDeassertDelay == NULL)){
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    status = USB_SPI_ERRCODE_SUCCESS;

    SetupPacket.Length      = 8;
    SetupPacket.Request     = GET_SPI_DELAY;
    SetupPacket.RequestType = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    SetupPacket.Index       = channel;

    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    if(!status)
    {
        // The firmware returns the channel in [0].
        *delayMode = data[1];
        *interByteDelay = (WORD)((data[2] << 8) | data[3]);
        *postAssertDelay = (WORD)((data[4] << 8) | data[5]);
        *preDeassertDelay = (WORD)((data[6] << 8) | data[7]);
    }
    return status;
}


/*!
 * \brief Set the FIFO full threshold
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] fifoFullThreshold The FIFO full threshold value to set
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetFifoFullThreshold(CP213x_DEVICE hDevice, BYTE fifoFullThreshold)
{
    USB_SPI_STATUS status;
    BYTE data[16];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    data[0] = fifoFullThreshold;
    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 1;
    SetupPacket.Request     = SET_FULL_THRESHOLD;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    return status;
}


/*!
 * \brief Get the FIFO full threshold value
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] pFifoFullThreshold The FIFO full threshold
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetFifoFullThreshold(CP213x_DEVICE hDevice, BYTE* pFifoFullThreshold)
{
    USB_SPI_STATUS status;
    SETUP_PACKET SetupPacket;
    BYTE data[16];
    DWORD cbRead = 0;
    
    if (pFifoFullThreshold == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 0x01;
    SetupPacket.Request     = GET_FULL_THRESHOLD;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    if(!status)
    {
        *pFifoFullThreshold = data[0];
    }
    return status;
}


/*!
 * \brief Set SPI chip select control word
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] channel
 * \param[in] mode Chip select mode 0: Idle, 1: Active, 2: Active; all other channels idle
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetChipSelect(CP213x_DEVICE hDevice, BYTE channel, BYTE mode)
{
    USB_SPI_STATUS status;
    BYTE data[16];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    data[0] = channel;      // GPIO Index
    data[1] = mode;         // Mode (0: Idle, 1: Active, 2: Active; all other channels idle.

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 2;
    SetupPacket.Request     = SET_CHIP_SELECT;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    return status;
}


/*!
 * \brief Get SPI chip select control word
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] channelCsEnable
 * \param[out] pinCsEnable
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetChipSelect(CP213x_DEVICE hDevice, WORD* channelCsEnable, WORD* pinCsEnable)
{
    USB_SPI_STATUS status;
    SETUP_PACKET SetupPacket;
    BYTE data[16];
    DWORD cbRead = 0;
    
    if ((channelCsEnable == NULL) || (pinCsEnable == NULL)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 4;
    SetupPacket.Request     = GET_CHIP_SELECT;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    if(!status)
    {
        *channelCsEnable = (WORD)((data[0] << 8) | data[1]);
        *pinCsEnable = (WORD)((data[2] << 8) | data[3]);
    }
    return status;
}

/*!
 * \brief Set the GPIO mode control
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] index GPIO number to configure
 * \param[in] mode GPIO mode
 * \param[in] level GPIO state [0,1]
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetGpioModeAndLevel(CP213x_DEVICE hDevice, BYTE index, BYTE mode, BYTE level)
{
    USB_SPI_STATUS status;
    BYTE data[16];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    // Validate parameters
    if ((index >= CP213x_NUM_GPIO) ||
        ((mode!=GPIO_MODE_INPUT) && (mode!=GPIO_MODE_OUTPUT_PP) && (mode!=GPIO_MODE_OUTPUT_OD)) ||
        (level > 1))
    {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    data[0] = index;
    data[1] = mode;
    data[2] = level;

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 3;
    SetupPacket.Request     = SET_GPIO_MODE_AND_LEVEL;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    return status;
}


/*!
 * \brief Get the GPIO mode control
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] channel The GPIO line
 * \param[out] mode The mode for the specified GPIO line
 * \param[out] level The level for the specified GPIO line
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetGpioModeAndLevel(CP213x_DEVICE hDevice, BYTE channel, BYTE* mode, BYTE* level)
{
    USB_SPI_STATUS status;
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;
    BYTE data[16];
    
    if ((mode == NULL) || (level == NULL)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 4;
    SetupPacket.Request     = GET_GPIO_MODE_AND_LEVEL;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    if(!status)
    {
        BYTE byteOffset = gpioByteMap[channel].offset;
        BYTE byteMask = gpioByteMap[channel].mask;
        *level = data[byteOffset] & byteMask ? 1 : 0;
        *mode =  data[byteOffset+2] & byteMask ? 1 : 0;
    }
    else
    {
        DPRINT("\r\nERROR: CP213x_GetGpioModeAndLevel: ControlTransfer() returned 0x%02X", status);
    }
    return status;
}


/*!
 * \brief Set the GPIO values
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] mask The mask of which GPIO values to set
 * \param[in] gpioValues The GPIO values to set for the specified mask
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetGpioValues( CP213x_DEVICE hDevice, WORD mask, WORD gpioValues )
{
    USB_SPI_STATUS status;
    BYTE data[16];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 4;
    SetupPacket.Request     = SET_GPIO_VALUES;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;

    ZeroMemory(data, sizeof(data));
    BYTE byteOffset, byteMask;
    for(int i = 0; i < CP213x_NUM_GPIO; i++)
    {
        byteOffset = gpioValuesByteMap[i].offset;
        byteMask = gpioValuesByteMap[i].mask;
        if(mask & 1<<i)
        {
            // Set corresponding bit mask (byte 2 or 3)
            data[byteOffset+2] |= byteMask;
            // Set corresponding bit value (byte 0 or 1)
            if(gpioValues & 1<<i)
                data[byteOffset] |= gpioValuesByteMap[i].mask;
            else
                data[byteOffset] &= ~(gpioValuesByteMap[i].mask);
        }
    }
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    return status;
}


/*!
 * \brief Get the GPIO values
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] gpioValues The GPIO values
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetGpioValues(CP213x_DEVICE hDevice, WORD* gpioValues)
{
    USB_SPI_STATUS status;

    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;
    BYTE data[16];
    
    if (gpioValues == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    } 
    
    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 2;
    SetupPacket.Request     = GET_GPIO_VALUES;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    if(!status)
    {
        WORD valueWord = 0;
        BYTE offset, mask;
        for(int i = 0; i < CP213x_NUM_GPIO; i++)
        {
            offset = gpioValuesByteMap[i].offset;
            mask = gpioValuesByteMap[i].mask;
            if(data[offset] & mask)
                valueWord |= 1<<i;  // Set corresponding bit
        }
        *gpioValues = valueWord;
    }
    return status;
}


/*!
 * \brief Set the Event Count value
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] mode Event Counter mode
 * \param[in] count Event count value to set
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetEventCounter(CP213x_DEVICE hDevice, BYTE mode, WORD count)
{
    USB_SPI_STATUS status;
    BYTE data[16];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    data[0] = mode;                             // event count mode
    data[1] = (BYTE)((count & 0xFF00) >> 8);    // counter MSB
    data[2] = (BYTE)(count & 0x00FF);           // counter LSB

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 3;
    SetupPacket.Request     = SET_EVENT_COUNTER;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    return status;
}


/*!
 * \brief Get the Event Count value
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] mode The current Event Counter mode
 * \param[out] count The current Event Counter count
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetEventCounter(CP213x_DEVICE hDevice, BYTE* mode, WORD* count)
{
    USB_SPI_STATUS status;
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;
    BYTE data[16];

    if ((mode == NULL) || (count == NULL)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 3;
    SetupPacket.Request     = GET_EVENT_COUNTER;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    if(!status)
    {
        *mode = data[0];
        *count = (data[1] << 8) | data[2];
    }
    return status;
}

/*!
 * \brief Set clock divider
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] clockDivider Clock divider value
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetClockDivider(CP213x_DEVICE hDevice, BYTE clockDivider)
{
   USB_SPI_STATUS status;
   BYTE data[16];
   SETUP_PACKET SetupPacket;
   DWORD cbRead = 0;

   data[0] = clockDivider;

   ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
   SetupPacket.Length       = 1;
   SetupPacket.Request      = SET_CLOCK_DIVIDER;
   SetupPacket.Index        = 0;
   SetupPacket.RequestType  = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
   status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
   return status;
}


/*!
 * \brief Get clock divider
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] pClockDivider Clock divider value
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetClockDivider(CP213x_DEVICE hDevice, BYTE* pClockDivider)
{
    USB_SPI_STATUS status;
    SETUP_PACKET SetupPacket;
    BYTE data[16];
    DWORD cbRead = 0;
    
    if (pClockDivider == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length       = 0x01;
    SetupPacket.Request      = GET_CLOCK_DIVIDER;
    SetupPacket.Index        = 0;
    SetupPacket.RequestType  = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value        = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    if(!status)
    {
        *pClockDivider = data[0];
    }
   return status;
}



/*!
 * \internal
 * \brief Get the state of the MultiMaster mode
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] state The state of multi-master mode
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetMultiMasterState(CP213x_DEVICE hDevice, BYTE* state)
{
    USB_SPI_STATUS status;
    BYTE data[16];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    if (state == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length       = 0x01;
    SetupPacket.Request      = GET_MULTI_MASTER_STATE;
    SetupPacket.Index        = 0;
    SetupPacket.RequestType  = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value        = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    if(!status)
    {
       *state = data[0];
    }
    return status;
}


/*!
 * \internal
 * \brief Set the control of the MultiMaster mode
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] multiMasterControl The multi-master mode control value
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetMultiMasterControl(CP213x_DEVICE hDevice, BYTE multiMasterControl )
{
    USB_SPI_STATUS status;
    BYTE data[16];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    data[0] = multiMasterControl;

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 1;
    SetupPacket.Request     = SET_MULTIMASTER_CONTROL;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    return status;
}


/*!
 * \internal
 * \brief Set the configuration of the MultiMaster mode
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] multiMasterConfig The multi-master config value
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetMultiMasterConfig(CP213x_DEVICE hDevice, BYTE multiMasterConfig)
{
    USB_SPI_STATUS status;
    BYTE data[16];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    data[0] = multiMasterConfig;

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 1;
    SetupPacket.Request     = SET_MULTIMASTER_CONFIG;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    return status;
}


/*!
 * \brief Perform a USB control transfer
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] SetupPacket USB Setup Packet
 * \param[in] Buffer Data to transfer
 * \param[out] LengthTransferred Number of bytes actually transferred
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS CP213x_ControlTransfer(CP213x_DEVICE hDevice, SETUP_PACKET SetupPacket, BYTE* Buffer, DWORD* LengthTransferred)
{
    USB_SPI_STATUS status;
    CCP213xDevice* dev = (CCP213xDevice*)hDevice;
    
    // Check device object
    if (DeviceList.Validate(dev))
    {
        status = dev->ControlTransfer(SetupPacket, Buffer, LengthTransferred);
    }
    else
    {
        status = USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
    }
     
    return status;
}

//=================================================================================================================================
// Data Transfer Functions
//=================================================================================================================================

/*!
 * \brief Perform SPI Write
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] pWriteBuf Buffer of data to write
 * \param[in] length Number of bytes to write
 * \param[in] releaseBusAfterTransfer 1: Release buffer after transfer is complete
 * \param[in] timeoutMs Timeout (ms)
 * \param[out] pBytesActuallyWritten Number of bytes actually written
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT
 * \return USB_SPI_ERRCODE_PIPE_WRITE_FAIL
 * \return USB_SPI_ERRCODE_INVALID_ENUM_VALUE
 */
USB_SPI_STATUS
CP213x_TransferWrite(CP213x_DEVICE hDevice, BYTE pWriteBuf[], DWORD length, BOOL releaseBusAfterTransfer, DWORD timeoutMs, DWORD * pBytesActuallyWritten)
{
    int status;
    BYTE cmdBuffer[EP_BUFSIZE];
    DWORD dataBytesInFirstPacket = 0;
    DWORD bytesWritten;

    if ((pWriteBuf == NULL) || (pBytesActuallyWritten == NULL)){
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    DPRINT("\r\nWrite()\r\n");
    cmdBuffer[0] = CMD_TRANSFER_DATA;       // Transfer Command LSB
    cmdBuffer[1] = 0x00;                    // Transfer Command MSB
    cmdBuffer[2] = SUBCMD_WRITE;            // SubCommand LSB
    // For SubCommand MSB, the device expects only 0x00 or SUBCMD_MSB_RELEASE_BUS
    releaseBusAfterTransfer ? cmdBuffer[3] = SUBCMD_MSB_RELEASE_BUS : cmdBuffer[3] = 0;

    cmdBuffer[4] = BYTE(length);
    cmdBuffer[5] = BYTE(length >> 8);
    cmdBuffer[6] = BYTE(length >> 16);
    cmdBuffer[7] = BYTE(length >> 24);

    if (length + CMD_SIZE > EP_BUFSIZE)
    {
        dataBytesInFirstPacket = EP_BUFSIZE - CMD_SIZE;
    }
    else
    {
        dataBytesInFirstPacket = length;
    }

    memcpy(&cmdBuffer[CMD_SIZE], pWriteBuf, dataBytesInFirstPacket);
    status = ReadWritePipe(hDevice, PIPE_RW_TYPE_BULK_WRITE, dataBytesInFirstPacket + CMD_SIZE, cmdBuffer, CMD_TIMEOUT_MS, &bytesWritten);
    if (status)
    {
        DPRINT("\r\nERROR: First ReadWritePipe(WRITE) returned error: %d.\r\n", status);
        goto done;
    }
    else
    {
        *pBytesActuallyWritten = bytesWritten - CMD_SIZE;   // Transferred count does not include command size
        if (length > (EP_BUFSIZE - CMD_SIZE))
        {
            // Write remaining bytes
            status = ReadWritePipe(hDevice,
                                    PIPE_RW_TYPE_BULK_WRITE,
                                    (length - (EP_BUFSIZE - CMD_SIZE)),     // Remaining length
                                    (pWriteBuf + (EP_BUFSIZE - CMD_SIZE)),  // Offset in write buffer
                                    timeoutMs, &bytesWritten);
            if(status)
            {
                DPRINT("\r\nERROR: Second ReadWritePipe(WRITE) returned error: %d.\r\n", status);
                goto done;
            }

            *pBytesActuallyWritten += bytesWritten;
        }
    }
    // Compare number of bytes actually written with requested value
    if(*pBytesActuallyWritten != length)
    {
        DPRINT("\r\nWrite: Number of bytes transferred (%d) is not as requested (%d).\r\n", *pBytesActuallyWritten, length);
    }

done:
    return (USB_SPI_STATUS) status;
}// end CP213x_TransferWrite


#if defined(__linux__)
/*!
 * \brief Perform SPI Read (synchronous)
 *
 * \details This function returns after 'length' bytes have been read, or a timeout or error occurs
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] pReadBuf Buffer of data to read
 * \param[in] length Number of bytes to read
 * \param[in] releaseBusAfterTransfer 1: Release buffer after transfer is complete
 * \param[in] timeoutMs Timeout (ms)
 * \param[out] pBytesActuallyRead Number of bytes actually read
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT
 * \return USB_SPI_ERRCODE_PIPE_READ_FAIL
 * \return USB_SPI_ERRCODE_INVALID_ENUM_VALUE
 */
USB_SPI_STATUS
CP213x_TransferReadSync(CP213x_DEVICE hDevice, BYTE pReadBuf[], DWORD length, BOOL releaseBusAfterTransfer, DWORD timeoutMs, DWORD * pBytesActuallyRead)
{
    int status;
    CCP213xDevice* dev = (CCP213xDevice*)hDevice;
    BYTE cmdBuffer[EP_BUFSIZE];
    DWORD bytesWritten = 0;
    DWORD bytesReadThisTime = 0;
    DWORD totalBytesRead = 0;
    DWORD bytesToRead;
    int waitObject;
    fd_set set;
    struct timeval selTimeout;
    int selResult = 0;
    int waitResult = 0;
    uint64_t waitStatus = 0;
    
    // Check device object
    if (!DeviceList.Validate(dev)) {
        return USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
    }
        
    if( (pReadBuf == NULL) ||
        (0 == length) ||
        (pBytesActuallyRead == NULL)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }    

    *pBytesActuallyRead = 0;
    
    DPRINT("\r\nRead_Sync()\r\n");
    // Ensure requested read size is less than WinUsb maximum
    bytesToRead = length;
    if (bytesToRead > MAX_BULK_BUFFER_SIZE)
    {
        //DPRINT("\r\nRead_Sync: Reducing bytesToRead (%ld) to inEpMaxTransferSize (%ld)", bytesToRead, MAX_BULK_BUFFER_SIZE);
        bytesToRead = MAX_BULK_BUFFER_SIZE;
    }

    // Setup a wait object to determine when a read has occurred
    waitObject = eventfd(0, 0);
    if (waitObject == -1) {
        return USB_SPI_ERRCODE_SYSTEM_ERROR;
    }
    
    // Call Read before calling Write
    status = dev->BulkTransfer_Asynch(dev->GetInEndpointAddress(), pReadBuf, bytesToRead, &bytesReadThisTime, timeoutMs, waitObject);
    if (status != USB_SPI_ERRCODE_SUCCESS) {
        DPRINT("\r\nERROR: Read_Sync: BulkTransfer_Asynch did not return error, as expected, status = %d!\r\n", status);
        goto done;
    }

    // Send SPI READ command
    cmdBuffer[0] = CMD_TRANSFER_DATA;       // Transfer Command LSB
    cmdBuffer[1] = 0x00;                    // Transfer Command MSB
    cmdBuffer[2] = SUBCMD_READ;             // SubCommand LSB
    // For SubCommand MSB, the device expects only 0x00 or SUBCMD_MSB_RELEASE_BUS
    releaseBusAfterTransfer ? cmdBuffer[3] = SUBCMD_MSB_RELEASE_BUS : cmdBuffer[3] = 0;
    cmdBuffer[4] = BYTE(length);
    cmdBuffer[5] = BYTE(length >> 8);
    cmdBuffer[6] = BYTE(length >> 16);
    cmdBuffer[7] = BYTE(length >> 24);
    status = ReadWritePipe(hDevice, PIPE_RW_TYPE_BULK_WRITE, CMD_SIZE, cmdBuffer, CMD_TIMEOUT_MS, &bytesWritten);
    if(status) {
        DPRINT("\r\nERROR: Read_Sync: ReadWritePipe(WRITE) returned status %d.\r\n", status);
        goto done;
    }

    while(1) {
        // Wait for previously-launched Read to finish
        do {
            FD_ZERO(&set);
            FD_SET(waitObject, &set);
            selTimeout.tv_sec = 0;
            selTimeout.tv_usec = 10000;
            selResult = select(waitObject + 1, &set, NULL, NULL, &selTimeout);
            if (selResult == -1)
                break;
            else if (selResult > 0) {
                waitResult = read(waitObject, &waitStatus, sizeof(uint64_t));
            } else {
                struct timeval usbTimeout;
                usbTimeout.tv_sec = 0;
                usbTimeout.tv_usec = 100;
                libusb_handle_events_timeout(NULL, &usbTimeout);
            }
        } while ((selResult == 0) && (waitResult == 0));

        if (0 == waitResult) {
            status = USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT;
            bytesReadThisTime = 0;
            DPRINT("\r\n***** Read_Sync: TIMEOUT break");
            break;
        } else {
            if (waitStatus == 1) {
                totalBytesRead += bytesReadThisTime;
                //DPRINT("\r\nRead_Sync: bytesReadThisTime = %d\ttotalBytesRead = %d", bytesReadThisTime, totalBytesRead);
                if(totalBytesRead >= length) {
                    //DPRINT("\r\n***** Read_Sync: READ COMPLETE break");
                    break;  // All bytes have been read; don't launch another Read operation
                } else {
                    //DPRINT("\r\n***** Read_Sync: totalBytesRead (%ld) not >= length (%ld)", totalBytesRead, length);
                }
            } else { //waitStatus == 2
                status = USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT;
                bytesReadThisTime = 0;
                DPRINT("\r\n***** Read_Sync: TIMEOUT break");
                break;
            }
        }
        // Launch another Read operation if necessary
        //DPRINT("\r\n***** Read_Sync: Launching another Read operation");
        bytesToRead = length - totalBytesRead;
        if (bytesToRead > MAX_BULK_BUFFER_SIZE) {
            bytesToRead = MAX_BULK_BUFFER_SIZE;
        }
        bytesReadThisTime = 0;
        close(waitObject);
        waitObject = eventfd(0, 0);
        waitStatus = 0;
        waitResult = 0;
        selResult = 0;
        if (waitObject == -1) {
            return USB_SPI_ERRCODE_SYSTEM_ERROR;
        }
        status = dev->BulkTransfer_Asynch(dev->GetInEndpointAddress(), pReadBuf, bytesToRead, &bytesReadThisTime, timeoutMs, waitObject);
        if (status != USB_SPI_ERRCODE_SUCCESS) {
            // Unexpected error
            DPRINT("\r\nERROR: Read_Sync: BulkTransfer_Asynch did not return error, as expected, status = %d!\r\n", status);
            break;
        }
    }// end while(1)

    close(waitObject);
    
    // Compare number of bytes actually read with requested value
    if(totalBytesRead != length) {
        DPRINT("\r\nRead_Sync: Number of bytes transferred (%d) is not as requested (%d).", totalBytesRead, length);
        status = USB_SPI_ERRCODE_PIPE_READ_FAIL;
    }

done:
    *pBytesActuallyRead = totalBytesRead;
    return (USB_SPI_STATUS) status;
}// end CP213x_TransferReadSync


/*!
 * \brief Perform SPI Write/Read (synchronous)
 *
 * \details This function returns after 'length' bytes have been written and read, or a timeout or error occurs
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] pWriteBuf Buffer of data to write
 * \param[in] pReadBuf Buffer of data to read
 * \param[in] length Number of bytes to write and read
 * \param[in] releaseBusAfterTransfer Release buf after transfer is complete
 * \param[in] timeoutMs Timeout (ms)
 * \param[out] pBytesActuallyTransferred Number of bytes actually written and read
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT
 * \return USB_SPI_ERRCODE_PIPE_WRITE_FAIL
 * \return USB_SPI_ERRCODE_PIPE_READ_FAIL
 * \return USB_SPI_ERRCODE_INVALID_ENUM_VALUE
 */
USB_SPI_STATUS
CP213x_TransferWriteRead(CP213x_DEVICE hDevice, BYTE pWriteBuf[], BYTE pReadBuf[], DWORD length, BOOL releaseBusAfterTransfer, DWORD timeoutMs, DWORD * pBytesActuallyTransferred)
{
    int status;
    CCP213xDevice* dev = (CCP213xDevice*)hDevice;
    BYTE cmdBuffer[EP_BUFSIZE];
    DWORD bytesToTransfer = 0;
    DWORD totalBytesWritten = 0;
    DWORD bytesWrittenThisTime = 0;
    DWORD bytesReadThisTime = 0;
    DWORD totalBytesRead = 0;
    DWORD bytesToRead = length;
    int waitObject;
    fd_set set;
    struct timeval selTimeout;
    int selResult = 0;
    int waitResult = 0;
    uint64_t waitStatus = 0;

    // Check device object
    if (!DeviceList.Validate(dev)) {
        return USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
    }
    
    if ((pWriteBuf == NULL) || (pReadBuf == NULL) || (pBytesActuallyTransferred == NULL) || (0 == length)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
         
    *pBytesActuallyTransferred = 0;
    
    // Setup a wait object to determine when a read has occurred
    waitObject = eventfd(0, 0);
    if (waitObject == -1) {
        return USB_SPI_ERRCODE_SYSTEM_ERROR;
    }
    
    // Call Read before calling Write
    status = dev->BulkTransfer_Asynch(dev->GetInEndpointAddress(), pReadBuf, bytesToRead, &bytesReadThisTime, timeoutMs, waitObject);
    if (status != USB_SPI_ERRCODE_SUCCESS) {
        DPRINT("\r\nERROR: Read_Sync: BulkTransfer_Asynch did not return error, as expected, status = %d!\r\n", status);
        goto done;
    }

    // Send SPI WRITEREAD command
    cmdBuffer[0] = CMD_TRANSFER_DATA;  // Transfer Command LSB
    cmdBuffer[1] = 0x00;               // Transfer Command MSB
    cmdBuffer[2] = SUBCMD_WRITEREAD;   // SubCommand LSB
    // For SubCommand MSB, the device expects only 0x00 or SUBCMD_MSB_RELEASE_BUS
    releaseBusAfterTransfer ? cmdBuffer[3] = SUBCMD_MSB_RELEASE_BUS : cmdBuffer[3] = 0;
    cmdBuffer[4] = BYTE(length);
    cmdBuffer[5] = BYTE(length >> 8);
    cmdBuffer[6] = BYTE(length >> 16);
    cmdBuffer[7] = BYTE(length >> 24);

    if (length + CMD_SIZE > EP_BUFSIZE) {
        bytesToTransfer = EP_BUFSIZE - CMD_SIZE;
    } else {
        bytesToTransfer = length;
    }
    memcpy(&cmdBuffer[CMD_SIZE], &pWriteBuf[0], bytesToTransfer);  // Fill the rest of the packet with data from WriteBuf.
    //DPRINT("\r\nWriteRead: Calling ReadWritePipe(WRITE, %d).\r\n", bytesToTransfer + CMD_SIZE);
    status = ReadWritePipe(hDevice, PIPE_RW_TYPE_BULK_WRITE, bytesToTransfer + CMD_SIZE, cmdBuffer, CMD_TIMEOUT_MS, &bytesWrittenThisTime);
    if (status != USB_SPI_ERRCODE_SUCCESS) {
        DPRINT("\r\nERROR: First ReadWritePipe(WRITE) returned error: %d.\r\n", status);
        goto done;
    }

    //DPRINT("\r\nFirst ReadWritePipe(WRITE) returned SUCCESS.\r\n");
    totalBytesWritten += bytesWrittenThisTime - CMD_SIZE;   // Transferred count does not include command size

    // Wait for previously-launched Read to finish
    do {
        FD_ZERO(&set);
        FD_SET(waitObject, &set);
        selTimeout.tv_sec = 0;
        selTimeout.tv_usec = 10000/40;
        selResult = select(waitObject + 1, &set, NULL, NULL, &selTimeout);
        if (selResult == -1)
            break;
        else if (selResult > 0) {
            waitResult = read(waitObject, &waitStatus, sizeof(uint64_t));
        } else {
            struct timeval usbTimeout;
            usbTimeout.tv_sec = 0;
            usbTimeout.tv_usec = 100;
            libusb_handle_events_timeout(NULL, &usbTimeout);
        }
    } while ((selResult == 0) && (waitResult == 0));

    if (0 == waitResult) {
        status = USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT;
        bytesReadThisTime = 0;
        DPRINT("\r\n***** WriteRead(READ): TIMEOUT");
    } else  {
        totalBytesRead += bytesReadThisTime;
    }

    if (length > (EP_BUFSIZE - CMD_SIZE)) {
        // Launch the second ReadPipe operation before calling WritePipe
        bytesToRead = length - totalBytesRead;
        if (bytesToRead > MAX_BULK_BUFFER_SIZE)
        {
            bytesToRead = MAX_BULK_BUFFER_SIZE;
        }
        bytesReadThisTime = 0;

        // Reset a wait object to determine when a read has occurred
        bytesReadThisTime = 0;
        close(waitObject);
        waitObject = eventfd(0, 0);
        waitStatus = 0;
        waitResult = 0;
        selResult = 0;
        if (waitObject == -1) {
            return USB_SPI_ERRCODE_SYSTEM_ERROR;
        }

        //DPRINT("\r\n***** WriteRead: Launching the second Read operation (%d bytes)", bytesToRead);
        status = dev->BulkTransfer_Asynch(dev->GetInEndpointAddress(), pReadBuf, bytesToRead, &bytesReadThisTime, timeoutMs, waitObject);
        if (status != USB_SPI_ERRCODE_SUCCESS) {
            DPRINT("\r\nERROR: WriteRead: BulkTransfer_Asynch did not return error, as expected, status = %d!\r\n", status);
            goto done;
        }

        // Write remaining bytes
        //DPRINT("\r\nWriteRead: Second call to ReadWritePipe(WRITE, %d).\r\n", length - (EP_BUFSIZE - CMD_SIZE));
        status = ReadWritePipe(hDevice,
                                PIPE_RW_TYPE_BULK_WRITE,
                                (length - (EP_BUFSIZE - CMD_SIZE)),     // Remaining length
                                (pWriteBuf + (EP_BUFSIZE - CMD_SIZE)),  // Offset in write buffer
                                timeoutMs, &bytesWrittenThisTime);
        if (status != USB_SPI_ERRCODE_SUCCESS) {
            DPRINT("\r\nERROR: Second ReadWritePipe(WRITE) returned error: 0x%02X (%d).\r\n", status, status);
            goto done;
        } else {
            //DPRINT("\r\nSecond ReadWritePipe(WRITE) returned SUCCESS.\r\n");
            totalBytesWritten += bytesWrittenThisTime;
        }

        // Wait for second Read to finish
        do {
            FD_ZERO(&set);
            FD_SET(waitObject, &set);
            selTimeout.tv_sec = 0;
            selTimeout.tv_usec = 10000/40;
            selResult = select(waitObject + 1, &set, NULL, NULL, &selTimeout);
            if (selResult == -1)
                break;
            else if (selResult > 0) {
                waitResult = read(waitObject, &waitStatus, sizeof(uint64_t));
            } else {
                struct timeval usbTimeout;
                usbTimeout.tv_sec = 0;
                usbTimeout.tv_usec = 100;
                libusb_handle_events_timeout(NULL, &usbTimeout);
            }
        } while ((selResult == 0) && (waitResult == 0));

        if (0 == waitResult) {
            status = USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT;
            bytesReadThisTime = 0;
            DPRINT("\r\n***** Read_Sync: TIMEOUT break");
        } else {
            totalBytesRead += bytesReadThisTime;
            //DPRINT("\r\nWriteRead: bytesReadThisTime = %d\ttotalBytesRead = %d", bytesReadThisTime, totalBytesRead);
            if(totalBytesRead >= length) {
                //DPRINT("\r\n***** WriteRead: READ is complete.\r\n");
            } else {
                DPRINT("\r\n***** WriteRead: totalBytesRead (%ld) not >= length (%ld)", totalBytesRead, length);
            }
        }
    } // if (length > (EP_BUFSIZE - CMD_SIZE))

    // Compare number of bytes actually written with requested value
    if (totalBytesWritten != length) {
        DPRINT("\r\nWARNING: WriteRead: Number of bytes written (%d) is not as requested (%d).\r\n", totalBytesWritten, length);
    }

done:
    *pBytesActuallyTransferred = totalBytesRead;

    close(waitObject);

    // Compare number of bytes actually read with requested value
    if(totalBytesRead != length)
    {
        DPRINT("\r\nWriteRead: Number of bytes transferred (%d) is not as requested (%d).", totalBytesRead, length);
        status = USB_SPI_ERRCODE_PIPE_READ_FAIL;
    }
    return (USB_SPI_STATUS) status;
}// end CP213x_TransferWriteRead
#endif // defined(__linux__)


/*!
 * \brief Perform SPI Read with RTR (synchronous)
 *
 * \details The device reads SPI data in 'blockSize' chunks while the RTR pin is asserted.
 *  This function returns after 'totalSize' bytes have been read, or a timeout or error occurs.
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] pReadBuf Buffer of data to read
 * \param[in] totalSize Total number of bytes to read
 * \param[in] blockSize Number of bytes to read at a time
 * \param[in] releaseBusAfterTransfer 1: Release buffer after transfer is complete
 * \param[in] timeoutMs Timeout (ms)
 * \param[out] pBytesActuallyRead Number of bytes actually read
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_PIPE_ABORT_FAIL
 * \return USB_SPI_ERRCODE_PIPE_FLUSH_FAIL
 * \return USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT
 * \return USB_SPI_ERRCODE_PIPE_WRITE_FAIL
 */
USB_SPI_STATUS
CP213x_TransferReadRtrSync(  CP213x_DEVICE hDevice,
                        BYTE pReadBuf[],
                        DWORD totalSize,
                        DWORD blockSize,
                        BOOL releaseBusAfterTransfer,
                        DWORD timeoutMs,
                        DWORD * pBytesActuallyRead)
{
    int status;
    CCP213xDevice* dev = (CCP213xDevice*)hDevice;
    BYTE cmdBuffer[EP_BUFSIZE];
    DWORD bytesToRead;
    DWORD bytesRead;
    DWORD bytesWritten;
    DWORD totalBytesRead = 0;

    DPRINT("\r\nRTRead_Sync()\r\n");

    if (pBytesActuallyRead == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    // Check device object
    if (!DeviceList.Validate(dev)) {
        return USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
    }
    
    // Check parameters
    if( (pReadBuf == NULL) ||
        (0 == totalSize) ||
        (blockSize > totalSize) ||
        (pBytesActuallyRead == NULL)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    cmdBuffer[0] = CMD_TRANSFER_DATA;  // Transfer Command LSB
    cmdBuffer[1] = 0x00;               // Transfer Command MSB
    cmdBuffer[2] = SUBCMD_RTREAD;      // SubCommand LSB
    // For SubCommand MSB, the device expects only 0x00 or SUBCMD_MSB_RELEASE_BUS
    releaseBusAfterTransfer ? cmdBuffer[3] = SUBCMD_MSB_RELEASE_BUS : cmdBuffer[3] = 0;

    cmdBuffer[4] = (BYTE)(totalSize);
    cmdBuffer[5] = (BYTE)(totalSize >> 8);
    cmdBuffer[6] = (BYTE)(totalSize >> 16);
    cmdBuffer[7] = (BYTE)(totalSize >> 24);

    //DPRINT("Command_RTRead: Sending command packet (8 bytes)\r\n");
    status = ReadWritePipe(hDevice, PIPE_RW_TYPE_BULK_WRITE, CMD_SIZE, cmdBuffer, CMD_TIMEOUT_MS, &bytesWritten);
    if(status != USB_SPI_ERRCODE_SUCCESS) {
        DPRINT("\r\nERROR: RTRead_Sync -> ReadWritePipe(WRITE) returned %02X\r\n", status);
        goto done;
    } else {
        for (totalBytesRead = 0; totalBytesRead  < totalSize; ) {
            bytesToRead = totalSize - totalBytesRead;
            if (bytesToRead > blockSize) {
                bytesToRead = blockSize;
            }

            //DPRINT("Waiting to read %d bytes...\r\n", bytesToRead);

            status = ReadWritePipe(hDevice, PIPE_RW_TYPE_BULK_READ, bytesToRead, pReadBuf, timeoutMs, &bytesRead);
            if(status != USB_SPI_ERRCODE_SUCCESS) {
                DPRINT("\r\nERROR: RTRead_Sync -> ReadWritePipe(READ) returned %02X\r\n", status);
                goto done;
            } else {
                totalBytesRead += bytesRead;
                DPRINT("\r\nRTRead_Sync: Received %d bytes. Total = %d\r\n", bytesRead, totalBytesRead);
            }
        }   // end for
    }

done:
    *pBytesActuallyRead = totalBytesRead;

    // Exit RTR mode
    if(CP213x_SetRtrStop(hDevice, 1) != USB_SPI_ERRCODE_SUCCESS) {
        DPRINT("\r\nERROR: RTRead_Sync -> RTRStop_Set returned %02X\r\n", status);
    } else {
        DPRINT("\r\nRTRead_Sync: Exited RTR mode.\r\n");
    }

    // Compare number of bytes actually read with requested value
    if(*pBytesActuallyRead != totalSize) {
        DPRINT("\r\nRTRead_Sync: Number of bytes transferred (%d) is not as requested (%d).\r\n", totalBytesRead, totalSize);
    }
    return (USB_SPI_STATUS) status;
}// end CP213x_TransferReadRtrSync


/*!
 * \brief Perform SPI Read with RTR (asynchronous)
 *
 * \details The device reads SPI data in 'blockSize' chunks while the RTR pin is asserted.
 *  This function returns immediately.  The application should call CP213x_ReadPoll() periodically to read data.
 *  When 'totalSize' bytes have been read, the Read operation is terminated.  To terminate the operation before 
 *  then, the application should call 'CP213x_ReadAbort'.
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] totalSize Total number of bytes to read
 * \param[in] blockSize Number of bytes to read at a time
 * \param[in] releaseBusAfterTransfer Release buf after transfer is complete
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_PARAMETER
 * \return USB_SPI_ERRCODE_READ_THREAD_CREATE_FAILURE
 * \return USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT
 * \return USB_SPI_ERRCODE_INVALID_ENUM_VALUE
 * \return USB_SPI_ERRCODE_PIPE_READ_FAIL
 * \return USB_SPI_ERRCODE_READ_THREAD_START_FAILURE
 */
USB_SPI_STATUS
CP213x_TransferReadRtrAsync(  CP213x_DEVICE hDevice,
                        DWORD totalSize,
                        DWORD blockSize,
                        BOOL releaseBusAfterTransfer)
{
    int status;
    CCP213xDevice* dev = (CCP213xDevice*)hDevice;
    BYTE cmdBuffer[EP_BUFSIZE];
    DWORD bytesWritten;

    DPRINT("\r\nRTRead_Async()\r\n");
    
    // Check device object
    if (!DeviceList.Validate(dev)) {
        return USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
    }
    
    // Check parameters
    if( (0 == totalSize) ||
        (blockSize > totalSize)) {
        DPRINT("\r\nRead_Async parameter check FAILED.\r\n");
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    if (dev->IsAsyncReadInProgress()) {
        return USB_SPI_ERRCODE_READ_THREAD_CREATE_FAILURE;
    }
    
    // Start Read thread before sending Read command to CP2130
    dev->SetupAsyncReadThreadParam(blockSize, totalSize, releaseBusAfterTransfer, 10);
    status =  pthread_create(dev->GetAsyncReadThread(), NULL, &readThread, (void*)dev->GetAsyncReadThreadParam());
    if(status != 0) {
        //printf("Error: pthread_create() failed\n");
        return USB_SPI_ERRCODE_READ_THREAD_CREATE_FAILURE;
    }

    cmdBuffer[0] = CMD_TRANSFER_DATA;  // Transfer Command LSB
    cmdBuffer[1] = 0x00;               // Transfer Command MSB
    cmdBuffer[2] = SUBCMD_RTREAD;      // SubCommand LSB
    // For SubCommand MSB, the device expects only 0x00 or SUBCMD_MSB_RELEASE_BUS
    releaseBusAfterTransfer ? cmdBuffer[3] = SUBCMD_MSB_RELEASE_BUS : cmdBuffer[3] = 0;

    cmdBuffer[4] = (BYTE)(totalSize);
    cmdBuffer[5] = (BYTE)(totalSize >> 8);
    cmdBuffer[6] = (BYTE)(totalSize >> 16);
    cmdBuffer[7] = (BYTE)(totalSize >> 24);

    //DPRINT("Command_RTRead: Sending command packet (8 bytes)\r\n");
    status = ReadWritePipe(hDevice, PIPE_RW_TYPE_BULK_WRITE, CMD_SIZE, cmdBuffer, CMD_TIMEOUT_MS, &bytesWritten);
    if(status) {
        DPRINT("\r\nERROR: RTRead -> ReadWritePipe(WRITE) returned %02X\r\n", status);
        goto error;
    }

    return (USB_SPI_STATUS) status;

error:

    // Exit RTR mode
    if(CP213x_SetRtrStop(hDevice, 1) != USB_SPI_ERRCODE_SUCCESS) {
        DPRINT("\r\nERROR: RTRead_Async -> RTRStop_Set\r\n");
    } else {
        DPRINT("\r\nRTRead_Async: Exited RTR mode.\r\n");
    }

    return (USB_SPI_STATUS) status;
}// end CP213x_TransferReadRtrAsync


/*!
 * \brief Perform SPI Read (asynchronous)
 *
 * \details The device reads SPI data in 'blockSize' chunks.
 *  This function returns immediately.  The application should call CP213x_ReadPoll() periodically to read data.
 *  When 'totalSize' bytes have been read, the Read operation is terminated.  To terminate the operation before 
 *  then, the application should call 'CP213x_ReadAbort'.
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] totalSize Total number of bytes to read
 * \param[in] blockSize Number of bytes to read at a time
 * \param[in] releaseBusAfterTransfer Release buf after transfer is complete
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_READ_THREAD_CREATE_FAILURE
 * \return USB_SPI_ERRCODE_PIPE_ABORT_FAIL
 * \return USB_SPI_ERRCODE_PIPE_FLUSH_FAIL
 * \return USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT
 * \return USB_SPI_ERRCODE_PIPE_WRITE_FAIL
 * \return USB_SPI_ERRCODE_READ_THREAD_START_FAILURE
 */
USB_SPI_STATUS
CP213x_TransferReadAsync(CP213x_DEVICE hDevice, DWORD totalSize, DWORD blockSize, BOOL releaseBusAfterTransfer)
{
    int status;
    CCP213xDevice* dev = (CCP213xDevice*)hDevice;
    BYTE cmdBuffer[EP_BUFSIZE];
    DWORD bytesWritten;

    DPRINT("\r\nRead_Async()\r\n");

    // Check device object
    if (!DeviceList.Validate(dev)) {
        return USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
    }
    
    // Check parameters
    if( (0 == totalSize) ||
        (blockSize > totalSize)) {
        DPRINT("\r\nRead_Async parameter check FAILED.\r\n");
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    if (dev->IsAsyncReadInProgress()) {
        return USB_SPI_ERRCODE_READ_THREAD_CREATE_FAILURE;
    }
    
    // Start Read thread before sending Read command to CP2130
    dev->SetupAsyncReadThreadParam(blockSize, totalSize, releaseBusAfterTransfer, 10);
    status =  pthread_create(dev->GetAsyncReadThread(), NULL, &readThread, (void*)dev->GetAsyncReadThreadParam());
    if(status != 0) {
        //printf("Error: pthread_create() failed\n");
        return USB_SPI_ERRCODE_READ_THREAD_CREATE_FAILURE;
    }

    //
    // Send Read command to CP2130
    //
    cmdBuffer[0] = CMD_TRANSFER_DATA;   // Transfer Command LSB
    cmdBuffer[1] = 0x00;                // Transfer Command MSB
    cmdBuffer[2] = SUBCMD_READ;         // SubCommand LSB
    // For SubCommand MSB, the device expects only 0x00 or SUBCMD_MSB_RELEASE_BUS
    releaseBusAfterTransfer ? cmdBuffer[3] = SUBCMD_MSB_RELEASE_BUS : cmdBuffer[3] = 0;

    cmdBuffer[4] = BYTE(totalSize);
    cmdBuffer[5] = BYTE(totalSize >> 8);
    cmdBuffer[6] = BYTE(totalSize >> 16);
    cmdBuffer[7] = BYTE(totalSize >> 24);

    //DPRINT("Command_Read_Async(): Calling BULK_IN_WRITE ( 8 bytes).\r\n");
    status = ReadWritePipe(hDevice, PIPE_RW_TYPE_BULK_WRITE, CMD_SIZE, cmdBuffer, CMD_TIMEOUT_MS, &bytesWritten);
    if(status)
    {
        DPRINT("\r\nERROR: Read_Async: ReadWritePipe(WRITE) returned %d.\r\n", status);
        return (USB_SPI_STATUS) status;
    }

    return (USB_SPI_STATUS) status;
}// end CP213x_TransferReadAsync


/*!
 * \brief Read bytes received during an asynchronous Read or ReadRTR operation
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] pReadBuf Buffer of data to read
 * \param[in] maxLength Maximum number of bytes to read
 * \param[out] pBytesActuallyRead Number of bytes actually read
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_NULL_POINTER
 * \return USB_SPI_ERRCODE_READ_THREAD_NOT_RUNNING
 */
USB_SPI_STATUS
CP213x_ReadPoll(CP213x_DEVICE hDevice, BYTE *pReadBuf, DWORD maxLength, DWORD * pBytesActuallyRead)
{
    int status = USB_SPI_ERRCODE_SUCCESS;
    DWORD bytesTransferred = 0;
    CCP213xDevice* dev = (CCP213xDevice*)hDevice;
    
    // Check device object
    if (!DeviceList.Validate(dev)) {
        return USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
    }
    
    pthread_mutex_lock(&lock);  // lock the critical section
    
    AsyncReadThreadParam* paramObj = dev->GetAsyncReadThreadParam();
    paramObj->userRead = true;
    DWORD byteNum = 0;

    while(paramObj->buffer.size() > 0 && byteNum < maxLength){
        pReadBuf[byteNum] = paramObj->buffer.front();
        paramObj->buffer.erase(paramObj->buffer.begin());
        byteNum+=1;
    }
    
    pthread_mutex_unlock(&lock); // unlock once you are done

    *pBytesActuallyRead = byteNum;
    if(bytesTransferred != maxLength)   // Only print debug trace if different
    {
        //DPRINT("\r\n*** Read_Poll: maxLength: %d bytesTransferred: %d\r\n", maxLength, bytesTransferred);
    }
    return (USB_SPI_STATUS) status;
}// end CP213x_TransferReadPoll


/*!
 * \brief Abort an asynchronous Read or ReadRTR operation
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_NULL_POINTER
 */
USB_SPI_STATUS
CP213x_ReadAbort(CP213x_DEVICE hDevice)
{
    int status;
    CCP213xDevice* dev = (CCP213xDevice*)hDevice;

    // Check device object
    if (!DeviceList.Validate(dev)) {
        return USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
    }
    
    if (dev->IsAsyncReadInProgress()) {
        dev->CancelAsyncRead();
        status = USB_SPI_ERRCODE_SUCCESS;
    } else {
        DPRINT("\r\nReadAbort(): pReadThread is NULL.");
        status = USB_SPI_ERRCODE_NULL_POINTER;
    }

    return (USB_SPI_STATUS)status;
}


//=================================================================================================================================
// Functions to Set/Get EPROM info (e.g. USB string descriptors, pin config, lock, etc.
//=================================================================================================================================

/*!
 * \brief Set the Lock values
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] lockValue The Lock values to set
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetLock(CP213x_DEVICE hDevice, WORD lockValue)
{
    int status;
    BYTE data[16];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    data[0] = (lockValue & 0xFF00) >> 8; // Lock byte MSB
    data[1] = lockValue & 0xFF;          // Lock byte LSB

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 2;
    SetupPacket.Request     = SET_LOCK_BYTE;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = MEMKEYS;  // Pass Flash/OTP keys here
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);

    return (USB_SPI_STATUS)status;
}


/*!
 * \brief Get the Lock values
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] lockValue The Lock values
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetLock(CP213x_DEVICE hDevice, WORD* lockValue)
{
    USB_SPI_STATUS status;
    BYTE data[16];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    if (lockValue == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 2;
    SetupPacket.Request     = GET_LOCK_BYTE;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    if(!status)
    {
        *lockValue = (data[0] << 8) | data[1];
    }
    return status;
}


/*!
 * \brief Set the USB device Configuration info
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] vid New VID to be set in the device
 * \param[in] pid New PID to be set in the device
 * \param[in] power New Power to be set in the device
 * \param[in] powerMode New Power Mode to be set in the device
 * \param[in] releaseVersion New Release version to be set in the device
 * \param[in] transferPriority New Transfer priority to be set in the device
 * \param[in] mask Mask that represents which of the settings to write to the device
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetUsbConfig(CP213x_DEVICE hDevice, WORD vid, WORD pid, BYTE power, BYTE powerMode, WORD releaseVersion, BYTE transferPriority, BYTE mask)
{
    USB_SPI_STATUS status;
    BYTE data[16];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    data[0] = vid & 0xFF;
    data[1] = (vid & 0xFF00) >> 8;
    data[2] = pid & 0xFF;
    data[3] = (pid & 0xFF00) >> 8;
    data[4] = power;
    data[5] = powerMode;
    data[6] = (releaseVersion & 0xFF00) >> 8;
    data[7] = releaseVersion & 0xFF;
    data[8] = transferPriority;
    data[9] = mask;

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 10;
    SetupPacket.Request     = SET_USB_CONFIG;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = MEMKEYS;  // Pass Flash/OTP keys here
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    return status;
}


/*!
 * \brief Get the USB Device Configuration info
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] vid VID of the device
 * \param[out] pid PID of the device
 * \param[out] power Power of the device
 * \param[out] powerMode Power Mode of the device
 * \param[out] releaseVersion Release version of the device
 * \param[out] transferPriority Transfer priority of the device
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetUsbConfig(CP213x_DEVICE hDevice, WORD* vid, WORD* pid, BYTE* power, BYTE* powerMode, WORD* releaseVersion, BYTE* transferPriority)
{
    USB_SPI_STATUS status;
    BYTE data[SIZE_USB_CONFIG];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    if ((vid == NULL) || (pid == NULL) || (power == NULL) || (powerMode == NULL) || (releaseVersion == NULL) || (transferPriority == NULL)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = SIZE_USB_CONFIG;
    SetupPacket.Request     = GET_USB_CONFIG;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &cbRead);
    if(!status)
    {
        *vid = data[0] | (data[1] << 8);
        *pid = data[2] | (data[3] << 8);
        *power = data[4];
        *powerMode = data[5];
        *releaseVersion = data[7] | (data[6] << 8);
        *transferPriority = data[8];
    }
    return status;
}


/*!
 * \brief Get the USB device Manufacturer string
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] manufacturingString Pointer to buffer that will be populated with Manufacturer string
 * \param[out] strlen Pointer to BYTE that will be set to the Manufacturer string length
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetManufacturingString(CP213x_DEVICE hDevice, LPSTR manufacturingString, BYTE* strlen)
{
    USB_SPI_STATUS status;

    if ((strlen == NULL) || (manufacturingString == NULL)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    status = (USB_SPI_STATUS) GetCustomLongString(hDevice, GET_MANU1, GET_MANU2, manufacturingString, strlen);
    return status;
}


/*!
 * \brief Set the USB device Manufacturer string
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] manufacturingString Buffer containing the USB device Manufacturer string to be set
 * \param[in] strlen The length of the supplied Manufacturer string
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetManufacturingString(CP213x_DEVICE hDevice, LPCSTR manufacturingString, BYTE strlen)
{
    if ((strlen > MFG_STRLEN)  || (manufacturingString == NULL)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    int status = SetCustomLongString(hDevice, SET_MANU1, SET_MANU2, manufacturingString, strlen);
    return (USB_SPI_STATUS) status;
}


/*!
 * \brief Get the USB device Product string
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] productString Buffer that will be populated with the USB device Product string
 * \param[out] strlen Pointer to BYTE that will be set to the Product string length
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetProductString(CP213x_DEVICE hDevice, LPSTR productString, BYTE* strlen)
{
    int status;

    if ((strlen == NULL) || (productString == NULL)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    status = GetCustomLongString(hDevice, GET_PROD1, GET_PROD2, productString, strlen);
    return (USB_SPI_STATUS) status;
}


/*!
 * \brief Set the USB device Product string
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] productString Buffer containing the USB device Product string
 * \param[in] strlen The length of the supplied Product string
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetProductString(CP213x_DEVICE hDevice, LPCSTR productString, BYTE strlen)
{
    if ((strlen > PRODUCT_STRLEN)  || (productString == NULL)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    int status = SetCustomLongString(hDevice, SET_PROD1, SET_PROD2, productString, strlen);
    return (USB_SPI_STATUS) status;
}


/*!
 * \brief Get the USB device Serial Number string
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] serialString Buffer that will be populated with the USB device Serial Number string
 * \param[out] strlen Pointer to BYTE that will be set to the Serial Number string length
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetSerialString(CP213x_DEVICE hDevice, LPSTR serialString, BYTE* strlen)
{
    USB_SPI_STATUS status;
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;
    BYTE payload[64];

    if ((strlen == NULL) || (serialString == NULL)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    *strlen = 0;
    memset(payload, 0x00, sizeof(payload));
    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 64;
    SetupPacket.Request     = GET_SERIAL;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hDevice, SetupPacket, payload, &cbRead);

    if(!status)
    {
        // Get string length from first data packet
        // (size does not include the 0x03 string descriptor constant)
        *strlen = (payload[0] - 1) / 2;

        // Allocate a blank full-size unicode string buffer
        BYTE unicode[SHORT_STRING_LEN];
        memset(unicode, 0x00, sizeof(unicode));

        // Retrieve Unicode string from the device
        memcpy(unicode, &payload[2], SHORT_STRING_LEN);

        // Convert Unicode to ASCII
        for (int i = 0; i < (SHORT_STRING_LEN/2); i++)
        {
            serialString[i] = unicode[i * 2];
        }
    }
    return status;
}


/*!
 * \brief Set the USB device Serial Number string
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in]  serialString Buffer containing the USB device Serial Number string
 * \param[in] strlen The length of the supplied Serial Number string
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetSerialString(CP213x_DEVICE hDevice, LPCSTR serialString, BYTE strlen)
{
    USB_SPI_STATUS status;
    BYTE payload[64];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;
    memset(payload, 0x00, sizeof(payload));

    if ((strlen > SERIAL_STRLEN)  || (serialString == NULL)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    payload[0] = (strlen * 2) + 2;          // String Desc Length
    payload[1] = 0x03;                      // String Desc ID

    // Convert ASCII to Unicode string
    for (int i = 0; i < strlen; i++)
    {
        payload[i*2+2] = serialString[i];
    }

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 64;
    SetupPacket.Request     = SET_SERIAL;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = MEMKEYS;  // Pass Flash/OTP keys here
    status = CP213x_ControlTransfer(hDevice, SetupPacket, payload, &cbRead);
    return status;
}


/*!
 * \brief Get the USB device Pin Configuration info
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] pinConfig Buffer containing the USB device Pin Configuration info
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_GetPinConfig(CP213x_DEVICE hDevice, BYTE* pinConfig)
{
    USB_SPI_STATUS status;

    SETUP_PACKET SetupPacket;
    DWORD numBytesTransferred = 0;

    if (pinConfig == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = SIZE_PIN_CONFIG;
    SetupPacket.Request     = GET_PIN_CONFIG;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;

    status = CP213x_ControlTransfer(hDevice, SetupPacket, pinConfig, &numBytesTransferred);

    if(!status && (numBytesTransferred != SIZE_PIN_CONFIG))
    {
        DPRINT("\r\nERROR: Size of PinConfig array was not as expected.\r\n");
        status = USB_SPI_ERRCODE_INVALID_TRANSFER_SIZE;
    }
    return status;
}


/*!
 * \brief Set the USB device Pin Configuration info
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] pinConfig Buffer containing the USB device Pin Configuration info
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_SetPinConfig(CP213x_DEVICE hDevice, BYTE pinConfig[SIZE_PIN_CONFIG])
{
    USB_SPI_STATUS status;
    BYTE data[SIZE_PIN_CONFIG];
    SETUP_PACKET SetupPacket;
    DWORD numBytesTransferred = 0;

    if (pinConfig == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    memcpy(&data[0], &pinConfig[0], SIZE_PIN_CONFIG);

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = SIZE_PIN_CONFIG;
    SetupPacket.Request     = SET_PIN_CONFIG;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = MEMKEYS;  // Pass Flash/OTP keys here
    status = CP213x_ControlTransfer(hDevice, SetupPacket, data, &numBytesTransferred);
    return status;
}


/*!
 * \brief Read device EPROM configuration
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[out] pRbuffer Pointer to byte array to be filled
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_ReadProm(CP213x_DEVICE hDevice, BYTE pRbuffer[])
{
    USB_SPI_STATUS status;
    BYTE report[EP_BUFSIZE];
    SETUP_PACKET SetupPacket;

    DWORD cbRead = 0;

    if (pRbuffer == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    memset(report, 0xCC, sizeof(report));
    SetupPacket.Length      = EP_BUFSIZE;
    SetupPacket.Request     = GET_PROM_CONFIG;
    SetupPacket.RequestType = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;

    for(int blockNum = 0; blockNum < (USER_CONFIG_AREA_SIZE/64); blockNum++)
    {
        SetupPacket.Index = blockNum;
        status = CP213x_ControlTransfer(hDevice, SetupPacket, report, &cbRead);
        if (status)
        {
            DPRINT("\r\nERROR: ReadPROM -> ControlTransfer returned %02X\r\n", status);
            break;
        }
        else
        {
            memcpy(pRbuffer + (blockNum * 64), report, 64);
        }
    }
    return status;
}


/*!
 * \brief Write device EPROM configuration
 *
 * \details
 *
 * \param[in] hDevice USB Interface Handle
 * \param[in] pWbuffer Pointer to byte array containing data to write to EPROM
 * \return USB_SPI_ERRCODE_SUCCESS
 * \return USB_SPI_ERRCODE_INVALID_HANDLE
 * \return USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR
 */
USB_SPI_STATUS
CP213x_WriteProm(CP213x_DEVICE hDevice, BYTE pWbuffer[])
{
    USB_SPI_STATUS status;
    BYTE report[EP_BUFSIZE];
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    if (pWbuffer == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    SetupPacket.Request     = SET_PROM_CONFIG;
    SetupPacket.Length = 64;
    SetupPacket.RequestType = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = MEMKEYS;  // Pass Flash/OTP keys here

    for(int blockNum = 0; blockNum < (USER_CONFIG_AREA_SIZE/64); blockNum++)
    {
        SetupPacket.Index = blockNum;
        memcpy(report, pWbuffer + (blockNum * 64), 64);
        status = CP213x_ControlTransfer(hDevice, SetupPacket, report, &cbRead);
        if (status)
        {
            DPRINT("\r\nERROR: ReadPROM -> ControlTransfer returned %02X\r\n", status);
            break;
        }
    }
    return status;
}

//=================================================================================================================================
// Internal (non-API) Functions
//=================================================================================================================================

int ReadWritePipe(CP213x_DEVICE hDevice, BYTE rwType, DWORD size, BYTE buff[], DWORD timeoutMs, DWORD * pBytesTransferred)
{
    int status = USB_SPI_ERRCODE_SUCCESS;
    int result;
    DWORD bytesTransferred;
    CCP213xDevice* dev = (CCP213xDevice*)hDevice;
    
    // Check device object
    if (!DeviceList.Validate(dev)) {
        return USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
    }
    
    if ((buff == NULL) || (pBytesTransferred == NULL)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    if (PIPE_RW_TYPE_BULK_WRITE == rwType) {// WRITE
        result = dev->BulkTransfer(dev->GetOutEndpointAddress(), buff, size, &bytesTransferred, timeoutMs);	

        if (result == 0)    // WritePipe passed
        {
            DPRINT("\r\nlibusb_bulk_transfer returned immediately: Requested: %ld Actual: %ld.\r\n", size, bytesTransferred);
            status = USB_SPI_ERRCODE_SUCCESS;
        } else if (result == LIBUSB_ERROR_TIMEOUT) {
            if (bytesTransferred == 0) {
                status = USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT;
            } else {
                unsigned int sum = bytesTransferred;
                while (sum < size) {
                    result = dev->BulkTransfer(dev->GetOutEndpointAddress(), buff, size, &bytesTransferred, timeoutMs);
                    unsigned int oldSum = sum;
                    sum += bytesTransferred;
                    if (sum == oldSum) {
                        status = USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT;
                        break;
                    }
                }
            }
        } else {
            DPRINT("\r\nUNEXPECTED ERROR %ld from nlibusb_bulk_transfer. Requested = %d Actual = %d.\r\n", status, size, bytesTransferred);
            bytesTransferred = 0;
            
            status = USB_SPI_ERRCODE_PIPE_WRITE_FAIL;
        }     
    }// end WRITE
    else if (PIPE_RW_TYPE_BULK_READ == rwType) {// READ
        result = dev->BulkTransfer(dev->GetInEndpointAddress(), buff, size, &bytesTransferred, timeoutMs);	
        
        if (result == 0)// ReadPipe passed
        {
            DPRINT("\r\nlibusb_bulk_transfer returned immediately: Requested: %ld Actual: %ld.\r\n", size, bytesTransferred);
            status = USB_SPI_ERRCODE_SUCCESS;
        } else if (result == LIBUSB_ERROR_TIMEOUT) {
            if (bytesTransferred == 0) {
                status = USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT;
            } else {
                unsigned int sum = bytesTransferred;
                while (sum < size) {
                    result = dev->BulkTransfer(dev->GetInEndpointAddress(), buff, size, &bytesTransferred, timeoutMs);
                    unsigned int oldSum = sum;
                    sum += bytesTransferred;
                    if (sum == oldSum) {
                        status = USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT;
                        break;
                    }
                }
            }
        } else {
            bytesTransferred = 0;
            DPRINT("\r\nUNEXPECTED ERROR %ld from nlibusb_bulk_transfer. Requested = %d Actual = %d.\r\n", status, size, bytesTransferred);
            status = USB_SPI_ERRCODE_PIPE_READ_FAIL;
        }   
    }// end if (PIPE_RW_TYPE_BULK_IN_READ
    else
    {
        bytesTransferred = 0;
        DPRINT("\r\nERROR: Invalid rwType passed to ReadWritePipe().\r\n");
        status = USB_SPI_ERRCODE_INVALID_ENUM_VALUE;
    }

    *pBytesTransferred = bytesTransferred;  // Update parameter for calling function
    return status;
}// end ReadWritePipe

int SetCustomLongString(HANDLE hWinUSBDevice, BYTE rID_1, BYTE rID_2, LPCSTR ascii, BYTE strlen)
{
    int status;

    // Allocate a blank full-size unicode string buffer
    BYTE unicode[LONG_STRING_LEN_1 + LONG_STRING_LEN_2];
    memset(unicode, 0x00, sizeof(unicode));

    // Convert ASCII to Unicode
    for (int i = 0; i < strlen; i++)
    {
        unicode[i*2] = ascii[i];
    }
    // Break the string into two reports
    BYTE report1[64];
    BYTE report2[64];

    report1[0] = (strlen * 2) + 2;                                          // String Desc Length
    report1[1] = 0x03;                                                      // String Desc ID 

    memcpy(&report1[2], &unicode[0], LONG_STRING_LEN_1);                    // String Part 1
    
    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 64;
    SetupPacket.Request     = rID_1;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_HOST_TO_DEVICE | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = MEMKEYS;  // Pass Flash/OTP keys here
    status = CP213x_ControlTransfer(hWinUSBDevice, SetupPacket, report1, &cbRead);

    if( status == USB_SPI_ERRCODE_SUCCESS )
    {
        SetupPacket.Request = rID_2;
        memcpy(&report2[0], &unicode[LONG_STRING_LEN_1], LONG_STRING_LEN_2);    // String Part 2
        status = CP213x_ControlTransfer(hWinUSBDevice, SetupPacket, report2, &cbRead);
    }
    return status;
}


int GetCustomLongString(HANDLE hWinUSBDevice, BYTE rID_1, BYTE rID_2, LPSTR ascii, BYTE* strlen)
{
    int status;

    BYTE report1[64];
    memset(report1, 0x00, sizeof(report1));
    BYTE report2[64];
    memset(report2, 0x00, sizeof(report2));

    if (strlen == NULL) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }
    
    *strlen = 0;

    SETUP_PACKET SetupPacket;
    DWORD cbRead = 0;

    ZeroMemory(&SetupPacket, sizeof(SETUP_PACKET));
    SetupPacket.Length      = 64;
    SetupPacket.Request     = rID_1;
    SetupPacket.Index       = 0;
    SetupPacket.RequestType = REQTYPE_DIR_DEVICE_TO_HOST | REQTYPE_TYPE_VENDOR;
    SetupPacket.Value       = 0;
    status = CP213x_ControlTransfer(hWinUSBDevice, SetupPacket, report1, &cbRead);

    if( status == USB_SPI_ERRCODE_SUCCESS )
    {
        SetupPacket.Request   = rID_2;
        status = CP213x_ControlTransfer(hWinUSBDevice, SetupPacket, report2, &cbRead);
        if( status == USB_SPI_ERRCODE_SUCCESS )
        {
            // Get string length from first report
            // (size does not include the 0x03 string descriptor constant)
            *strlen = (report1[0] - 1) / 2;

            // Allocate a blank full-size unicode string buffer
            BYTE unicode[LONG_STRING_LEN_1 + LONG_STRING_LEN_2];
            memset(unicode, 0x00, sizeof(unicode));

            // Retrieve Unicode string from the device
            memcpy(&((BYTE*)unicode)[0], &report1[2], LONG_STRING_LEN_1);
            memcpy(&((BYTE*)unicode)[LONG_STRING_LEN_1], &report2[0], LONG_STRING_LEN_2);

            // Convert Unicode to ASCII
            for (int i = 0; i < ((LONG_STRING_LEN_1 + LONG_STRING_LEN_2)/2); i++)
            {
                ascii[i] = unicode[i * 2];
            }
        }
    }
    return status;
}
