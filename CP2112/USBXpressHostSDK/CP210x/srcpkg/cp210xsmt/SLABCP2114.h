/////////////////////////////////////////////////////////////////////////////
// SLABCP2114.h
// For SLABHIDtoUART.dll
// and Silicon Labs CP2114 HID to UART
/////////////////////////////////////////////////////////////////////////////
#ifndef SLAB_CP2114_H
#define SLAB_CP2114_H

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "SLABHIDtoUART.h"
#include "CP2114_Common.h"

/////////////////////////////////////////////////////////////////////////////
// Return Code Definitions
/////////////////////////////////////////////////////////////////////////////

/// @defgroup ConfigErrorDefs Errors associated with Config definitions
/// @{
#define HID_UART_INVALID_CONFIG_NUMBER              kInvalidConfigNumber        ///< Requested config number >= MAX_CONFIGS
#define HID_UART_BOOT_INDEXES_DEPLETED              kBootIndicesDepleted        ///< All boot indices have been used
#define HID_UART_REQUESTED_CONFIG_NOT_PRESENT       kRequestedConfigNotPresent  ///< Pointer to requested config is 0xFFFF
#define HID_UART_CONFIG_INVALID                     kRequestedConfigInvalid     ///< Specified config consists of invalid parameters
#define HID_UART_CONFIG_POINTERS_DEPLETED           kConfigPointersDepleted     ///< All Config pointer slots have been used
#define HID_UART_CONFIG_SPACE_DEPLETED              kConfigSpaceDepleted        ///< Not enough space to save the new config
#define HID_UART_BOOT_INDEX_UNCHANGED               kBootIndexUnchanged         ///< The user-specified boot index is already the current boot index
#define HID_UART_CONFIG_UNCHANGED                   kConfigUnchanged            ///< The Config is already as the user requested
#define HID_UART_INVALID_CONFIG_SEQUENCE_IDENTIFIER kConfigInvalidConfigIdentifier  ///< Invalid Config string ID
#define HID_UART_INVALID_CONFIG_SETTINGS            kConfigSettingsInvalid      ///< Configuration contains invalid setting(s)
#define HID_UART_UNSUPPORTED_CONFIG_FORMAT          kUnsupportedConfigFormat    ///< The config format version is not supported by library/firmware
/// @}

// 'Set Parameters for Next Get' errors
#define HID_UART_INVALID_NUMBER_OF_CACHED_PARAMS    kInvalidNumberOfCachedParams///< Specified tSetParamsForNextGet.params > MAX_CACHED_PARAMS
#define HID_UART_UNEXPECTED_CACHE_DATA              kUnexpectedCacheData        ///< Something in tSetParamsForNextGet was not as expected

/// @defgroup I2CErrorsDefs I2C Error definitions
/// @{
#define HID_UART_I2C_BUSY                           kI2cBusy                    ///< I2C bus is busy
#define HID_UART_I2C_TIMEOUT                        kI2cTimeout                 ///< Timeout waiting for I2C event (start, ACK, etc.)
#define HID_UART_I2C_INVALID_TOKEN                  kI2cInvalidToken            ///< I2C interpreter detected invalid token
#define HID_UART_I2C_INVALID_WRITE_LENGTH           kI2cInvalidWriteLength      ///< Specified number of bytes to write is 0 or too large
#define HID_UART_I2C_INVALID_CONFIG_LENGTH          kI2cInvalidConfigLength     ///< Specified configuration length is invalid
#define HID_UART_I2C_SCL_STUCK_LOW                  kI2cSclStuckLow             ///< SCL line is stuck low
#define HID_UART_I2C_SDA_STUCK_LOW                  kI2cSdaStuckLow             ///< SDA line is stuck low
/// @}


