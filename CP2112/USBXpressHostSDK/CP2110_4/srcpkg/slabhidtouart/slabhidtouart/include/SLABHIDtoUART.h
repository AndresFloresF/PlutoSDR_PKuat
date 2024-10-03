/////////////////////////////////////////////////////////////////////////////
// SLABHIDtoUART.h
// For SLABHIDtoUART.dll
// and Silicon Labs CP2110/CP2114 HID to UART
/////////////////////////////////////////////////////////////////////////////
#ifndef HOST_LIB_SLABHIDUART_INCLUDE_SLABHIDTOUART_H_INCLUDED_XF4QYQM3BK
#define HOST_LIB_SLABHIDUART_INCLUDE_SLABHIDTOUART_H_INCLUDED_XF4QYQM3BK

/// @addtogroup slabhiduart CP211X Serial over HID Library
///
/// The Silicon Labs HID-to-UART interface library provides a simple
/// API to configure and operate CP2110 and CP2114 devices.
///
/// The library provides interface abstraction so that users can develop their application
/// without writing any USB HID code. Silicon Labs provides both static and dynamic libraries implementing
/// the CP2110 and CP2114 Interface Specification for Windows, Mac OS X, and Linux.
/// Similarly, various include files are provided to import library functions into C#.NET
/// and Visual Basic.NET.
///
/// The APIs for SLABHIDtoUART interface library are
/// described below.
///
/// This document supercedes, obsoletes and takes precedences over AN433 https://www.silabs.com/documents/public/application-notes/AN433-CP2110-4-HID-to-UART-API-Spec.pdf.
///
/// The SLABHIDtoUART Host API is provided in the form of a Windows Dynamic Link Library (DLL), SLABHIDtoUART.DLL and as a Windows
/// static link library (SLABHIDtoUART.static.LIB). The host interface
/// library communicates with the bridge controller device via the provided device driver and the operating system's USB stack.
///
/// Custom applications can use the SLABHIDtoUART API implemented in SLABHIDtoUART.DLL. To use functions implemented in SLABHIDtoUART.DLL link SLABHIDtoUART.LIB
/// include SLABHIDtoUART.h into any source code files that call any functions implemented in SLABHIDtoUART.DLL and include the PATH to the location of SLABHIDtoUART.DLL
/// in your custom application Visual Studio .VCXPROJ Profile File's Properties->Linker->General->Additional Library Directories property.
///
/// Typically, the user initiates communication with the target device/s by making a call to @ref HidUart_GetNumDevices(). This call
/// returns the number of CP2110 and CP2114 target devices. This number is used as a range when calling @ref HidUart_GetProductString() to build a list
/// of devices connected to the host machine.
/// A handle to the device must first be opened by a call to @ref HidUart_Open() using an index determined from the call to @ref HidUart_GetNumDevices().
/// The handle will be used for all subsequent accesses. When I/O operations are complete, the device handle is closed by a call to
/// @ref HidUart_Close().
///
/// @{

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include	"silabs_defs.h"
#include	"silabs_sal.h"
#include "Types.h"

/////////////////////////////////////////////////////////////////////////////
// Tool-chain-dependent hacks
/////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the HID_TO_UART_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// HID_TO_UART_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#if defined(HID_TO_UART_EXPORTS)
#define HID_TO_UART_API
#else
#if defined(SILABS_STATIC_LINK)
#define HID_TO_UART_API extern
#pragma comment(lib, "SLABHIDtoUART.static.lib")
#else // defined(SILABS_STATIC_LINK)
#define HID_TO_UART_API __declspec(dllimport)
#pragma comment(lib, "SLABHIDtoUART.lib")
#endif // defined(SILABS_STATIC_LINK)
#endif // defined(HID_TO_UART_EXPORTS)
#else // !_WIN32
#define HID_TO_UART_API ///< HID to UART API
#define WINAPI ///< Win API
#endif // !_WIN32



/////////////////////////////////////////////////////////////////////////////
// Return Code Definitions
/////////////////////////////////////////////////////////////////////////////

#if !defined(USE_LEGACY_HID_UART_STATUS)
///@enum _HID_UART_STATUS
///@brief API Status Return Codes. Alias of this enum is HID_UART_STATUS
typedef enum _HID_UART_STATUS {
	HID_UART_SUCCESS					= 0x00	///< Success
	, HID_UART_DEVICE_NOT_FOUND			= 0x01					///< the specified device was not found
	, HID_UART_INVALID_HANDLE			= 0x02					///< an handle parameter was not valid
	, HID_UART_INVALID_DEVICE_OBJECT	= 0x03					///< the device object pointer does not match the address of a valid HID - to - UART device
	, HID_UART_INVALID_PARAMETER		= 0x04					///< a parameter was not valid
	, HID_UART_INVALID_REQUEST_LENGTH	= 0x05					///< the specified number of bytes to read or write is invalid.

	, HID_UART_READ_ERROR				= 0x10					///< the read was not successful and did not time out.
	, HID_UART_WRITE_ERROR				= 0x11					///< the write was not successful.
	, HID_UART_READ_TIMED_OUT			= 0x12					///< a read failed to return the number of bytes requested before the read timeout elapsed.
	, HID_UART_WRITE_TIMED_OUT			= 0x13					///< a write failed to complete sending the number of bytes requested before the write timeout elapsed.
	, HID_UART_DEVICE_IO_FAILED			= 0x14					///< host was unable to get or set a feature report.
	, HID_UART_DEVICE_ACCESS_ERROR		= 0x15					///< the device or device property could not be accessed.Either the device is not opened, already opened when trying to open, or an error occurred when trying to get HID information.
	, HID_UART_DEVICE_NOT_SUPPORTED		= 0x16					///< the current device does not support the corresponding action
	, HID_UART_INVALID_CONFIG_VERSION	= 0x17					///< tbd

	, HID_UART_DEVICE_ALREADY_OPENED	= 0x22					///< the specified device is already (exclusively) opened

	, HID_UART_UNKNOWN_ERROR			= SILABS_STATUS_UNKNOWN_ERROR					///< This value should never be returned.
} HID_UART_STATUS, *PHID_UART_STATUS;
///@typedef PHID_UART_STATUS
///@brief API Status Return Codes pointer
#else // defined(USE_LEGACY_HID_UART_STATUS)
/// @typedef HID_UART_STATUS
/// @brief HID_UART_STATUS
typedef int HID_UART_STATUS;
/// @defgroup HID_UART_STATUS HID uart return statuses
/// @{
// API Status Return Codes
#define HID_UART_SUCCESS									0x00	///< HID to UART success
#define	HID_UART_DEVICE_NOT_FOUND					0x01 ///< HID to UART device not found
#define HID_UART_INVALID_HANDLE						0x02 ///< HID to UART invalid handle
#define	HID_UART_INVALID_DEVICE_OBJECT				0x03 ///< HID to UART invalid device object
#define	HID_UART_INVALID_PARAMETER					0x04 ///< HID to UART invalid parameter
#define	HID_UART_INVALID_REQUEST_LENGTH				0x05 ///< HID to UART invalid request length

