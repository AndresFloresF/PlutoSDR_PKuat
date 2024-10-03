#ifndef SLAB_USB_SPI_PRIVATE_H
#define SLAB_USB_SPI_PRIVATE_H

#include "Types.h"
#include "SLAB_USB_SPI.h"


// Library version definitions
#define LIBVERSION_MAJOR        1
#define LIBVERSION_MINOR        1
#ifdef _DEBUG
#define LIBVERSION_ISRELEASE    FALSE
#else
#define LIBVERSION_ISRELEASE    TRUE
#endif


#define PIPE_RW_TYPE_BULK_WRITE  1
#define PIPE_RW_TYPE_BULK_READ   2

#define LANG_ID                     0x409   // English (United States)

// Used as an argument to WinUsb_GetOverlappedResult
#define bWAIT_TRUE  TRUE
#define bWAIT_FALSE FALSE

#define FIFO_FULL_THRESHOLD_SYNC          192
#define FIFO_FULL_THRESHOLD_ASYNC         128

#define MEMKEYS                           (0xA5F1)    // Flash/OTP memory keys
#define USER_CONFIG_AREA_SIZE             512

#define SIZE_USB_CONFIG             9

#define MAX_BULK_BUFFER_SIZE 1048576 //1MB max bulk transfer size

//==============================================================================
// Type and structure definitions
//==============================================================================
typedef struct
{
    BYTE offset;
    BYTE mask;
} tGpioMap;

/// This struct has the same format as _USB_SETUP_PACKET, defined in winusb.h.
/// Using this typedef enables portable application code, i.e. not dependent on winusb.h.
typedef struct SETUP_PACKET
{
    BYTE    RequestType;
    BYTE    Request;
    WORD    Value;
    WORD    Index;
    WORD    Length;
} SETUP_PACKET, *PSETUP_PACKET;

//==============================================================================
// Setup Packet format
//==============================================================================

// Table 9-2.  Format of Setup Data
// 
// Offset     Field       Size  Value         Description
// ------|---------------|----|--------|------------------------------
// 0     | bmRequestType | 1  | Bitmap | D7: Data transfer direction
//       |               |    |        |    0 = Host-to-device
//       |               |    |        |    1 = Device-to-host
//       |               |    |        | D6...5: Type
//       |               |    |        |    0 = Standard
//       |               |    |        |    1 = Class
//       |               |    |        |    2 = Vendor
//       |               |    |        |    3 = Reserved
//       |               |    |        | D4...0: Recipient
//       |               |    |        |    0 = Device
//       |               |    |        |    1 = Interface
//       |               |    |        |    2 = Endpoint
//       |               |    |        |    3 = Other
//       |               |    |        |    4...31 = Reserved
//       |               |    |        |
// 1     |  bRequest     | 1  |  Value | Specific request (refer to Table 9-3)
//       |               |    |        |
// 2     |  wValue       | 2  |  Value | Varies according to request
//       |               |    |        |
// 4     |  wIndex       | 2  | Index  | Varies according to request; typically
//       |               |    |   or   | used to pass an index or offset.
//       |               |    | Offset |
//       |               |    |        |
// 6     |  wLength      | 2  |  Count | Number of bytes to transfer in data stage.

// bmRequestType (USB Table 9-2)
#define REQTYPE_DIR_HOST_TO_DEVICE  0x00    //  0xxx xxxx
#define REQTYPE_DIR_DEVICE_TO_HOST  0x80    //  1xxx xxxx

#define REQTYPE_TYPE_STANDARD       0x00    //  x00x xxxx
#define REQTYPE_TYPE_CLASS          0x20    //  x01x xxxx
#define REQTYPE_TYPE_VENDOR         0x40    //  x10x xxxx

#define REQTYPE_RECP_DEVICE         0x00    //  xxx0 0000
#define REQTYPE_RECP_INTERFACE      0x01    //  xxx0 0001
#define REQTYPE_RECP_ENDPOINT       0x02    //  xxx0 0010
#define REQTYPE_RECP_OTHER          0x03    //  xxx0 0011

// bRequest (USB Table 9-4, Standard Request Codes)
#define REQ_GET_STATUS              0
#define REQ_CLEAR_FEATURE           1
#define REQ_SET_FEATURE             3
#define REQ_SET_ADDRESS             5
#define REQ_GET_DESCRIPTOR          6
#define REQ_SET_DESCRIPTOR          7
#define REQ_GET_CONFIGURATION       8
#define REQ_SET_CONFIGURATION       9
#define REQ_GET_INTERFACE           10
#define REQ_SET_INTERFACE           11
#define REQ_SYNCH_FRAME             12

// wValue: Descriptor Type (USB Table 9-5)
#define DESC_TYPE_DEVICE            1
#define DESC_TYPE_CONFIGURATION     2
#define DESC_TYPE_STRING            3
#define DESC_TYPE_INTERFACE         4
#define DESC_TYPE_ENDPOINT          5
#define DESC_TYPE_DEV_QUAL          6
#define DESC_TYPE_OTHER_SPD_CONFIG  7
#define DESC_TYPE_INTERFACE_POWER   8