/////////////////////////////////////////////////////////////////////////////
// Pin Definitions
/////////////////////////////////////////////////////////////////////////////
/// Pin Config Mode Array indice
/// @defgroup PinConfigModeArrayIndices Pin Config Mode Array indices
/// @{
#define CP2114_INDEX_GPIO_0                         0 ///< CP2114 gpio 0 index
#define CP2114_INDEX_GPIO_1                         1 ///< CP2114 gpio 1 index
#define CP2114_INDEX_GPIO_2                         2 ///< CP2114 gpio 2 index
#define CP2114_INDEX_GPIO_3                         3 ///< CP2114 gpio 3 index
#define CP2114_INDEX_GPIO_4                         4 ///< CP2114 gpio 4 index
#define CP2114_INDEX_GPIO_5                         5 ///< CP2114 gpio 5 index
#define CP2114_INDEX_GPIO_6                         6 ///< CP2114 gpio 6 index
#define CP2114_INDEX_GPIO_7                         7 ///< CP2114 gpio 7 index
#define CP2114_INDEX_GPIO_8                         8 ///< CP2114 gpio 8 index
#define CP2114_INDEX_GPIO_9                         9 ///< CP2114 gpio 9 index
#define CP2114_INDEX_TX                             10 ///< CP2114 tx index
#define CP2114_INDEX_RX                             11 ///< CP2114 rx index
#define CP2114_INDEX_SUSPEND                        12 ///< CP2114 suspend index
#define CP2114_INDEX_SUSPEND_BAR                    13 ///< CP2114 suspend bar index
// Size of the above array pointed to by pinConfig parameter in CP2114_SetPinConfig and CP2114_GetPinConfig
#define CP2114_PIN_CONFIG_SIZE                      14	///< Config size, number of indexes
/// @}

/// Pin Bitmasks
/// @defgroup PinBitmasks Pin Bitmasks definitions
/// @{
#define CP2114_MASK_GPIO_0                          0x0001 ///< CP2114 gpio 0 mask
#define CP2114_MASK_GPIO_1                          0x0002 ///< CP2114 gpio 1 mask
#define CP2114_MASK_GPIO_2                          0x0004 ///< CP2114 gpio 2 mask
#define CP2114_MASK_GPIO_3                          0x0008 ///< CP2114 gpio 3 mask
#define CP2114_MASK_GPIO_4                          0x0010 ///< CP2114 gpio 4 mask
#define CP2114_MASK_GPIO_5                          0x0020 ///< CP2114 gpio 5 mask
#define CP2114_MASK_GPIO_6                          0x0040 ///< CP2114 gpio 6 mask
#define CP2114_MASK_GPIO_7                          0x0080 ///< CP2114 gpio 7 mask
#define CP2114_MASK_GPIO_8                          0x0100 ///< CP2114 gpio 8 mask
#define CP2114_MASK_GPIO_9                          0x0200 ///< CP2114 gpio 9 mask
#define CP2114_MASK_TX                              0x0400 ///< CP2114 gpio tx mask
#define CP2114_MASK_RX                              0x0800 ///< CP2114 gpio rx mask
#define CP2114_MASK_SUSPEND                         0x1000 ///< CP2114 gpio suspend mask
#define CP2114_MASK_SUSPEND_BAR                     0x2000 ///< CP2114 gpio suspend bar mask
/// @}

/////////////////////////////////////////////////////////////////////////////
// DAC Definitions
/////////////////////////////////////////////////////////////////////////////

#define MAX_DAC_CONFIG_SIZE (2*512 - sizeof(CP2114_RAM_CONFIG_STRUCT)) ///< CP2114 Max DAC config size

// 60 is the largest common multiple of 2, 3 and 4. This ensures that 
// an I2C transactions contained in the SET/GET_DAC_REGISTERS payload 
// will not be split across multiple frames.
#define DAC_REGISTERS_PAYLOAD_MAX_LEN               60 ///< CP2114 DAC registers payload max lenth

/////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////

typedef tDeviceCaps CP2114_CAPS_STRUCT, *PCP2114_CAPS_STRUCT;	///< CP2114 device capabilities