#define	HID_UART_READ_ERROR							0x10 ///< HID to UART read error
#define	HID_UART_WRITE_ERROR						0x11 ///< HID to UART write error
#define	HID_UART_READ_TIMED_OUT						0x12 ///< HID to UART read time out
#define	HID_UART_WRITE_TIMED_OUT					0x13 ///< HID to UART write time out
#define	HID_UART_DEVICE_IO_FAILED					0x14 ///< HID to UART devic io fail
#define HID_UART_DEVICE_ACCESS_ERROR				0x15 ///< HID to UART device access error
#define HID_UART_DEVICE_NOT_SUPPORTED				0x16 ///< HID to UART device not supported
#define HID_UART_INVALID_CONFIG_VERSION             0x17 ///< HID to UART invalid config version

#define HID_UART_UNKNOWN_ERROR						SILABS_STATUS_UNKNOWN_ERROR ///< HID to UART success
/// @}
#endif // defined(USE_LEGACY_HID_UART_STATUS)

/////////////////////////////////////////////////////////////////////////////
// String Definitions
/////////////////////////////////////////////////////////////////////////////

/// Product String option Types
/// @defgroup GetStringOptions Product String option Types
/// @{
#define HID_UART_GET_VID_STR						0x01	///< Vendor ID string
#define HID_UART_GET_PID_STR						0x02	///< Product ID string
#define HID_UART_GET_PATH_STR						0x03	///< Full Path string a la "Device Path" "A NULL-terminated string that contains the device interface path. This path can be passed to Win32 functions such as CreateFile(). "
#define HID_UART_GET_SERIAL_STR						0x04	///< Serial Number string
#define HID_UART_GET_MANUFACTURER_STR				0x05	///< Manufacturer string
#define HID_UART_GET_PRODUCT_STR					0x06	///< Product string
/// @}

// String Lengths
#define HID_UART_DEVICE_STRLEN						260		///< CP210x Maximum Device-side string length
///@typedef HID_UART_DEVICE_STR
///@brief HID_UART_DEVICE_STR
typedef char HID_UART_DEVICE_STR[HID_UART_DEVICE_STRLEN];

/////////////////////////////////////////////////////////////////////////////
// UART Definitions
/////////////////////////////////////////////////////////////////////////////

/// @defgroup GetUartStatus HidUart_GetUartStatus bitsmasks
/// @brief @ref HidUart_GetUartStatus() bitsmasks
/// @{
/// Error Status
#define HID_UART_PARITY_ERROR						0x01	///< Parity error
#define HID_UART_OVERRUN_ERROR						0x02	///< Overrun error

/// Line Break Status
#define HID_UART_LINE_BREAK_INACTIVE				0x00	///< Line break inactive
#define HID_UART_LINE_BREAK_ACTIVE					0x01	///< Line break active
/// @}

/// Data Bits 
/// @defgroup HID_UART_NUMBER_DATA_BITS Number of Data Bits for UART communication
/// @brief @ref HidUart_SetUartConfig() Number of Data Bits for UART communication
/// @{
#define HID_UART_FIVE_DATA_BITS						0x00	///< 5 data bits
#define HID_UART_SIX_DATA_BITS						0x01	///< 6 data bits
#define HID_UART_SEVEN_DATA_BITS					0x02	///< 7 data bits
#define HID_UART_EIGHT_DATA_BITS					0x03	///< 8 data bits
/// @}

/// Parity
/// @defgroup HID_UART_PARITY The parity for UART communication
/// @brief @ref HidUart_SetUartConfig() The parity for UART communication
/// @{
#define HID_UART_NO_PARITY							0x00	///< No parity
#define HID_UART_ODD_PARITY							0x01	///< Odd parity (sum of data bits is odd)
#define HID_UART_EVEN_PARITY						0x02	///< Even parity (sum of data bits is even)
#define HID_UART_MARK_PARITY						0x03	///< Mark parity (always 1)
#define HID_UART_SPACE_PARITY						0x04	///< Space parity (always 0)
/// @}

/// Stop Bits
/// Short = 1 stop bit
/// Long  = 1.5 stop bits (5 data bits)
///       = 2 stop bits (6-8 data bits)
/// @defgroup HID_UART_STOP_BITS The number of stop bits for UART communication
/// @brief @ref HidUart_SetUartConfig() The number of stop bits for UART communication
/// @{
#define HID_UART_SHORT_STOP_BIT						0x00	///< 1 stop bit
#define HID_UART_LONG_STOP_BIT						0x01	///< 5 data bits: 1.5 stop bits; 6 - 8 data bits : 2 stop bits
/// @}

/// Flow Control
/// @defgroup HID_UART_FLOW_CONTROL The type of flow control for UART communication
/// @brief @ref HidUart_SetUartConfig() The type of flow control for UART communication
/// @{
#define HID_UART_NO_FLOW_CONTROL					0x00	///< No flow control
#define HID_UART_RTS_CTS_FLOW_CONTROL				0x01	///< RTS/CTS hardware flow control
/// @}

// Read/Write Limits
/// @defgroup HID_UART_READ_WRITE_SIZES Read/Write Limits
/// @{
#define HID_UART_MIN_READ_SIZE						1		///< Minimum read request size see @ref HidUart_ReadLatch().
#define HID_UART_MAX_READ_SIZE						32768	///< Maximum read request size see @ref HidUart_ReadLatch().
#define HID_UART_MIN_WRITE_SIZE						1		///< Minimum write request size see @ref HidUart_WriteLatch().
#define HID_UART_MAX_WRITE_SIZE						4096	///< Maximum write request size see @ref HidUart_WriteLatch().
/// @}

/////////////////////////////////////////////////////////////////////////////
// User Customization Definitions
/////////////////////////////////////////////////////////////////////////////

