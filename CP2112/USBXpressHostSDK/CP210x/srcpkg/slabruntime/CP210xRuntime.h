

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SILABS_STATUS_SUCCESS 0

typedef enum _CP210x_STATUS {
	CP210x_SUCCESS = SILABS_STATUS_SUCCESS	///< Success
	, CP210x_INVALID_HANDLE = 0x01			///< an handle parameter was not valid
	, CP210x_INVALID_PARAMETER = 0x02			///< a parameter was not valid
	, CP210x_DEVICE_IO_FAILED = 0x03	///< device I/O failed
	, CP210x_FUNCTION_NOT_SUPPORTED = 0x04			///< the specified function is not supported
	, CP210x_GLOBAL_DATA_ERROR = 0x05	///< global data error
	, CP210x_FILE_ERROR = 0x06	///< file error
	, CP210x_COMMAND_FAILED = 0x08	///< command failed
	, CP210x_INVALID_ACCESS_TYPE = 0x09	///< invalid access type

	, CP210x_DEVICE_NOT_FOUND = 0xFF			///< the specified device was not found
} CP210x_STATUS, *PCP210x_STATUS;



	
CP210x_STATUS CP210xRT_GetNumDevices(uint32_t * lpdwNumDevices);

CP210x_STATUS CP210xRT_Open(const uint32_t DeviceIndex, HANDLE ** pHandle);

CP210x_STATUS CP210xRT_Close(const HANDLE * handle);

CP210x_STATUS CP210xRT_GetReceiverMaxTimeout(const HANDLE * handle, uint16_t * pMaxTimeoutInMicroseconds);

CP210x_STATUS CP210xRT_SetReceiverMaxTimeout(const HANDLE * handle, const uint16_t maxTimeoutInMicroseconds);


#ifdef __cplusplus
}
#endif