typedef _RAM_CONFIG_STRUCT CP2114_RAM_CONFIG_STRUCT, *PCP2114_RAM_CONFIG_STRUCT;	///< CP2114 RAM configuration

/////////////////////////////////////////////////////////////////////////////
// DAC Structures
/////////////////////////////////////////////////////////////////////////////
/// @union _CP2114_OTP_CONFIG SLABCP2114.h
/// @brief tbd
typedef union _CP2114_OTP_CONFIG
{
	/// @struct CP2114_B01 SLABCP2114.h
	/// @brief if config_version == CP2114_CONFIG_VERSION_B01
	struct // if config_version == CP2114_CONFIG_VERSION_B01
    {
        BYTE RemConfig[ RAM_CONFIG_SIZE_B01]; ///< Rem configuration
        BYTE DacConfig[ MAX_DAC_CONFIG_SIZE]; ///< Dac configuration
    } CP2114_B01; ///< CP2114_CONFIG_VERSION_B01
	/// @struct CP2114_B02 SLABCP2114.h
	/// @brief if config_version == CP2114_CONFIG_VERSION_B02
	struct // if config_version == CP2114_CONFIG_VERSION_B02
    {
        BYTE PemConfig[ RAM_CONFIG_SIZE_B02]; ///< Pem configuration
        BYTE DacConfig[ MAX_DAC_CONFIG_SIZE]; ///< Dac configuration
    } CP2114_B02; ///< CP2114_CONFIG_VERSION_B02
    BYTE Other[0xffff]; ///< Max size that can be specified in 2 bytes
} CP2114_OTP_CONFIG, *PCP2114_OTP_CONFIG;

/// @struct _CP2114_OTP_CONFIG_GET SLABCP2114.h
/// @brief tbd
typedef struct _CP2114_OTP_CONFIG_GET
{
    U16 Length; ///< length
    CP2114_OTP_CONFIG OtpConfig; ///< otp config
} CP2114_OTP_CONFIG_GET, *PCP2114_OTP_CONFIG_GET;

/////////////////////////////////////////////////////////////////////////////
// Exported Library Functions
/////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

// CP2114_GetVersions
/// @brief Reads and returns the CP2114 API and firmware versions.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] api_version points at a 1-byte buffer into which the API version of the device value will be written. See @ref TBD.
/// @param[out] fw_version points at a 1-byte buffer into which the firmware version of the device value will be written. See @ref TBD.
/// @param[out] config_version points at a 1-byte buffer into which whether the device is B01 or B02 value will be written. See @ref TBD.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2114
///	@retval	#HID_UART_INVALID_PARAMETER -- api_version or fw_version or config_version is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
CP2114_GetVersions(_In_ _Pre_defensive_  HID_UART_DEVICE device, _Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* api_version, _Out_writes_bytes_(1) BYTE* fw_version, _Out_writes_bytes_(1) BYTE* config_version);

// CP2114_SetPinConfig
/// @brief Sets the Pin Config to the device. Allows one-time configuration of the GPIO mode for each pin.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] pinConfig points at a 14/@ref CP2114_PIN_CONFIG_SIZE-byte buffer that configures the GPIO mode for each of the 13 pins. The RX pin is not configurable. See @ref PinConfigModes.
/// @param[in] useSuspendValues Specifies if the device is to use suspendValue and suspendMode when device is in USB suspend. If set to 1, the device will use these values. If cleared to 0, the device's GPIO pins will remain in the state they were in before entering USB suspend.
/// @param[in] suspendValue This is the latch value that will be driven on each GPIO pin when the device is in a suspend state. See @ref SuspendValueBitValues.
/// @param[in] suspendMode Specifies the mode for each GPIO pin when the device is in a suspend state. See @ref SuspendModeBitValues.
/// @param[in] rs485Level Specifies the RS-485 pin level of GPIO.2 when configured in RS-485 mode. See @ref RS485ActiveLevels.
/// @param[in] clkDiv Divider applied to GPIO0_CLK clock outputFor 1–255, the output frequency is 24MHz / (2 x clkDiv).
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
CP2114_SetPinConfig(
	_In_ _Pre_defensive_ const HID_UART_DEVICE device,
	_In_reads_bytes_(CP2114_PIN_CONFIG_SIZE) _Pre_defensive_ BYTE* pinConfig,
	_In_ _Pre_defensive_ const BOOL useSuspendValues,
	_In_ _Pre_defensive_ const WORD suspendValue,
	_In_ _Pre_defensive_ const WORD suspendMode,
	_In_ _Pre_defensive_ const BYTE clkDiv
);