// User-Customizable Field Lock Bitmasks
/// @defgroup HID_UART_LOCK_BITMASKS User-Customizable Field Lock Bitmasks
/// @{
#define HID_UART_LOCK_PRODUCT_STR_1					0x0001	///< Product String
#define HID_UART_LOCK_PRODUCT_STR_2					0x0002	///< Product String
#define HID_UART_LOCK_SERIAL_STR					0x0004	///< Serial Number String
#define HID_UART_LOCK_PIN_CONFIG					0x0008	///< Pin Config
#define HID_UART_LOCK_VID							0x0100	///< VID
#define HID_UART_LOCK_PID							0x0200	///< PID
#define HID_UART_LOCK_POWER							0x0400	///< Power
#define HID_UART_LOCK_POWER_MODE					0x0800	///< Power Mode
#define HID_UART_LOCK_RELEASE_VERSION				0x1000	///< Release Version
#define HID_UART_LOCK_FLUSH_BUFFERS					0x2000	///< Flush Buffers
#define HID_UART_LOCK_MFG_STR_1						0x4000	///< Manufacturing String
#define HID_UART_LOCK_MFG_STR_2						0x8000	///< Manufacturing String
/// @}

// Field Lock Bit Values
/// @defgroup HID_UART_LOCK_VALUES Field Lock Bit Values
/// @{
#define HID_UART_LOCK_UNLOCKED						1	///< Unlocked
#define HID_UART_LOCK_LOCKED						0	///< Locked
/// @}

/// Power Max Value (500 mA)
#define HID_UART_BUS_POWER_MAX						0xFA ///< Max HID to UART bus power

/// Power Modes
/// @defgroup HID_UART_POWER_MODES Power Modes
/// @{
#define HID_UART_BUS_POWER							0x00	///< Bus powered
#define HID_UART_SELF_POWER_VREG_DIS				0x01	///< Self-powered and Voltage regulator is DISabled
#define HID_UART_SELF_POWER_VREG_EN					0x02	///< Self-powered and Voltage regulator is ENabled
/// @}

/// Flush Buffers Bitmasks
/// @defgroup HID_UART_FLUSH_BITMASKS Flush Buffers Bitmasks
/// @{
#define HID_UART_FLUSH_TX_OPEN						0x01	///< Flush on Open, Transmit
#define HID_UART_FLUSH_TX_CLOSE						0x02	///< Flush on Close, Transmit
#define HID_UART_FLUSH_RX_OPEN						0x04	///< Flush on Open, Receive
#define HID_UART_FLUSH_RX_CLOSE						0x08	///< Flush on Close, Receive
/// @}

/// USB Config Bitmasks
/// @defgroup HID_UART_SET_BITMASKS USB Config Bitmasks
/// @brief USB Config Bitmasks. See @ref HidUart_SetUsbConfig()
/// @{
#define HID_UART_SET_VID							0x01	///< VID
#define HID_UART_SET_PID							0x02	///< PID
#define HID_UART_SET_POWER							0x04	///< Power
#define HID_UART_SET_POWER_MODE						0x08	///< Power Mode
#define HID_UART_SET_RELEASE_VERSION				0x10	///< Release Version
#define HID_UART_SET_FLUSH_BUFFERS					0x20	///< Flush Buffers

/// USB Config Bit Values
#define HID_UART_SET_IGNORE							0	///< Field will be unchanged
#define HID_UART_SET_PROGRAM						1	///< Field will be reprogrammed
/// @}

/// Device-side string lengths
/// @defgroup HID_DeviceSideStringLengths Device-side string lengths
/// @{
#define HID_UART_MFG_STRLEN							62	///< device Max Manufacturing string length
#define HID_UART_PRODUCT_STRLEN						62	///< device Max Product string length
#define HID_UART_SERIAL_STRLEN						30	///< device Max Serial Number string length
/// @}

///@typedef HID_UART_MFG_STR
///@brief HID to UART manufacturing string
typedef char HID_UART_MFG_STR[HID_UART_MFG_STRLEN];

///@typedef HID_UART_PRODUCT_STR
///@brief HID to UART product string
typedef char HID_UART_PRODUCT_STR[HID_UART_PRODUCT_STRLEN];

///@typedef HID_UART_SERIAL_STR
///@brief HID to UART serial string
typedef char HID_UART_SERIAL_STR[HID_UART_SERIAL_STRLEN];

/////////////////////////////////////////////////////////////////////////////
// Pin Definitions
/////////////////////////////////////////////////////////////////////////////

/// Pin Config Modes
/// @defgroup PinConfigModes Pin Config Modes
/// @{
#define HID_UART_GPIO_MODE_INPUT					0x00	///< GPIO Input
#define HID_UART_GPIO_MODE_OUTPUT_OD				0x01	///< GPIO Output-Open Drain
#define HID_UART_GPIO_MODE_OUTPUT_PP				0x02	///< GPIO Output-Push Pull
#define HID_UART_GPIO_MODE_FUNCTION1				0x03	///< Pin specific function and mode
#define HID_UART_GPIO_MODE_FUNCTION2				0x04	///< tbd
/// @}

/// Suspend Value Bit Values
/// @defgroup SuspendValueBitValues Suspend Value Bit Values
/// @{
#define HID_UART_VALUE_SUSPEND_LO					0	///< Latch = 0 in suspend
#define HID_UART_VALUE_SUSPEND_HI					1	///< Latch = 1 in suspend
/// @}

/// Suspend Mode Bit Values
/// @defgroup SuspendModeBitValues Suspend Mode Bit Values
/// @{
#define HID_UART_MODE_SUSPEND_OD					0	///< Open Drain in suspend
#define HID_UART_MODE_SUSPEND_PP					1	///< Push Pull in suspend
/// @}

/// RS485 Active Levels
/// @defgroup RS485ActiveLevels RS485 Active Levels
/// @{
#define HID_UART_MODE_RS485_ACTIVE_LO				0x00	///< GPIO.2/RS485 pin is active low
#define HID_UART_MODE_RS485_ACTIVE_HI				0x01	///< GPIO.2/RS485 pin is active high
/// @}

/////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////
///@typedef HID_UART_DEVICE
///@brief HID to UART device
typedef void* HID_UART_DEVICE;
///@typedef U8
///@brief unsigned uint 8
typedef unsigned char U8;
///@typedef U16
///@brief unsigned uint 16
typedef unsigned short U16;

