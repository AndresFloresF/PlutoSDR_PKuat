/////////////////////////////////////////////////////////////////////////////
// SLABCP2110.h
// For SLABHIDtoUART.dll
// and Silicon Labs CP2110 HID to UART
/////////////////////////////////////////////////////////////////////////////
#ifndef SLAB_CP2110_H
#define SLAB_CP2110_H

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "SLABHIDtoUART.h"
#include "silabs_sal.h"

/////////////////////////////////////////////////////////////////////////////
// Pin Definitions
/////////////////////////////////////////////////////////////////////////////

/// Pin Config Mode Array Indices
#if defined(NOT_YET)
typedef enum _CP2110_PIN_CONFIG_INDEX {
	CP2110_PIN_CONFIG_INDEX_GPIO_0_CLK			= 0		///< GPIO.0/CLK
	, CP2110_PIN_CONFIG_INDEX_GPIO_1_RTS		= 1		///< GPIO.1/RTS
	, CP2110_PIN_CONFIG_INDEX_GPIO_2_CTS		= 2		///< GPIO.2/CTS
	, CP2110_PIN_CONFIG_INDEX_GPIO_3_RS485		= 3		///< GPIO.3/RS485
	, CP2110_PIN_CONFIG_INDEX_GPIO_4_TX_TOGGLE	= 4		///< GPIO.4/TX Toggle
	, CP2110_PIN_CONFIG_INDEX_GPIO_5_RX_TOGGLE	= 5		///< GPIO.5/RX Toggle
	, CP2110_PIN_CONFIG_INDEX_GPIO_6			= 6		///< GPIO.6
	, CP2110_PIN_CONFIG_INDEX_GPIO_7			= 7		///< GPIO.7
	, CP2110_PIN_CONFIG_INDEX_GPIO_8			= 8		///< GPIO.8
	, CP2110_PIN_CONFIG_INDEX_GPIO_9			= 9		///< GPIO.9
	, CP2110_PIN_CONFIG_INDEX_TX				= 10	///< TX
	, CP2110_PIN_CONFIG_INDEX_SUSPEND			= 11	///< Suspend
	, CP2110_PIN_CONFIG_INDEX_SUSPEND_BAR		= 12	///< /Suspend
	, CP2110_INDEXES_NUMBER_OF_INDEXES
} CP2110_PIN_CONFIG_INDEX, *PCP2110_PIN_CONFIG_INDEX;
#else // defined(NOT_YET)
/// @defgroup PinConfigModeArrayIndices Pin Config Mode Array indices
/// @{
#define CP2110_INDEX_GPIO_0_CLK			            0	///< GPIO.0/CLK
#define CP2110_INDEX_GPIO_1_RTS			            1	///< GPIO.1/RTS
#define CP2110_INDEX_GPIO_2_CTS			            2	///< GPIO.2/CTS
#define CP2110_INDEX_GPIO_3_RS485			        3	///< GPIO.3/RS485
#define CP2110_INDEX_GPIO_4_TX_TOGGLE		        4	///< GPIO.4/TX Toggle
#define CP2110_INDEX_GPIO_5_RX_TOGGLE		        5	///< GPIO.5/RX Toggle
#define CP2110_INDEX_GPIO_6				            6	///< GPIO.6
#define CP2110_INDEX_GPIO_7				            7	///< GPIO.7
#define CP2110_INDEX_GPIO_8				            8	///< GPIO.8
#define CP2110_INDEX_GPIO_9				            9	///< GPIO.9
#define CP2110_INDEX_TX					            10	///< TX
#define CP2110_INDEX_SUSPEND				        11	///< Suspend
#define CP2110_INDEX_SUSPEND_BAR			        12	///< /Suspend
// Size of the above array pointed to by pinConfig parameter in @ref HidUart_SetPinConfig() and @ref HidUart_GetPinConfig()
#define CP2110_PIN_CONFIG_SIZE					13	///< Config size, number of indexes
/// @}
#endif // defined(NOT_YET)