// CP2114_GetPinConfig
/// @brief Reads and returns the Pin Config from the device
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] pinConfig points at a 14/@ref CP2114_PIN_CONFIG_SIZE-byte buffer into which the pin config will be written
/// @param[out] useSuspendValues points at a BOOL into which the configuration for using the values in suspendValue and suspendMode when in suspend mode will be written. This bit is the same as bit 15 of suspendMode.
/// @param[out] suspendValue points at a 2-byte buffer into which the latch value that will be driven on each GPIO pin when the device is in a suspend state value will be written
/// @param[out] suspendMode points at a 2-byte buffer into which the mode for each GPIO pin when the device is in a suspend state value will be written
/// @param[out] rs485Level points at a 1-byte buffer into which the RS-485 pin level of GPIO.2 when configured in RS–485 mode value will be written
/// @param[out] clkDiv points at a 1-byte buffer into which the Divider applied to GPIO0_CLK clock outputFor 1–255, the output frequency is 24MHz / (2 x clkDiv).value will be written
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
CP2114_GetPinConfig(
	_In_ _Pre_defensive_ const HID_UART_DEVICE device,
	_Out_writes_bytes_(CP2114_PIN_CONFIG_SIZE) _Pre_defensive_ BYTE* pinConfig,
	_Out_writes_bytes_(sizeof(BOOL)) _Pre_defensive_ BOOL* useSuspendValues,
	_Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* suspendValue,
	_Out_writes_bytes_(sizeof(WORD)) _Pre_defensive_ WORD* suspendMode,
	_Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE* clkDiv
);

// CP2114_GetDeviceStatus
// This function shouldn't be called in normal conditions. The other APIs call this function.
// However, due to possible reenumeration after HidUart_SetRamConfig, getCP2114Status can't be called
// immediately after HidUart_SetRamConfig, this can be used to clear possible ConfigUnchanged status
// prior to the next command.
/// @brief Reads and returns the status of the device.
///
/// @note The device status is cleared on a read.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] pCP2114Status points at a 1-byte buffer into which the status byte value will be written. See @ref TBD.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2114
///	@retval	#HID_UART_INVALID_PARAMETER -- pCP2114Status is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
CP2114_GetDeviceStatus(
	_In_ _Pre_defensive_ const HID_UART_DEVICE device,
	_Out_writes_bytes_(sizeof(BYTE)) _Pre_defensive_ BYTE *pCP2114Status
);

// CP2114_GetDeviceCaps
/// @brief Reads and returns the CP2114 device capabilities.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] pCP2114CapsStruct points at a tbd-byte buffer into which the CP2114 device capabilities will be written. See @ref CP2114_CAPS_STRUCT.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2114
///	@retval	#HID_UART_INVALID_PARAMETER -- pCP2114CapsStruct is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
CP2114_GetDeviceCaps(
	_In_ _Pre_defensive_ const HID_UART_DEVICE device,
	_Out_writes_bytes_(sizeof(CP2114_CAPS_STRUCT)) _Pre_defensive_ PCP2114_CAPS_STRUCT pCP2114CapsStruct
);