/////////////////////////////////////////////////////////////////////////////
// Exported Library Functions
/////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/// @brief Determines the number of devices connected to the system with matching Vendor ID and Product ID
///
/// @param[out] lpdwNumDevices a pointer to a uint32_t location to hold the returned device count
/// @param[in] vid is the 2-byte Vendor ID value, filter device results by this Vendor ID.
/// @param[in] pid is the 2-byte Product ID value, filter device results by this Product ID.
///
/// @note If both vid and pid are set to 0x0000, then HID devices will not be filtered by VID/PID.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_PARAMETER -- lpdwNumDevices is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetNumDevices(_Out_writes_bytes_(sizeof(DWORD)) _Pre_defensive_ LPDWORD lpdwNumDevices, _In_ _Pre_defensive_ const WORD vid, _In_ _Pre_defensive_ const WORD pid);

/// @brief This function returns a null-terminated vendor ID string, product ID string, serial number string, device path
/// string, manufacturer string, or product string for the device specified by an index passed in deviceNum.
/// 
/// @details The index for the first device is 0 and the last device is the value returned by @ref HidUart_GetNumDevices() - 1.
///
/// @param[in] deviceNum Index of the device for which the string is desired.
/// @param[in] vid is the 2-byte Vendor ID value, filter device results by this Vendor ID.
/// @param[in] pid is the 2-byte Product ID value, filter device results by this Product ID.
/// @param[out] deviceString - Variable of type @ref GetStringOptions which will contain a NULL terminated
/// ASCII device string on return. The string is 260 bytes on Windows and 512 bytes on macos and Linux.
/// @param[in] options - Determines if deviceString contains a vendor ID string, product ID string, serial number string,
/// device path string, manufacturer string, or product string. See @ref GetStringOptions
///
/// @note If both @p vid and @p pid are set to 0x0000, then HID devices will not be filtered by VID/PID.
///
/// @note BUG: This is an string-UNSAFE function, not defensive against a "too small" buffer provided as the @p deviceString parameter.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_DEVICE_NOT_FOUND -- deviceNum is an unexpected value
///	@retval	#HID_UART_DEVICE_ALREADY_OPENED -- the specified device is already (exclusively) opened
///	@retval	#HID_UART_DEVICE_ACCESS_ERROR -- tbd
///	@retval	#HID_UART_INVALID_PARAMETER -- deviceString and/or options is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetString(_In_ _Pre_defensive_ const DWORD deviceNum, _In_ _Pre_defensive_ const WORD vid, _In_ _Pre_defensive_ const WORD pid, _Out_ _Pre_defensive_ char* deviceString, _In_ _Pre_defensive_ const DWORD options);

/// @brief This function returns a null-terminated vendor ID string, product ID string, serial number string, device path
/// string, manufacturer string, or product string for the device specified by device.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] deviceString - Variable of type @ref GetStringOptions which will contain a NULL terminated
/// ASCII device string on return. The string is 260 bytes on Windows and 512 bytes on macos
/// and Linux.
/// @param[in] options - Determines if deviceString contains a vendor ID string, product ID string, serial number string,
/// device path string, manufacturer string, or product string. See @ref GetStringOptions
///
/// @note BUG: This is an string-UNSAFE function, not defensive against a "too small" buffer provided as the @p deviceString parameter.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_ACCESS_ERROR -- tbd
///	@retval	#HID_UART_INVALID_PARAMETER -- deviceString and/or options is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetOpenedString(_In_ _Pre_defensive_ const HID_UART_DEVICE device, char* deviceString, _In_ _Pre_defensive_ const DWORD options);

/// @brief This function returns a null-terminated USB string descriptor for the device specified by an index
/// passed in deviceNum. (Windows/Linux only)
///
/// @param[in] deviceNum Index of the device for which the string is desired.
/// @param[in] vid is the 2-byte Vendor ID value, filter device results by this Vendor ID.
/// @param[in] pid is the 2-byte Product ID value, filter device results by this Product ID.
/// @param[in] stringIndex - Specifies the device-specific index of the USB string descriptor to return.
/// @param[out] deviceString - Variable of type @ref GetStringOptions which will contain a NULL terminated
/// device descriptor string on return. The string is 260 bytes on Windows and 512 bytes on Linux.
///
/// @note If both vid and pid are set to 0x0000, then HID devices will not be filtered by VID/PID.
///
/// @note BUG: This is an string-UNSAFE function, not defensive against a "too small" buffer provided as the @p deviceString parameter.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_DEVICE_NOT_FOUND -- deviceNum is an unexpected value
///	@retval	#HID_UART_DEVICE_ACCESS_ERROR -- tbd
///	@retval	#HID_UART_INVALID_PARAMETER -- deviceString is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetIndexedString(_In_ _Pre_defensive_ const DWORD deviceNum, _In_ _Pre_defensive_ const WORD vid, _In_ _Pre_defensive_ const WORD pid, _In_ _Pre_defensive_ const DWORD stringIndex, char* deviceString);

/// @brief This function returns a null-terminated USB string descriptor for the device specified by device. (Windows/Linux only)
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] stringIndex - Specifies the device-specific index of the USB string descriptor to return.
/// @param[out] deviceString - Variable of type @ref GetStringOptions which will contain a NULL terminated
/// device descriptor string on return. The string is 260 bytes on Windows and 512 bytes on Linux.
///
/// @note BUG: This is an string-UNSAFE function, not defensive against a "too small" buffer provided as the @p deviceString parameter.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_ACCESS_ERROR -- tbd
///	@retval	#HID_UART_INVALID_PARAMETER -- deviceString is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetOpenedIndexedString(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _In_ _Pre_defensive_ const DWORD stringIndex, char* deviceString);

/// @brief This function returns the device vendor ID, product ID, and device release number for the device specified
/// by an index passed in deviceNum.
///
/// @param[in] deviceNum Index of the device for which the string is desired.
/// @param[in] vid is the 2-byte Vendor ID value, filter device results by this Vendor ID.
/// @param[in] pid is the 2-byte Product ID value, filter device results by this Product ID.
/// @param[out] deviceVid - returns the 2-byte Vendor ID value from the device.
/// @param[out] devicePid - returns the 2-byte Product ID value from the device.
/// @param[out] pDeviceReleaseNumber points at a 2-byte buffer into which the USB device bcdVersion, or device release number, value will be written
///
/// @note If both vid and pid are zero (0x0000), devices will NOT be filtered by vid/pid.
///
/// @note A successfully returned pDeviceReleaseNumber is in binary-coded decimal (BCD).
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_DEVICE_NOT_FOUND -- deviceNum is an unexpected value
///	@retval	#HID_UART_DEVICE_ALREADY_OPENED -- the specified device is already (exclusively) opened
///	@retval	#HID_UART_INVALID_PARAMETER -- deviceVid, devicePid and/or pDeviceReleaseNumber is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetAttributes(_In_ _Pre_defensive_ const DWORD deviceNum, _In_ _Pre_defensive_ const WORD vid, _In_ _Pre_defensive_ const WORD pid, _Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* deviceVid, _Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* devicePid, _Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* pDeviceReleaseNumber);