/// Pin Bitmasks for Suspend
/// @defgroup PinBitmasksForSuspend Pin Bitmasks for Suspend definitions
/// @{
// TODO: Enstone: typedef enum _CP2110_BITMASK_FOR_SUSPEND { } CP2110_BITMASK_FOR_SUSPEND, *PCP2110_BITMASK_FOR_SUSPEND;
#define CP2110_MASK_GPIO_0_CLK			0x0001	///< GPIO.0/CLK
#define CP2110_MASK_GPIO_1_RTS			0x0002	///< GPIO.1/RTS
#define CP2110_MASK_GPIO_2_CTS			0x0004	///< GPIO.2/CTS
#define CP2110_MASK_GPIO_3_RS485		0x0008	///< GPIO.3/RS485
#define CP2110_MASK_TX					0x0010	///< TX
#define CP2110_MASK_RX					0x0020	///< RX
#define CP2110_MASK_GPIO_4_TX_TOGGLE	0x0040	///< TX Toggle
#define CP2110_MASK_GPIO_5_RX_TOGGLE	0x0080	///< RX Toggle
#define CP2110_MASK_SUSPEND_BAR			0x0100	///< /Suspend
#define CP2110_MASK_NA					0x0200	///< N/A, unused
#define CP2110_MASK_GPIO_6				0x0400	///< GPIO.6
#define CP2110_MASK_GPIO_7				0x0800	///< GPIO.7
#define CP2110_MASK_GPIO_8				0x1000	///< GPIO.8
#define CP2110_MASK_GPIO_9				0x2000	///< GPIO.9
#define CP2110_MASK_SUSPEND				0x4000	///< Suspend
#define CP2110_MASK_NAII				0x8000	///< N/A, unused
/// @}

/////////////////////////////////////////////////////////////////////////////
// Exported Library Functions
/////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// HidUart_SetPinConfig
/// @brief Sets the Pin Config to the device. Allows one-time configuration of the GPIO mode for each pin.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] pinConfig points at a 13/@ref CP2110_PIN_CONFIG_SIZE-byte buffer that configures the GPIO mode for each of the 13 pins. The RX pin is not configurable. See @ref PinConfigModes.
/// @param[in] useSuspendValues Specifies if the device is to use suspendValue and suspendMode when device is in USB suspend. If set to 1, the device will use these values. If cleared to 0, the device's GPIO pins will remain in the state they were in before entering USB suspend.
/// @param[in] suspendValue This is the latch value that will be driven on each GPIO pin when the device is in a suspend state. See @ref SuspendValueBitValues.
/// @param[in] suspendMode Specifies the mode for each GPIO pin when the device is in a suspend state. See @ref SuspendModeBitValues.
/// @param[in] rs485Level Specifies the RS-485 pin level of GPIO.2 when configured in RS-485 mode. See @ref RS485ActiveLevels.
/// @param[in] clkDiv Divider applied to GPIO0_CLK clock outputFor 1-255, the output frequency is 24MHz / (2 x clkDiv).
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_INVALID_PARAMETER -- pinConfig is an unexpected value
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2110
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetPinConfig(
	_In_ _Pre_defensive_ const HID_UART_DEVICE device,
	_In_reads_bytes_(CP2110_PIN_CONFIG_SIZE) _Pre_defensive_ BYTE* pinConfig,
	_In_ _Pre_defensive_ const BOOL useSuspendValues,
	_In_ _Pre_defensive_ const WORD suspendValue,
	_In_ _Pre_defensive_ const WORD suspendMode,
	_In_ _Pre_defensive_ const BYTE rs485Level,
	_In_ _Pre_defensive_ const BYTE clkDiv
);

// HidUart_GetPinConfig
/// @brief Reads and returns the Pin Config from the device
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] pinConfig points at a 13/@ref CP2110_PIN_CONFIG_SIZE-byte buffer into which the pin config will be written
/// @param[out] useSuspendValues points at a BOOL into which the configuration for using the values in suspendValue and suspendMode when in suspend mode will be written. This bit is the same as bit 15 of suspendMode.
/// @param[out] suspendValue points at a 2-byte buffer into which the latch value that will be driven on each GPIO pin when the device is in a suspend state value will be written
/// @param[out] suspendMode points at a 2-byte buffer into which the mode for each GPIO pin when the device is in a suspend state value will be written
/// @param[out] rs485Level points at a 1-byte buffer into which the RS-485 pin level of GPIO.2 when configured in RS-485 mode value will be written
/// @param[out] clkDiv points at a 1-byte buffer into which the Divider applied to GPIO0_CLK clock outputFor 1-255, the output frequency is 24MHz / (2 x clkDiv).value will be written
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2110
///	@retval	#HID_UART_INVALID_PARAMETER -- pinConfig or useSuspendValues or suspendValue or suspendMode or rs485Level or clkDiv is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetPinConfig(
	_In_ _Pre_defensive_ const HID_UART_DEVICE device,
	_Out_writes_bytes_(CP2110_PIN_CONFIG_SIZE) _Pre_defensive_ BYTE* pinConfig,
	_Out_writes_bytes_(sizeof(BOOL)) _Pre_defensive_ BOOL* useSuspendValues,
	_Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* suspendValue,
	_Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* suspendMode,
	_Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* rs485Level,
	_Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* clkDiv
);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLAB_CP2110_H