// CP2114_SetRamConfig
/// @brief Configures the CP2114 RAM configuration parameters with the given values.
///
/// These settings are written
/// to the internal volatile RAM of the CP2114 and are overwritten on power cycle or reset with the
/// values contained in the specified boot configuration.
/// The CP2114 data sheet has more information on the audio configuration string format.
///
/// The intent of the CP2114_SetRamConfig() function is to allow temporary evaluation of minor configuration
/// changes(e.g.codec register settings) before programming the changes into a new OTP
/// EPROM configuration. However, there are some configuration elements that should not be changed
/// using this function.
///
/// Changing any of the following clocking options requires that the new configuration be written to OTP
/// EPROM because the clocking options are applied only when the CP2114 comes out of reset.Resetting
/// the CP2114 after applying a new RAM configuration is not an option, because at reset the existing
/// RAM configuration data will be overwritten with data from the specified boot configuration.
/// • USBCLK source(Internal / External)
/// • SYSCLK source(Internal / External)
/// • SYSCLK frequency(48.000 MHz or 49.152 MHz)
///
/// Changing certain other configuration options in RAM has been seen to cause problems with some
/// host operating systems. Presumably the problems are due to the host saving information from the
/// CP2114’s USB descriptors the first time a unique CP2114 is recognized, but not updating this information
/// when the same device re - enumerates with different capabilities. If improper host behavior is
/// observed after changing these(or any other) configuration options in RAM, a new OTP EPROM configuration
/// should be created instead.
/// • Audio synchronization mode(Asynchronous / Synchronous)
/// • CP2114 support for playback volume and mute
/// • Playback volume parameters(Min / Max / Resolution)
///
/// Follow these steps when switching between OTP EPROM configurations:
///		1. If all the GPIO 8 - 5 pins remain in their default codec select state, these pins can be used to select
///		the new configuration and the applied logic state can be changed at this time.
///
///		@note On the CP2114 - EK motherboard, JP16 connects GPIO, 8 to the CTS output of the RS - 232
/// 	level shifter device, and so must be disconnected when using GPIO 8 as a codec select line.
///
/// 	2. Otherwise, if any of the GPIO 8 - 5 pins have been reconfigured to something other than codec
/// 	select, the configuration utility must be used to program the OTP boot config with the index of the
/// 	desired configuration.
/// 	3. Disconnect the CP2114 from USB and power.
/// 	4. For Windows hosts, the CP2114 devices should be uninstalled. The USBDeview utility allows
/// 	users to uninstall USB devices on Windows, and can be run as a GUI or from the command line.
/// 	The following command uninstalls all CP2114s on a system : "C:\<pathname>\USBDeview.exe"
/// 	/ remove_by_pid 10C4; EAB0". The <pathname> tag represents the actual path to USBDeview.
/// 	exe file. Quotes are required (as shown) if the pathname contains spaces. The example
/// 	command specifies the CP2114’s default PID(0x10C4) and VID(0xEAB0) values; these arguments
/// 	must be changed if the VID or PID has been reprogrammed by the user.
/// 	5. Reconnect the CP2114 to power and USB.
/// 	6. Verify that the CP2114 device enumerates successfully.
/// 	7. Use the configuration utility to verify that the CP2114 is using the desired boot configuration.
/// 
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] pCP2114RamConfigStruct points at a tbd-byte buffer containing the RAM configuration parameter values that will be written. See @ref CP2114_RAM_CONFIG_STRUCT.
///
/// @note: For CP2114_SetRamConfig(), the Length field of the @ref CP2114_RAM_CONFIG_STRUCT does not matter. The size will be whatever the user application passes in.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2114
///	@retval	#HID_UART_INVALID_PARAMETER -- pCP2114RamConfigStruct is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
CP2114_SetRamConfig(
	_In_ _Pre_defensive_ const HID_UART_DEVICE device,
	_In_reads_bytes_(sizeof(CP2114_RAM_CONFIG_STRUCT)) _Pre_defensive_ const PCP2114_RAM_CONFIG_STRUCT pCP2114RamConfigStruct
);