/// @brief This function returns the device vendor ID, product ID, and device release number for the opened device.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] pDeviceVid - returns the 2-byte Vendor ID value from the device.
/// @param[out] pDevicePid - returns the 2-byte Product ID value from the device.
/// @param[out] pDeviceReleaseNumber points at a 2-byte buffer into which the USB device bcdVersion, or device release number, value will be written
///
/// @note A successfully returned @p pDeviceReleaseNumber is in binary-coded decimal (BCD).
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- pDeviceVid, pDevicePid and/or pDeviceReleaseNumber is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetOpenedAttributes(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* pDeviceVid, _Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* pDevicePid, _Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* pDeviceReleaseNumber);

/// @brief Opens a device using a device number between 0 and @ref HidUart_GetNumDevices() - 1, enables the
/// UART, and returns a device object pointer which will be used for subsequent accesses.
///
/// @param[out] pdevice - Returns a pointer to a HID-to-UART device object for subsequent accesses to the device.
/// @param[in] deviceNum - Zero-based device index, between 0 and (@ref HidUart_GetNumDevices() - 1).
/// @param[in] vid is the 2-byte Vendor ID value, filter device results by this Vendor ID.
/// @param[in] pid is the 2-byte Product ID value, filter device results by this Product ID.
///
/// @note If both @p vid and @p pid are zero (0x0000), devices will NOT be filtered by @p vid/@p pid.
///
/// @note Be careful when opening a device. Any HID device may be opened by this library. However, if the
/// device is not actually a CP211x, use of this library will cause undesirable results. The best course of
/// action would be to designate a unique VID/PID for CP211x devices only. The application should then
/// filter devices using this VID/PID.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_DEVICE_NOT_FOUND -- deviceNum is an unexpected value
///	@retval	#HID_UART_DEVICE_ACCESS_ERROR -- Could not access device (i.e. already opened?)
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum only support CP2110 and CP2114 devices
///	@retval	#HID_UART_INVALID_PARAMETER -- pdevice is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_Open(_Out_ HID_UART_DEVICE* pdevice, _In_ _Pre_defensive_ const DWORD deviceNum, _In_ _Pre_defensive_ const WORD vid, _In_ _Pre_defensive_ const WORD pid);

/// @brief Closes an opened device using the device object pointer provided by @ref HidUart_Open().
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
///
/// @note The device parameter is invalid after calling @ref HidUart_Close(). Set device to NULL after calling @ref HidUart_Close().
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_HANDLE -- failed, the device specified by device does not have a valid handle
///	@retval	#HID_UART_DEVICE_ACCESS_ERROR -- tbd
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_Close(_In_ _Pre_defensive_ const HID_UART_DEVICE device);

/// @brief Returns the device opened status.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] pbIsOpened points to a buffer into which a Boolean flag where TRUE means the device object pointer is valid and the device has been opened using @ref HidUart_Open(), and FALSE means it has not will be written
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- pbIsOpened is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_IsOpened(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _Out_writes_bytes_(sizeof(BOOL)) _Pre_defensive_ BOOL* pbIsOpened);

/// @brief Enables or disables the UART.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] enable - Set to TRUE/non-zer0 to enable the UART, FALSE/0 to disable
///
/// @note Enabling or disabling the UART will flush the UART FIFOs if the flushBuffers parameter is enabled by
/// calling @ref HidUart_SetUsbConfig().
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetUartEnable(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _In_ _Pre_defensive_ const BOOL enable);

/// @brief Returns the UART enable status.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] penable points to a buffer into which a Boolean flag where TRUE means the UART is enabled, and FALSE means is is not will be written
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- penable is an unexpected value
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetUartEnable(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _Out_writes_bytes_(sizeof(BOOL)) _Pre_defensive_ BOOL* penable);

/// @brief Reads the available number of bytes into the supplied buffer and returns the number of bytes read
/// which can be less than the number of bytes requested. This function returns synchronously after
/// reading the requested number of bytes or after the timeout duration has elapsed.
/// Read and write timeouts can be set using @ref HidUart_SetTimeouts().
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] buffer - Address of a buffer to be filled with read data.
/// @param[in] numBytesToRead - Number of bytes to read from the device into the buffer (1-32768) value
/// must be less than or equal to the size of buffer.
/// @param[out] pNumBytesRead points to a buffer into which the number of bytes actually read into the buffer will be written.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- buffer and/or pNumBytesRead is an unexpected value
///	@retval	#HID_UART_INVALID_REQUEST_LENGTH -- numBytesToRead must be in the range (@ref HID_UART_MIN_READ_SIZE /1 - @ref HID_UART_MAX_READ_SIZE /32768)
///	@retval	#HID_UART_READ_ERROR -- tbd
///	@retval	#HID_UART_READ_TIMED_OUT -- the number of bytes read is less than the number of bytes requested and the read timeout has elapsed.
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_Read(_In_ _Pre_defensive_ const HID_UART_DEVICE device, BYTE* buffer, _In_ _Pre_defensive_ const DWORD numBytesToRead, _Out_writes_bytes_(sizeof(DWORD)) _Pre_defensive_ DWORD* pNumBytesRead);

/// @brief Write the specified number of bytes from the supplied buffer to the device. This function returns synchronously
/// after writing the requested number of bytes or after the timeout duration has elapsed.
/// Read and write timeouts can be set using @ref HidUart_SetTimeouts().
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] buffer - Address of a buffer to be sent to the device.
/// @param[in] numBytesToWrite of bytes to write to the device (@ref HID_UART_MIN_WRITE_SIZE / 1 - @ref HID_UART_MAX_WRITE_SIZE / 4096 bytes) less than or equal to the size of buffer.
/// @param[out] pNumBytesWritten points to a buffer into which the number of bytes actually written to the device will be written.
///
/// @note HidUart_Write() returns HID_UART_WRITE_TIMED_OUT if the number of bytes written is less than
/// the number of bytes requested. Data is broken down into HID interrupt reports between 1 - 63 bytes
/// in size and transmitted. Each report will be given a specific amount of time to complete. This report
/// timeout is determined by writeTimeout in @ref HidUart_SetTimeouts(). Each interrupt report is given the
/// max timeout to complete because a timeout at the interrupt report level is considered an unrecoverable
/// error (the IO is canceled in an unknown state). If the HID set interrupt report times out, HidUart_Write() 
/// returns @ref HID_UART_WRITE_ERROR. The HidUart_Write() timeout may take up to
/// twice as long as the timeout specified to allow each interrupt report to complete.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- buffer and/or pNumBytesWritten is an unexpected value
///	@retval	#HID_UART_INVALID_REQUEST_LENGTH -- numBytesToWrite must be in the range (@ref HID_UART_MIN_WRITE_SIZE / 1 - @ref HID_UART_MAX_WRITE_SIZE / 4096)
///	@retval	#HID_UART_WRITE_ERROR -- tbd
///	@retval	#HID_UART_WRITE_TIMED_OUT -- the number of bytes written is less than the number of bytes requested. See note above for more information.
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_Write(_In_ _Pre_defensive_ const HID_UART_DEVICE device, BYTE* buffer, _In_ _Pre_defensive_ const DWORD numBytesToWrite, _Out_writes_bytes_(sizeof(DWORD)) _Pre_defensive_ DWORD* pNumBytesWritten);