// Standard Feature Selectors (USB Table 9-6)
#define FEATURE_DEVICE_REMOTE_WAKEUP    1   // Recipient must be Device
#define FEATURE_ENDPOINT_HALT           0   // Recipient must be Endpoint
#define FEATURE_TEST_MODE               2   // Recipient must be Device

//
// Vendor-specific bRequest definitions
//
#define RESET_DEVICE            0x10
#define GET_RO_VERSION          0x11

#define GET_GPIO_VALUES         0x20
#define SET_GPIO_VALUES         0x21
#define GET_GPIO_MODE_AND_LEVEL 0x22
#define SET_GPIO_MODE_AND_LEVEL 0x23
#define GET_CHIP_SELECT         0x24
#define SET_CHIP_SELECT         0x25

#define GET_SPI_CONTROL_BYTES   0x30
#define SET_SPI_CONTROL_BYTE    0x31
#define GET_SPI_DELAY           0x32
#define SET_SPI_DELAY           0x33
#define GET_FULL_THRESHOLD      0x34
#define SET_FULL_THRESHOLD      0x35
#define GET_RTR_STOP            0x36
#define SET_RTR_STOP            0x37
#define GET_MULTI_MASTER_STATE  0x38
//#define SET_MULTI_MASTER_STATE  0x39  // Not supported
//#define GET_MULTI_MASTER_CTL    0x40  // Not supported
#define SET_MULTIMASTER_CONTROL 0x41
#define GET_MULTIMASTER_CONFIG  0x42
#define SET_MULTIMASTER_CONFIG  0x43
#define GET_EVENT_COUNTER       0x44
#define SET_EVENT_COUNTER       0x45
#define GET_CLOCK_DIVIDER       0x46
#define SET_CLOCK_DIVIDER       0x47

#define GET_USB_CONFIG          0x60    // All customizable data that's not strings
#define SET_USB_CONFIG          0x61    // All customizable data that's not strings
#define GET_MANU1               0x62    // Get Set Manufacturing String 1
#define SET_MANU1               0x63    // Get Set Manufacturing String 1
#define GET_MANU2               0x64    // Get Set Manufacturing String 2
#define SET_MANU2               0x65    // Get Set Manufacturing String 2
#define GET_PROD1               0x66    // Get Set Product String 1
#define SET_PROD1               0x67    // Get Set Product String 1
#define GET_PROD2               0x68    // Get Set Product String 2
#define SET_PROD2               0x69    // Get Set Product String 2
#define GET_SERIAL              0x6A    // Get Set Serial String
#define SET_SERIAL              0x6B    // Get Set Serial String
#define GET_PIN_CONFIG          0x6C    // GPIO configuration
#define SET_PIN_CONFIG          0x6D    // GPIO configuration
#define GET_LOCK_BYTE           0x6E    // Get Set Lock Byte
#define SET_LOCK_BYTE           0x6F    // Get Set Lock Byte

#define GET_PROM_CONFIG         0x70    // Get EPROM all configuration
#define SET_PROM_CONFIG         0x71    // Set EPROM all configuration

//==============================================================================
// Undocumented API Functions
//==============================================================================
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

USB_SPI_STATUS
DPRINT(const char *format, ...);

USB_SPI_STATUS
CP213x_ControlTransfer      ( CP213x_DEVICE hDevice,
                                SETUP_PACKET SetupPacket,
                                BYTE* Buffer,
                                DWORD* LengthTransferred );

USB_SPI_STATUS
CP213x_GetMultiMasterState      ( CP213x_DEVICE hDevice, BYTE* multiMasterState );

USB_SPI_STATUS
CP213x_SetMultiMasterControl    ( CP213x_DEVICE hDevice, BYTE multiMasterControl );

USB_SPI_STATUS
CP213x_SetMultiMasterConfig     ( CP213x_DEVICE hDevice, BYTE multiMasterConfig );

//==============================================================================
// Internal (non-API) Functions
//==============================================================================

BOOL ReadWritePipe (CP213x_DEVICE phDeviceHandle,
                    BYTE rwType,
                    DWORD size,
                    BYTE * buff,
                    DWORD timeoutMs,
                    DWORD *bytesTransferred);

int SetCustomLongString(CP213x_DEVICE phDeviceHandle, BYTE rID_1, BYTE rID_2, LPCSTR ascii, BYTE strlen);
int GetCustomLongString(CP213x_DEVICE phDeviceHandle, BYTE rID_1, BYTE rID_2, LPSTR ascii, BYTE* strlen);

int DisplayPipeParameters(HANDLE phDeviceHandle, BYTE pipeID);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLAB_USB_SPI_PRIVATE_H