// CP2114_GetRamConfig
/// @brief Reads and returns the current CP2114 RAM configuration parameters.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[out] pCP2114RamConfigStruct points at a >65-byte buffer into which the current CP2114 RAM configuration parameters values will be written. See @ref CP2114_RAM_CONFIG_STRUCT.
///
/// The pCP2114RamConfigStruct buffer must be at least 65 bytes long. The first two bytes will contain the U16 size of the
/// following RAM config block. The config block for B01 and B02 devices is described in the device
/// data sheet.
/// 
///  @note For CP2114_GetRamConfig(), the Length does not matter. The returned size will be the
/// size of the OTP configuration that was booted, or whatever the user application passed in if a
/// subsequent CP2114_SetRamConfig() call was made.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2114
///	@retval	#HID_UART_INVALID_PARAMETER -- pCP2114RamConfigStruct is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
CP2114_GetRamConfig(
	_In_ _Pre_defensive_ const HID_UART_DEVICE device,
	_Out_writes_bytes_(sizeof(CP2114_RAM_CONFIG_STRUCT)) _Pre_defensive_ PCP2114_RAM_CONFIG_STRUCT pCP2114RamConfigStruct
);

// CP2114_SetDacRegisters
/// @brief Configures the device or attached DAC using multiples of 2-byte or 3-byte sequences.
///
/// The first byte is DAC register address or special in - band command.
///
/// The following byte(s) is the data to write in the specified DAC register if preceded by DAC register
/// address, or parameter(s) of the in - band command if preceded by reserved in - band command IDs.
///
/// Some DACs have 8 - bit registers, some have 16 - bit registers. For 8 - bit registers, 2 - byte pairs shall be
/// used. For 16 - bit registers, 3 - byte triplets shall be used.
///
/// See the User's Guide for details on in-band commands.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] pDacConfigBuffer points at a tbd-byte buffer containing the sequence buffer.
/// @param[in] dacConfigBufferLengthInBytes Length in bytes of the sequences.
///
/// @note While the CP2114_SetDacRegisters() function is applicable to both CP2114-B01 and CP2114-B02
/// devices, the @ref CP2114_I2cWriteData() function supports a wider range of data formats and is recommended
/// for B02 devices.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2114
///	@retval	#HID_UART_INVALID_PARAMETER -- pDacConfigBuffer or dacConfigBufferLengthInBytes is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
CP2114_SetDacRegisters(
	_In_ _Pre_defensive_ const HID_UART_DEVICE device,
	_In_reads_bytes_(dacConfigBufferLengthInBytes) _Pre_defensive_ const BYTE* pDacConfigBuffer,
	_In_ _Pre_defensive_ const BYTE dacConfigBufferLengthInBytes
);

// CP2114_GetDacRegisters
/// @brief Reads from the specified DAC registers via the I2C interface.
///
/// Unlike @ref CP2114_SetDacRegisters(), this
/// API retrieves DAC register settings only without intercepting any in - band commands. The host should
/// ensure valid DAC register addresses are used.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] dacStartAddress Register address from which to start.
/// @param[in] dacRegistersToRead Number of registers to read.
/// @param[out] pDacConfigBuffer points at a 1-byte buffer into which the status byte value will be written. See @ref TBD.
///
/// @note While the CP2114_GetDacRegisters() function is applicable to both CP2114-B01 and CP2114-B02
/// devices, the @ref CP2114_I2cReadData() function supports a wider range of data formats and is recommended
/// for B02 devices.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2114
///	@retval	#HID_UART_INVALID_PARAMETER -- dacStartAddress or dacRegistersToRead or pDacConfigBuffer is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
CP2114_GetDacRegisters(
	_In_ _Pre_defensive_ const HID_UART_DEVICE device,
	_In_ _Pre_defensive_ const BYTE dacStartAddress,
	_In_ _Pre_defensive_ const BYTE dacRegistersToRead,
	_Out_ _Pre_defensive_ BYTE* pDacConfigBuffer
);