/// @brief Flushes the receive buffer in the device and the HID driver and/or the transmit buffer in the device
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] bFlushTransmit Set to TRUE to flush the device transmit buffer.
/// @param[in] bFlushReceive Set to TRUE to flush the device receive buffer and HID receive buffer.
///
/// @note Flush means purge, or discard. It does not mean drain, hurry-along or wait for completion.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_FlushBuffers(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _In_ _Pre_defensive_ const BOOL bFlushTransmit, _In_ _Pre_defensive_ const BOOL bFlushReceive);

/// @brief cancels any pending HID reads and writes. (Windows only)
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_CancelIo(_In_ _Pre_defensive_ const HID_UART_DEVICE device);

/// @brief Sets the read and write timeouts.
///
/// Timeouts are used for @ref HidUart_ReadLatch() and @ref HidUart_WriteLatch(). The
/// default value for timeouts is 1000 ms, but timeouts can be set to wait for any number of milliseconds
/// between 0 and 0xFFFFFFFF.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] ReadTimeoutInMilliseconds is the @ref HidUart_ReadLatch() operation timeout in milliseconds.
/// @param[in] WriteTimeoutInMilliseconds is the 2-byte Product ID value.
///
/// @note Read and write timeouts are maintained for each device but are not persistent across @ref HidUart_Open()/@ref HidUart_Close().
///
/// @note If read timeouts are set to a large value and no data is received, then the application may appear
/// unresponsive. It is recommended to set timeouts appropriately before using the device.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetTimeouts(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _In_ _Pre_defensive_ const DWORD ReadTimeoutInMilliseconds, _In_ _Pre_defensive_ const DWORD WriteTimeoutInMilliseconds);

/// @brief Returns the current read and write timeouts specified in milliseconds
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] pReadTimeoutInMilliseconds points to a buffer into which the @ref HidUart_ReadLatch() operation timeout in milliseconds will be written.
/// @param[out] pWriteTimeoutInMilliseconds points to a buffer into which the @ref HidUart_WriteLatch() operation timeout in milliseconds will be written.
///
/// @note Read and write timeouts are maintained for each device but are not persistent across @ref HidUart_Open()/@ref HidUart_Close().
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- pReadTimeoutInMilliseconds and/or pWriteTimeoutInMilliseconds is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetTimeouts(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _Out_writes_bytes_(sizeof(DWORD)) _Pre_defensive_ DWORD* pReadTimeoutInMilliseconds, _Out_writes_bytes_(sizeof(DWORD)) _Pre_defensive_ DWORD* pWriteTimeoutInMilliseconds);

/// @brief Returns the number of bytes held in the device receive and transmit FIFO. Returns the parity/error
/// status and line break status.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] transmitFifoSize - Returns the number of bytes currently held in the device transmit FIFO.
/// @param[out] receiveFifoSize - Returns the number of bytes currently held in the device receive FIFO.
/// @param[out] errorStatus - Returns an error status bitmap describing @ref HID_UART_PARITY_ERROR parity and @ref HID_UART_OVERRUN_ERROR overrun errors function clears the errors.
/// @param[out] lineBreakStatus - Returns @ref HID_UART_LINE_BREAK_ACTIVE if line break is currently active and HID_UART_LINE_BREAK_INACTIVE otherwise.
///
/// @note The transmitFifoSize and receiveFifoSize only apply to data held in the device FIFOs; they do not include
/// data queued in the HID driver or interface library
///
/// @note Calling HidUart_GetUartStatus() reports the current errorStatus and then clears the errors.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- transmitFifoSize, and/or receiveFifoSize, and/or errorStatus, and/or lineBreakStatus is an unexpected value
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetUartStatus(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* transmitFifoSize, _Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* receiveFifoSize, _Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* errorStatus, _Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* lineBreakStatus);

/// @brief Sets the baud rate, data bits, parity, stop bits, and flow control.
///
/// Refer to the device data sheet for a list of supported configuration settings.
///
/// See also @ref HidUart_GetUartConfig().
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] baudRate is the 4-byte value for the baud rate for UART communication.
/// @param[in] dataBits is the 1-byte value for the number of data bits for UART communication. see @ref HID_UART_NUMBER_DATA_BITS.
/// @param[in] parity is the 1-byte value for the parity for UART communication. see @ref HID_UART_PARITY.
/// @param[in] stopBits is the 1-byte value for the number of stop bits for UART communication. see @ref HID_UART_STOP_BITS.
/// @param[in] flowControl is the 1-byte value for the type of flow control for UART communication. see @ref HID_UART_FLOW_CONTROL.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- dataBits and/or parity and/or stopBits and/or flowControl is an unexpected value
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetUartConfig(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _In_ _Pre_defensive_ const DWORD baudRate, _In_ _Pre_defensive_ const BYTE dataBits, _In_ _Pre_defensive_ const BYTE parity, _In_ _Pre_defensive_ const BYTE stopBits, _In_ _Pre_defensive_ const BYTE flowControl);