// CP2114_GetOtpConfig
/// @brief Retrieves a CP2114 configuration from OTP.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] cp2114ConfigNumber configuration number to retrieve CP2114 OTP.
/// @param[out] pCP2114ConfigStruct points at a x-byte buffer into which the configuration data returned from the device will be written. See @ref CP2114_OTP_CONFIG_GET.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2114
///	@retval	#HID_UART_INVALID_PARAMETER -- cp2114ConfigNumber or pCP2114ConfigStruct is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
CP2114_GetOtpConfig(
	_In_ _Pre_defensive_ const HID_UART_DEVICE device,
	_In_ _Pre_defensive_ const BYTE cp2114ConfigNumber,
	_Out_writes_bytes_(sizeof(CP2114_OTP_CONFIG_GET)) _Pre_defensive_ PCP2114_OTP_CONFIG_GET pCP2114ConfigStruct
);

// CP2114_CreateOtpConfig
/// @brief Creates a new CP2114 configuration in the available OTP space.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] configBufferLengthInBytes Length in bytes of the configuration to be written to OTP.
/// @param[in] pConfigBuffer points at a configBufferLengthInBytes-byte buffer containing the new configuration. See @ref TBD.
///
/// The buffer containing the new configuration is structured per @ref CP2114_CONFIG_STRUCT
/// excluding the U16 Length in CP2114_RAM_CONFIG_STRUCT.pConfigBuffer
/// has an internal structure that depends on the revision of the chip returned in config_version by
/// @ref CP2114_GetVersions() :
/// 	• B01—RAM_CONFIG_SIZE_B01 bytes of RAM configuration at the beginning, followed by
/// 	DAC configuration as the remainder of the data.
/// 	• B02—RAM_CONFIG_SIZE_B02 bytes of RAM configuration at the beginning, followed by
/// 	DAC configuration as the remainder of the data.
/// 	This function automatically inserts the 16-bit configBufferLength in front of the data before
///		writing to the OTP.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2114
///	@retval	#HID_UART_INVALID_PARAMETER -- configBufferLengthInBytes or pConfigBuffer is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
CP2114_CreateOtpConfig(
	_In_ _Pre_defensive_ HID_UART_DEVICE device,
	_In_ _Pre_defensive_ WORD configBufferLengthInBytes,
	_In_reads_bytes_(configBufferLengthInBytes) _Pre_defensive_ BYTE* pConfigBuffer
);

// CP2114_SetBootConfig
/// @brief Specifies the CP2114 configuration to be loaded from OTP on boot.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] cp2114ConfigNumber Configuration Index that will be set as the boot configuration upon reset. See @ref TBD.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2114
///	@retval	#HID_UART_INVALID_PARAMETER -- cp2114ConfigNumber is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
CP2114_SetBootConfig(
	_In_ _Pre_defensive_ const HID_UART_DEVICE device,
	_In_ _Pre_defensive_ const BYTE cp2114ConfigNumber
);

// CP2114_ReadOTP
/// @brief Returns partial or full OTP customization block.
///
/// The size of the OTP configuration space is 6 KB
/// (6144 bytes) for the CP2114 - B01 and 32 KB(32768 bytes) for the CP2114 - B02.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] cp2114Address The OTP address to read from.
/// @param[out] pReadBuffer points at a ReadLengthInBytes-byte buffer into which the data read from OTP space will be written. See @ref TBD.
/// @param[in] ReadLengthInBytes Length of OTP data to read, in bytes. See @ref TBD.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2114
///	@retval	#HID_UART_INVALID_PARAMETER -- cp2114Address or pReadBuffer or ReadLengthInBytes is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
CP2114_ReadOTP(
	_In_ _Pre_defensive_ const HID_UART_DEVICE device,
	_In_ _Pre_defensive_ const UINT cp2114Address,
	_Out_writes_bytes_(ReadLengthInBytes) _Pre_defensive_ BYTE* pReadBuffer,
	_In_ _Pre_defensive_ const UINT ReadLengthInBytes
);