/// @brief Gets the baud rate, data bits, parity, stop bits, and flow control.
///
/// Refer to the device data sheet for a list of supported configuration settings.
///
/// See also @ref HidUart_SetUartConfig().
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] pBaudRate points to a 4-byte buffer into which the value for the baud rate for UART communication is written and returned. 
/// @param[out] pDataBits points to a 1-byte buffer into which the value for the number of data bits for UART communication is written and returned. see @ref HID_UART_NUMBER_DATA_BITS.
/// @param[out] pParity points to a 1-byte buffer into which the value for the parity for UART communication is written and returned. see @ref HID_UART_PARITY.
/// @param[out] pStopBits points to a 1-byte buffer into which the value for the number of stop bits for UART communication is written and returned. see @ref HID_UART_STOP_BITS.
/// @param[out] pFlowControl points to a 1-byte buffer into which the value for the type of flow control for UART communication is written and returned. see @ref HID_UART_FLOW_CONTROL.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- pBaudRate, and/or pDataBits, and/or pParity, and/or pStopBits and/or pFlowControl is an unexpected value
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetUartConfig(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _Out_writes_bytes_(sizeof(DWORD)) _Pre_defensive_ DWORD* pBaudRate, _Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* pDataBits, _Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* pParity, _Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* pStopBits, _Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* pFlowControl);

/// @brief Causes the device to transmit a line break, holding the TX pin low, for the specified duration in milliseconds.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] DurationInMilliseconds is the 1-byte length of time in milliseconds to transmit the line break (1-125 ms).
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- DurationInMilliseconds is an unexpected value
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_StartBreak(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _In_ _Pre_defensive_ const BYTE DurationInMilliseconds);

/// @brief Stops the device from transmitting a line break.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
///
/// @note This function successfully no-ops if the device is not transmitting a line break. See @ref HidUart_StartBreak().
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_StopBreak(_In_ _Pre_defensive_ const HID_UART_DEVICE device);

/// @brief Initiates a full device reset.
///
/// Transmit and receive FIFOs will be cleared, UART settings will be reset to
/// default values (115200, 8N1, no flow control), and the device will re-enumerate.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
///
/// @note The device parameter is stale after successfully calling HidUart_Reset(). ??TBD Can calose be called? should close be called? Set device to NULL after calling HidUart_Reset or @ref HidUart_Close()?.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_Reset(_In_ _Pre_defensive_ const HID_UART_DEVICE device);

/// @brief Get the current port latch value from the device
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] pLatchValue points to a 2-byte buffer into which the port latch value is written and returned. see @ref PinConfigModeArrayIndices.
///
/// The port latch value (Logic High = 1, Logic Low = 0) as a GPIO input or flow
/// control pin that is an input, then the corresponding bit represents the input value. If a pin is configured
/// as a GPIO output pin or a flow control pin that is an output, then the corresponding bit
/// represents the logic level driven on the pin.
///
/// See @ref PinConfigModeArrayIndices Port Latch Pin Definition for more information on configuring GPIO and flow control pins. Bits
/// 9 and 15 of *pLatchValue are ignored.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- pLatchValue is an unexpected value
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_ReadLatch(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* pLatchValue);

/// @brief Sets the current port latch value to the device
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] LatchValue a 2-byte port latch value to write to the device. see @ref PinConfigModeArrayIndices.
/// @param[in] LatchMask is a 2-byte bitmask which determines which bits of the port latch value to change (Change = 1, Leave = 0).
///
/// The port latch value (Logic High = 1, Logic Low = 0) used to set the values
/// for GPIO pins or flow control pins that are configured as outputs.This function will not affect any
/// pins that are not configured as outputs.
///
/// See @ref PinConfigModeArrayIndices Port Latch Pin Definition for more information on configuring GPIO and flow control pins. Bits
/// 9 and 15 of LatchValue and LatchMask are ignored. Pins TX, RX, Suspend, and /Suspend cannot be written to using this function.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_WriteLatch(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _In_ _Pre_defensive_ const WORD LatchValue, _In_ _Pre_defensive_ const WORD LatchMask);

/// @brief Retrieves the part number and version of the CP211x device
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] pPartNumber points to a 1-byte buffer into which the device part number is written and returned. see @ref PinConfigModeArrayIndices.
/// @param[out] pVersion points to a 1-byte buffer into which the version value is written and returned.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- pPartNumber and/or pVersion is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetPartNumber(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* pPartNumber, _Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* pVersion);

/// @brief Returns the HID-to-UART Interface Library version information.
///
/// @param[out] pMajor points to a 1-byte buffer into which the major library version is written and returned.
/// @param[out] pMinor points to a 1-byte buffer into which the minor library version value is written and returned.
/// @param[out] pIsReleaseBuild points to a 4-byte buffer into which TRUE if the library is a release build, otherwise the library is a Debug build is written and returned.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_PARAMETER -- pMajor and/or pMinor and/or pIsReleaseBuild is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetLibraryVersion(_Out_writes_bytes_(1) BYTE* pMajor, _Out_writes_bytes_(1) BYTE* pMinor, _Out_writes_bytes_(sizeof(BOOL)) BOOL* pIsReleaseBuild);

/// @brief Returns the version of the underlying HID Device Interface Library that is currently in use.
///
/// @param[out] pMajor points to a 1-byte buffer into which the major library version is written and returned.
/// @param[out] pMinor points to a 1-byte buffer into which the minor library version value is written and returned.
/// @param[out] pIsReleaseBuild points to a 4-byte buffer into which TRUE if the library is a release build, otherwise the library is a Debug build is written and returned.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_PARAMETER -- pMajor and/or pMinor and/or pIsReleaseBuild is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetHidLibraryVersion(_Out_writes_bytes_(1) BYTE* pMajor, _Out_writes_bytes_(1) BYTE* pMinor, _Out_writes_bytes_(sizeof(BOOL)) BOOL* pIsReleaseBuild);

/// @brief Return the GUID for HIDClass devices ("return the HID GUID") (Windows only)
///
/// The HID GUID can be used to register for surprise removal notifications. See MSDNURL.
///
/// @param[out] pHIDGuid points at a caller-allocated GUID buffer into which the HID GUID value will be written
///
/// @note BUG: This is an buffer-UNSAFE function, not defensive against a "too small" buffer provided as the pHIDGuid parameter.
#ifdef _WIN32
/// @deprecated please use @ref HidUart_GetHidGuidSafe()
#endif
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_PARAMETER -- pHIDGuid is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetHidGuid(_Out_ void* pHIDGuid);
#ifdef _WIN32
/// @brief Return the GUID for HIDClass devices ("return the HID GUID") (Windows only)
///
/// The HID GUID can be used to register for surprise removal notifications. See MSDNURL.
///
/// @param[out] pHIDGuid points at a caller-allocated guiddef.h::GUID buffer into which the HID GUID value will be written
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_PARAMETER -- pHIDGuid is an unexpected value

_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetHidGuidSafe(_Out_ GUID *pHIDGuid);
#endif	// _WIN32