// CP2114_WriteOTP
/// @brief Reads and returns the the status of the device.
///
/// @note The device status is cleared on a read.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] cp2114Address The OTP address to start writing to.
/// @param[out] pWriteBuffer points at a writeLengthInBytes-byte buffer from which the data written to OTP space. See @ref TBD.
/// @param[in] writeLengthInBytes The length of write buffer, in bytes. See @ref TBD.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2114
///	@retval	#HID_UART_INVALID_PARAMETER -- cp2114Address or pWriteBuffer or writeLengthInBytes is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
CP2114_WriteOTP(
	_In_ _Pre_defensive_ HID_UART_DEVICE device,
	_In_ _Pre_defensive_ UINT cp2114Address,
	_In_reads_bytes_(writeLengthInBytes) _Pre_defensive_ BYTE* pWriteBuffer,
	_In_ _Pre_defensive_ UINT writeLengthInBytes
);

// CP2114_I2cWriteData
/// @brief Write data to I2C Slave Device
///
/// @note CP2114-B02 only.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] slaveAddress The left-justified I2C slave address to use for the write transaction.
/// @param[in] pWriteBuffer points at a writeLengthInBytes-byte buffer from which the data is written to I2C slave device. See @ref TBD.
/// @param[in] writeLengthInBytes The length of write buffer, in bytes (maximum 2 bytes). See @ref TBD.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2114
///	@retval	#HID_UART_INVALID_PARAMETER -- slaveAddress or pWriteBuffer or writeLengthInBytes is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
CP2114_I2cWriteData(
	_In_ _Pre_defensive_ const HID_UART_DEVICE device,
	_In_ _Pre_defensive_ const BYTE slaveAddress,
	_In_reads_bytes_(writeLength) _Pre_defensive_ const BYTE* pWriteBuffer,
	_In_ _Pre_defensive_ const BYTE writeLengthInBytes
);

// CP2114_I2cReadData
/// @brief Reads and returns data from I2C Slave Device.
///
/// @note CP2114-B02 only.
///
/// @param[in] device is the Device object pointer as returned by @ref HidUart_Open().
/// @param[in] slaveAddress The left-justified I2C slave address to use for the write transaction.
/// @param[in] pWriteBuffer points at a WriteLengthInBytes-byte buffer from which the data is written to I2C slave device. See @ref TBD.
/// @param[in] WriteLengthInBytes The length of write buffer, in bytes (maximum 2 bytes). See @ref TBD.
/// @param[out] pReadBuffer points at a ReadLengthInBytes-byte buffer into which the data read from I2C slave device will be written. See @ref TBD.
/// @param[in] ReadLengthInBytes The number of bytes to read (maximum 60 bytes). The size of the read buffer must
/// be at least as large as ReadLengthInBytes. See @ref TBD.
///
/// @returns Returns #HID_UART_SUCCESS on success, or another @ref HID_UART_STATUS Return value if there is an error.
///	@retval	#HID_UART_SUCCESS -- success
///	@retval	#HID_UART_INVALID_DEVICE_OBJECT -- device is not a valid, recognized Device object
///	@retval	#HID_UART_DEVICE_NOT_SUPPORTED -- Per device's PartNum device is not a CP2114
///	@retval	#HID_UART_INVALID_PARAMETER -- slaveAddress or pWriteBuffer or writeLengthInBytes or pReadBuffer or ReadLengthInBytes is an unexpected value
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_TO_UART_API HID_UART_STATUS WINAPI
CP2114_I2cReadData(
	_In_ _Pre_defensive_ const HID_UART_DEVICE device,
	_In_ _Pre_defensive_ const BYTE slaveAddress,
	_In_reads_bytes_(WriteLengthInBytes) _Pre_defensive_ const BYTE* pWriteBuffer,
	_In_ _Pre_defensive_ const BYTE WriteLengthInBytes,
	_Out_writes_bytes_(ReadLengthInBytes) _Pre_defensive_ BYTE* pReadBuffer,
	_In_ _Pre_defensive_ const BYTE ReadLengthInBytes
);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLAB_CP2114_H