/// @brief Permanently locks/disables device customization
///
/// @details When this function is successfully called, the specified fields are fully locked and cannot be further
/// customized. The user customization functions can be called and may return HID_UART_SUCCESS
/// even though the device was not programmed. Call the function's corresponding get function to verify
/// that customization was successful. Each field is stored in one time programmable memory (OTP) and
/// can only be customized once. After a field is customized, the corresponding lock bits are set to 0.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open(). See @ref HID_UART_LOCK_BITMASKS.
/// @param[in] lock - Bitmask specifying which fields can be customized/programmed (@ref HID_UART_LOCK_UNLOCKED) and which fields are already customized (@ref HID_UART_LOCK_LOCKED).
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetLock(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _In_ _Pre_defensive_ const WORD lock);

/// @brief Returns the device customization lock status.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] lock - Returns a bitmask specifying which fields are locked (@ref HID_UART_LOCK_LOCKED). See @ref HID_UART_LOCK_BITMASKS.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- lock is an unexpected value
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetLock(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* lock);

/// @brief Allows one-time customization of the USB configuration, which includes vendor ID, product ID, power,
/// power mode, release version, and flush buffers setting.Each field can be independently programmed
/// one time each via the mask field.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] vid is the 2-byte Vendor ID value.
/// @param[in] pid is the 2-byte Product ID value.
/// @param[in] power is the 1-byte power value which specifies the current requested by the device IFF the device is configured to be bus powered. See @ref HID_UART_BUS_POWER_MAX.
/// @param[in] powerMode is the 1-byte Power Mode value to configure the device as bus powered or self powered. See @ref HID_UART_POWER_MODES.
/// @param[in] releaseVersion is the 2-byte Major.Minor Release Version value.
/// @param[in] flushBuffers is the 2-byte Flush Buffers value.  See @ref HID_UART_FLUSH_BITMASKS.
/// @param[in] mask is the 1-byte bitmask value specifying which fields to customize.  See @ref HID_UART_SET_BITMASKS.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- power and/or powerMode is an unexpected value
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetUsbConfig(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _In_ const WORD vid, _In_ const WORD pid, _In_ const BYTE power, _In_ const BYTE powerMode, _In_ const WORD releaseVersion, _In_ const BYTE flushBuffers, _In_ const BYTE mask);

/// @brief Retrieves USB configuration, which includes vendor ID, product ID, power, power mode, release version,
/// and flush buffers setting.
///

/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] pVid points to a 1-byte buffer into which the Vendor ID  is written and returned.
/// @param[out] pPid points to a 1-byte buffer into which the Product ID  is written and returned.
/// @param[out] pPower points to a 1-byte buffer into which the power value which specifies the current requested by the device IFF the device is configured to be bus powered  is written and returned. See @ref HID_UART_BUS_POWER_MAX.
/// @param[out] pPowerMode points to a 1-byte buffer into which the Power Mode value to configure the device as bus powered or self powered  is written and returned. See @ref HID_UART_POWER_MODES.
/// @param[out] pReleaseVersion points to a 1-byte buffer into which the Major.Minor Release Version value is written and returned.
/// @param[out] pFlushBuffers is the 2-byte Flush Buffers value is written and returned.  See @ref HID_UART_FLUSH_BITMASKS.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- pVid and/or pPid and/or pPower and/or pPowerMode and/or pReleaseVersion and/or pFlushBuffers is an unexpected value
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetUsbConfig(_In_ _Pre_defensive_ const HID_UART_DEVICE device, _Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* pVid, _Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* pPid, _Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* pPower, _Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* pPowerMode, _Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* pReleaseVersion, _Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* pFlushBuffers);

/// @brief Allows one-time customization of the USB manufacturing string.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] manufacturingString Variable of type HID_UART_CP2110/4_MFG_STR, a 62-byte character
/// buffer containing the ASCII manufacturing string..
/// @param[in] strlen The length of manufacturingString in bytes.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- manufacturingString and/or strlen is an unexpected value
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetManufacturingString(_In_ _Pre_defensive_ const HID_UART_DEVICE device, char* manufacturingString, _In_ _Pre_defensive_ const BYTE strlen);

/// @brief Retrieves the USB manufacturing string
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] manufacturingString Variable of type HID_UART_CP2110/4_MFG_STR, a 62-byte character
/// buffer that will contain the ASCII manufacturing string.
/// @param[out] strlen Returns the length of the string in bytes.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- manufacturingString and/or strlen is an unexpected value
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetManufacturingString(_In_ _Pre_defensive_ const HID_UART_DEVICE device, char* manufacturingString, _Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* strlen);

/// @brief Allows one-time customization of the USB product string
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] productString Variable of type HID_UART_CP2110/4_PRODUCT_STR, a 62-byte character
/// buffer containing the ASCII product string.
/// @param[in] strlen The length of productString in bytes.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- productString and/or strlen is an unexpected value
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetProductString(_In_ _Pre_defensive_ const HID_UART_DEVICE device, char* productString, _In_ _Pre_defensive_ const BYTE strlen);

/// @brief Retrieves the USB product string
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] productString Variable of type HID_UART_CP2110/4_PRODUCT_STR, a 62-byte character
/// buffer that will contain the ASCII product string.
/// @param[out] strlen Returns the length of the string in bytes.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- productString and/or strlen is an unexpected value
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetProductString(_In_ _Pre_defensive_ const HID_UART_DEVICE device, char* productString, _Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* strlen);

/// @brief Allows one-time customization of the USB serial number string
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] serialString - Variable of type HID_UART_CP2110/4_SERIAL_STR, a 30-byte character buffer
/// containing the ASCII serial number string.
/// @param[in] strlen - The length of serialString in bytes.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- serialString and/or strlen is an unexpected value
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetSerialString(_In_ _Pre_defensive_ const HID_UART_DEVICE device, char* serialString, _In_ _Pre_defensive_ const BYTE strlen);

/// @brief Retrieves the USB Serial Number string
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] serialString - Variable of type HID_UART_CP2110/4_SERIAL_STR, a 30-byte character buffer
/// that will contain the Unicode product string.
/// @param[out] strlen - Returns the length of the string in bytes.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref _HID_UART_STATUS "HID_UART_STATUS" Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- serialString and/or strlen is an unexpected value
///	@retval	#HID_UART_DEVICE_IO_FAILED -- failed, the device failed to respond to I/O in any expected manner
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetSerialString(_In_ _Pre_defensive_ const HID_UART_DEVICE device, char* serialString, _Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* strlen);

#ifdef __cplusplus
}
#endif // __cplusplus

/// @}

#endif // HOST_LIB_SLABHIDUART_INCLUDE_SLABHIDTOUART_H_INCLUDED_XF4QYQM3BK
