// HIDtoUART.cpp : Defines the entry point for the DLL application.
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SLABCP2110.h"
#include "SLABCP2114.h"
#include "HIDtoUARTVersion.h"
#include "HIDtoUART.h"
#include "UsbReports.h"
#include "DeviceList.h"
#include <cassert>

#include "silabs_utils.h"

/////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////

bool IsManufacturingDevice(const DWORD devIndex, const WORD vid, const WORD pid);
bool HidUartIndexToHidDeviceIndex(DWORD &convIndex, const WORD vid, const WORD pid);

/////////////////////////////////////////////////////////////////////////////
// Global Variables
/////////////////////////////////////////////////////////////////////////////

CDeviceList<CHIDtoUART> HidUartDeviceList;
#define DeviceList HidUartDeviceList

/////////////////////////////////////////////////////////////////////////////
// Static Prototypes
/////////////////////////////////////////////////////////////////////////////

static HID_UART_STATUS SetCustomShortString(CHIDtoUART* device, BYTE rID, char* ascii, BYTE strlen);
static HID_UART_STATUS GetCustomShortString(CHIDtoUART* device, BYTE rID, char* ascii, BYTE* strlen);
static HID_UART_STATUS SetCustomLongString(CHIDtoUART* device, BYTE rID_1, BYTE rID_2, char* ascii, BYTE strlen);
static HID_UART_STATUS GetCustomLongString(CHIDtoUART* device, BYTE rID_1, BYTE rID_2, char* ascii, BYTE* strlen);

/////////////////////////////////////////////////////////////////////////////
// Feature Report Prototypes
/////////////////////////////////////////////////////////////////////////////

static HID_UART_STATUS SetUartEnable(CHIDtoUART* device, BOOL enable);
static HID_UART_STATUS GetUartEnable(CHIDtoUART* device, BOOL* enable);

static HID_UART_STATUS SetFlushBuffers(CHIDtoUART* device, BOOL flushTransmit, BOOL flushReceive);

static HID_UART_STATUS GetUartStatus(CHIDtoUART* device, WORD* transmitFifoSize, WORD* receiveFifoSize, BYTE* errorStatus, BYTE* lineBreakStatus);

static HID_UART_STATUS SetStartBreak(CHIDtoUART* device, _In_ const BYTE duration);
static HID_UART_STATUS SetStopBreak(CHIDtoUART* device);

static HID_UART_STATUS SetReset(CHIDtoUART* device);

_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
static HID_UART_STATUS GetPartNumber(CHIDtoUART* device, _Out_writes_bytes_(1) BYTE* partNumber, _Out_writes_bytes_(1) BYTE* version);

static HID_UART_STATUS SetLock(CHIDtoUART* device, WORD lock);
static HID_UART_STATUS GetLock(CHIDtoUART* device, WORD* lock);

_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
static HID_UART_STATUS SetUsbConfig(CHIDtoUART* device, _In_ const WORD vid, _In_ const WORD pid, BYTE power, _In_ const BYTE powerMode, _In_ const WORD releaseVersion, _In_ const BYTE flushBuffers, _In_ const BYTE mask);
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
static HID_UART_STATUS GetUsbConfig(_In_ const CHIDtoUART* device, _Out_writes_bytes_(2) WORD* pVid, _Out_writes_bytes_(2) WORD* pPid, _Out_writes_bytes_(1) BYTE* pPower, _Out_writes_bytes_(1) BYTE* pPowerMode, _Out_writes_bytes_(2) WORD* pReleaseVersion, _Out_writes_bytes_(1) BYTE* pFlushBuffers);

static HID_UART_STATUS SetManufacturingString(CHIDtoUART* device, char* manufacturingString, BYTE strlen);
static HID_UART_STATUS GetManufacturingString(CHIDtoUART* device, char* manufacturingString, BYTE* strlen);

static HID_UART_STATUS SetProductString(CHIDtoUART* device, char* productString, BYTE strlen);
static HID_UART_STATUS GetProductString(CHIDtoUART* device, char* productString, BYTE* strlen);

static HID_UART_STATUS SetSerialString(CHIDtoUART* device, char* serialString, BYTE strlen);
static HID_UART_STATUS GetSerialString(CHIDtoUART* device, char* serialString, BYTE* strlen);

/////////////////////////////////////////////////////////////////////////////
// Static Functions
/////////////////////////////////////////////////////////////////////////////

// This should exit once enough data were read or timeout expired, whatever happens first
// May read less than requested. The caller must check and call again if not enough.

static bool QueueReadData(CHIDtoUART* hidUart, DWORD requiredQueueSize, DWORD readTimeout)
{
	assert( requiredQueueSize > hidUart->queue.Size());
	const DWORD numBytesToRead = requiredQueueSize - hidUart->queue.Size();

	const WORD  RepSz     = HidDevice_GetInputReportBufferLength(hidUart->hid);
	const DWORD MaxRepCnt = HidDevice_GetMaxReportRequest(hidUart->hid);

    // Calculate how many reports to request. The result is just a rough approximation. First, it counts the
    // first byte in the report as data, second, it ignores the fact that reports may come partially filled.
    // So the calculation will usually yield less reports than necessary, except when just a few bytes are
    // required to fill the queue to requiredQueueSize. That's fine, the caller will just call again. The only
    // thing to be avoided like a plague is asking for more reports than necessary, because then HidDevice may
    // not return until the timeout expires, even though enough data have been received - this will generate bug reports.
	DWORD numReports = numBytesToRead / RepSz;
	if( numBytesToRead % RepSz)
	{
		numReports++;
	}
	if( numReports > MaxRepCnt)
	{
		numReports = MaxRepCnt;
	}

	DWORD	CbTmpRepBuf	= numReports * RepSz;
	BYTE*	tmpRepBuf	= new BYTE[CbTmpRepBuf];

	DWORD	bytesRead	= 0;
	BYTE	hidStatus	= HidDevice_GetInputReport_Interrupt_WithTimeout(hidUart->hid, tmpRepBuf, CbTmpRepBuf, (WORD)numReports, &bytesRead, readTimeout);

	const bool Success = (hidStatus == HID_DEVICE_SUCCESS || hidStatus == HID_DEVICE_TRANSFER_TIMEOUT);
	if (Success)
	{
		// Translate data from the array of equally-sized reports (each of which may be partially filled)
		// into contiguous actual data in hidUart->queue
		for (DWORD i = 0; i < bytesRead; i += HidDevice_GetInputReportBufferLength(hidUart->hid))
		{
			DWORD dataLen = tmpRepBuf[i]; // The first byte in the report is the data length

			// Throw the report away if the dataLen is erroneous
            // XXX - shouldn't we also do that if it's above HidDevice_GetInputReportBufferLength-1?
			if ((i + dataLen) < bytesRead)
			{
				// Enqueue the actual UART data
				hidUart->queue.Enqueue(&tmpRepBuf[i + 1], dataLen);
			}
		}
	}
	delete [] tmpRepBuf;
	return Success;
}

// Set feature report for single report string customization
HID_UART_STATUS SetCustomShortString(CHIDtoUART* device, BYTE rID, char* ascii, BYTE strlen)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	report[0] = rID;				// Report ID
	report[1] = (strlen * 2) + 2;	// String Desc Length
	report[2] = 0x03;				// String Desc ID

	// Convert ASCII to Unicode string
	for (int i = 0; i < strlen; i++)
	{
		report[i*2+3] = ascii[i];
	}

	// Set feature report to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_UART_SUCCESS;
	}

	return status;
}

// Get feature report for single report string customization
HID_UART_STATUS GetCustomShortString(CHIDtoUART* device, const BYTE rID, char* ascii, BYTE* strlen)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = rID;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		// Get string length from report excluding
		// the report ID and 0x03 string descriptor
		// constant
		*strlen = (report[1] - 2) / 2;

		// Allocate a blank full-size unicode string buffer
		BYTE unicode[SHORT_STRING_LEN];
		memset(unicode, 0x00, sizeof(unicode));

		// Retrieve Unicode serial string from the device
		memcpy(unicode, &report[3], SHORT_STRING_LEN);

		// Convert Unicode to ASCII
		for (int i = 0; i < (SHORT_STRING_LEN/2); i++)
		{
			ascii[i] = unicode[i * 2];
		}

		status = HID_UART_SUCCESS;
	}

	return status;
}

// Set feature report for two-report string customization
HID_UART_STATUS SetCustomLongString(CHIDtoUART* device, BYTE rID_1, BYTE rID_2, char* ascii, BYTE strlen)
{
	HID_UART_STATUS status = HID_UART_SUCCESS;

	// Allocate a blank full-size unicode string buffer
	BYTE unicode[LONG_STRING_LEN_1 + LONG_STRING_LEN_2];
	memset(unicode, 0x00, sizeof(unicode));

	// Convert ASCII to Unicode
	for (int i = 0; i < strlen; i++)
	{
		unicode[i*2] = ascii[i];
	}

	// Break the string into two reports
	BYTE report1[FEATURE_REPORT_LENGTH];
	BYTE report2[FEATURE_REPORT_LENGTH];

	report1[0] = rID_1;														// Report ID
	report1[1] = (strlen * 2) + 2;											// String Desc Length
	report1[2] = 0x03;														// String Desc ID 
	memcpy(&report1[3], &unicode[0], LONG_STRING_LEN_1);					// String Part 1
	
	report2[0] = rID_2;														// Report ID
	memcpy(&report2[1], &unicode[LONG_STRING_LEN_1], LONG_STRING_LEN_2);	// String Part 2

	// Set feature report1 to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report1, FEATURE_REPORT_LENGTH) != HID_DEVICE_SUCCESS)
	{
		status = HID_UART_DEVICE_IO_FAILED;
	}

	// Set feature report2 to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report2, FEATURE_REPORT_LENGTH) != HID_DEVICE_SUCCESS)
	{
		status = HID_UART_DEVICE_IO_FAILED;
	}

	return status;
}

// Get feature report for two-report string customization
HID_UART_STATUS GetCustomLongString(CHIDtoUART* device, BYTE rID_1, BYTE rID_2, char* ascii, BYTE* strlen)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report1[FEATURE_REPORT_LENGTH];
	memset(report1, 0x00, sizeof(report1));

	BYTE report2[FEATURE_REPORT_LENGTH];
	memset(report2, 0x00, sizeof(report2));

	// Report ID
	report1[0] = rID_1;
	report2[0] = rID_2;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report1, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS &&
		HidDevice_GetFeatureReport_Control(device->hid, report2, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		// Get string length from first report
		// excluding the report ID and 0x03 string descriptor
		// constant
		*strlen = (report1[1] - 2) / 2;

		// Allocate a blank full-size unicode string buffer
		BYTE unicode[LONG_STRING_LEN_1 + LONG_STRING_LEN_2];
		memset(unicode, 0x00, sizeof(unicode));

		// Retrieve Unicode string from the device
		memcpy(&((BYTE*)unicode)[0],					&report1[3],	LONG_STRING_LEN_1);
		memcpy(&((BYTE*)unicode)[LONG_STRING_LEN_1],	&report2[1],	LONG_STRING_LEN_2);

		// Convert Unicode to ASCII
		for (int i = 0; i < ((LONG_STRING_LEN_1 + LONG_STRING_LEN_2)/2); i++)
		{
			ascii[i] = unicode[i * 2];
		}

		status = HID_UART_SUCCESS;
	}
	else
	{
		// Failed return empty string
		*strlen = 0;
	}

	return status;
}

/////////////////////////////////////////////////////////////////////////////
// Feature Report Functions
/////////////////////////////////////////////////////////////////////////////

// Set feature report for HidUart_SetUartEnable()
HID_UART_STATUS SetUartEnable(CHIDtoUART* device, BOOL enable)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE enableMask;

	if (enable)
	{
		enableMask = 0x01;
	}
	else
	{
		enableMask = 0x00;
	}

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Prepare feature report
	report[0] = GETSET_UART_ENABLE;		// Report ID
	report[1] = enableMask;				// Enable option

	// Send feature report to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_UART_SUCCESS;
	}

	return status;
}

// Get feature report for HidUart_GetUartEnable()
HID_UART_STATUS GetUartEnable(CHIDtoUART* device, BOOL* enable)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GETSET_UART_ENABLE;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		// Extract the UART enable status from the report
		*enable = report[1];

		status = HID_UART_SUCCESS;
	}

	return status;
}

// Set feature report for HidUart_FlushBuffers()
HID_UART_STATUS SetFlushBuffers(CHIDtoUART* device, BOOL flushTransmit, BOOL flushReceive)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE flushMask = 0x00;

	if (flushTransmit)
		flushMask |= PURGE_TRANSMIT_MASK;
	if (flushReceive)
		flushMask |= PURGE_RECEIVE_MASK;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Prepare feature report
	report[0] = PURGE_FIFOS;	// Report ID
	report[1] = flushMask;		// Flush options

	// Send feature report to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_UART_SUCCESS;
	}

	return status;
}

// Get feature report for HidUart_GetUartStatus()
HID_UART_STATUS GetUartStatus(CHIDtoUART* device, WORD* transmitFifoSize, WORD* receiveFifoSize, BYTE* errorStatus, BYTE* lineBreakStatus)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GET_UART_STATUS;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		// Extract the UART settings from the report
		*transmitFifoSize	= MAKEWORD(report[2], report[1]);
		*receiveFifoSize	= MAKEWORD(report[4], report[3]);
		*errorStatus		= report[5];
		*lineBreakStatus	= report[6];

		status = HID_UART_SUCCESS;
	}

	return status;
}

// Set feature report for HidUart_SetUartConfig()
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
HID_UART_STATUS SetUartConfig(_In_ CHIDtoUART* device, _In_ const DWORD baudRate, _In_ const BYTE dataBits, _In_ const BYTE parity, _In_ const BYTE stopBits, _In_ const BYTE flowControl)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Prepare feature report
	report[0] = GETSET_UART_CONFIG;		// Report ID
	report[1] = (BYTE)(baudRate >> 24);	// Baud rate (big-endian)
	report[2] = (BYTE)(baudRate >> 16);
	report[3] = (BYTE)(baudRate >> 8);
	report[4] = (BYTE)(baudRate);
	report[5] = parity;					// Parity
	report[6] = flowControl;			// Flow Control
	report[7] = dataBits;				// Data Bits
	report[8] = stopBits;				// Stop Bits

	// Send feature report to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_UART_SUCCESS;
	}

	return status;
}

// Get feature report for HidUart_GetUartConfig()
HID_UART_STATUS GetUartConfig(CHIDtoUART* device, DWORD* baudRate, BYTE* dataBits, BYTE* parity, BYTE* stopBits, BYTE* flowControl)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GETSET_UART_CONFIG;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		*baudRate		= ((DWORD)report[1] << 24) | ((DWORD)report[2] << 16) | ((DWORD)report[3] << 8) | ((DWORD)report[4]);
		*parity			= report[5];
		*flowControl	= report[6];
		*dataBits		= report[7];
		*stopBits		= report[8];

		status = HID_UART_SUCCESS;
	}

	return status;
}

// Set feature report for HidUart_StartBreak()
_Use_decl_annotations_
HID_UART_STATUS SetStartBreak(CHIDtoUART* device, const BYTE DurationInMilliseconds)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Prepare feature report
	report[0] = TRANS_LINE_BREAK;	// Report ID
	report[1] = DurationInMilliseconds;			// Line break duration

	// Send feature report to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_UART_SUCCESS;
	}

	return status;
}

// Set feature report for HidUart_StopBreak()
HID_UART_STATUS SetStopBreak(CHIDtoUART* device)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Prepare feature report
	report[0] = STOP_LINE_BREAK;	// Report ID
	report[1] = 0x00;				// Unused byte

	// Send feature report to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_UART_SUCCESS;
	}

	return status;
}

// Set feature report for HidUart_Reset()
HID_UART_STATUS SetReset(CHIDtoUART* device)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = RESET_DEVICE;
	report[1] = 0x00;			// Currently unused (for re-enumeration)

	// Send feature report to the device
	BYTE hidStatus = HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH);

	// The device may reset too fast to respond successfully
	if (hidStatus == HID_DEVICE_SUCCESS || hidStatus == HID_DEVICE_TRANSFER_FAILED)
	//if (hidStatus == HID_DEVICE_SUCCESS)
	{
		status = HID_UART_SUCCESS;
	}

	return status;
}

// Get feature report for HidUart_ReadLatch()
HID_UART_STATUS GetReadLatch(CHIDtoUART* device, WORD* pLatchValue)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GET_GPIO_VALUES;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		// Extract the GPIO latch values
		if (pLatchValue) *pLatchValue = MAKEWORD(report[2], report[1]);

		status = HID_UART_SUCCESS;
	}

	return status;
}

// Set feature report for HidUart_WriteLatch()
_Check_return_
_Ret_range_(HID_UART_SUCCESS, HID_UART_UNKNOWN_ERROR)
_Success_(return == HID_UART_SUCCESS)
static
HID_UART_STATUS SetWriteLatch(_In_ CHIDtoUART* device, _In_ const WORD LatchValue, _In_ const WORD LatchMask)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = SET_GPIO_VALUES;
	report[1] = HIBYTE(LatchValue);
	report[2] = LOBYTE(LatchValue);
	report[3] = HIBYTE(LatchMask);
	report[4] = LOBYTE(LatchMask);

	// Set feature report to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_UART_SUCCESS;
	}

	return status;
}

// Get feature report for HidUart_GetPartNumber()
_Use_decl_annotations_
HID_UART_STATUS GetPartNumber(CHIDtoUART* device, BYTE* pPartNumber, BYTE* pVersion)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GET_VER_INFO;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		// Retrieve version information from the device
		if (pPartNumber) *pPartNumber = report[1];
		if (pVersion) *pVersion = report[2];

		status = HID_UART_SUCCESS;
	}

	return status;
}

// Set feature report for HidUart_SetLock()
HID_UART_STATUS SetLock(CHIDtoUART* device, WORD lock)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GETSET_LOCK_BYTE;	// Report ID
	report[1] = HIBYTE(lock);		// Lock bytes
	report[2] = LOBYTE(lock);

	// Set feature report to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_UART_SUCCESS;
	}

	return status;
}

// Get feature report for HidUart_GetLock()
HID_UART_STATUS GetLock(CHIDtoUART* device, WORD* lock)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GETSET_LOCK_BYTE;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		// Retrieve lock bytes from the device
		*lock = MAKEWORD(report[2], report[1]);

		status = HID_UART_SUCCESS;
	}

	return status;
}

// Set feature report for HidUart_SetUsbConfig()
_Use_decl_annotations_
HID_UART_STATUS SetUsbConfig(CHIDtoUART* device, WORD vid, WORD pid, BYTE power, BYTE powerMode, WORD releaseVersion, BYTE flushBuffers, BYTE mask)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GETSET_USB_CONFIG;			// Report ID
	report[1] = LOBYTE(vid);				// VID (Little-endian)
	report[2] = HIBYTE(vid);
	report[3] = LOBYTE(pid);				// PID (Little-endian)
	report[4] = HIBYTE(pid);
	report[5] = power;						// Power (mA/2)
	report[6] = powerMode;					// Power Mode (0: bus, 1: self (VREG off), 2: self (VREG on))
	report[7] = HIBYTE(releaseVersion);		// Release Version (major.minor)
	report[8] = LOBYTE(releaseVersion);
	report[9] = flushBuffers;				// Flush buffers mask
	report[10] = mask;						// Field mask

	// Set feature report to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_UART_SUCCESS;
	}

	return status;
}

// Get feature report for HidUart_GetUsbConfig()
_Use_decl_annotations_
HID_UART_STATUS GetUsbConfig(const CHIDtoUART* device, WORD* pVid, WORD* pPid, BYTE* pPower, BYTE* pPowerMode, WORD* pReleaseVersion, BYTE* pFlushBuffers)
{
	HID_UART_STATUS status = HID_UART_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GETSET_USB_CONFIG;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		// Retrieve USB customization from the device
		if (pVid) *pVid = MAKEWORD(report[1], report[2]);	// VID (Little-endian)
		if (pPid) *pPid = MAKEWORD(report[3], report[4]);	// PID (Little-endian)
		if (pPower) *pPower					= report[5];						// Power (mA/2)
		if (pPowerMode) *pPowerMode			= report[6];						// Power Mode (0: bus, 1: self (VREG off), 2: self (VREG on))
		if (pReleaseVersion) *pReleaseVersion	= MAKEWORD(report[8], report[7]);	// Release Version (major.minor)
		if (pFlushBuffers) *pFlushBuffers = report[9];						// Flush buffers mask

		status = HID_UART_SUCCESS;
	}

	return status;
}

// Set feature report for HidUart_SetManufacturingString()
HID_UART_STATUS SetManufacturingString(CHIDtoUART* device, char* manufacturingString, BYTE strlen)
{
	HID_UART_STATUS status = SetCustomLongString(device, GETSET_MANU1, GETSET_MANU2, manufacturingString, strlen);
	return status;
}

// Get feature report for HidUart_GetManufacturingString()
HID_UART_STATUS GetManufacturingString(CHIDtoUART* device, char* manufacturingString, BYTE* strlen)
{
	HID_UART_STATUS status = GetCustomLongString(device, GETSET_MANU1, GETSET_MANU2, manufacturingString, strlen);
	return status;
}

// Set feature report for HidUart_SetProductString()
HID_UART_STATUS SetProductString(CHIDtoUART* device, char* productString, BYTE strlen)
{
	HID_UART_STATUS status = SetCustomLongString(device, GETSET_PROD1, GETSET_PROD2, productString, strlen);
	return status;
}

// Get feature report for HidUart_GetProductString()
HID_UART_STATUS GetProductString(CHIDtoUART* device, char* productString, BYTE* strlen)
{
	HID_UART_STATUS status = GetCustomLongString(device, GETSET_PROD1, GETSET_PROD2, productString, strlen);
	return status;
}

// Set feature report for HidUart_SetSerialString()
HID_UART_STATUS SetSerialString(CHIDtoUART* device, char* serialString, BYTE strlen)
{
	HID_UART_STATUS status = SetCustomShortString(device, GETSET_SERSTR, serialString, strlen);
	return status;
}

// Get feature report for HidUart_GetSerialString()
HID_UART_STATUS GetSerialString(CHIDtoUART* device, char* serialString, BYTE* strlen)
{
	HID_UART_STATUS status = GetCustomShortString(device, GETSET_SERSTR, serialString, strlen);
	return status;
}

/////////////////////////////////////////////////////////////////////////////
// Exported Library Functions
/////////////////////////////////////////////////////////////////////////////

// HidUart_GetNumDevices
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetNumDevices(DWORD* pNumDevices, const WORD vid, const WORD pid)
{
	HID_UART_STATUS status = HID_UART_DEVICE_NOT_FOUND;

	if (SiLabs_Utils_IsValidParam(pNumDevices))
	{
		const DWORD HidCnt = HidDevice_GetNumHidDevices(vid, pid);
		DWORD numDevices = 0;

		for (DWORD index = 0; index < HidCnt; index++)
		{
			if (IsManufacturingDevice(index, vid, pid))
			{
				numDevices++;
			}
		}
		*pNumDevices = numDevices;
		status = HID_UART_SUCCESS;
	}
	else
	{
		status = HID_UART_INVALID_PARAMETER;
	}

	return status;
}

// Translate HID status to HID_UART status
static
HID_UART_STATUS
_HidUart_TranslateHIDStatus_To_HID_UARTStatus(_In_ const SLAB_HID_DEVICE_STATUS HIDStatus)
{
	HID_UART_STATUS HID_UARTStatus;

	switch (HIDStatus)
	{
	default:
		HID_UARTStatus = HID_UART_DEVICE_ACCESS_ERROR;
		break;

	case HID_DEVICE_SUCCESS:
		HID_UARTStatus = HID_UART_SUCCESS;
		break;

	case HID_DEVICE_HANDLE_ERROR:
		HID_UARTStatus = HID_UART_INVALID_HANDLE;
		break;

	case HID_DEVICE_NOT_FOUND:
		HID_UARTStatus = HID_UART_DEVICE_NOT_FOUND;
		break;
	case HID_DEVICE_ALREADY_OPENED:
		HID_UARTStatus = HID_UART_DEVICE_ALREADY_OPENED;
		break;

	case HID_DEVICE_INVALID_BUFFER_SIZE:
		HID_UARTStatus = HID_UART_INVALID_PARAMETER;
		break;
	}

	return HID_UARTStatus;
}

// HidUart_GetString
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetString(const DWORD deviceNum, const WORD vid, const WORD pid, char* deviceString, const DWORD options)
{
	DWORD deviceIndex = deviceNum;

	if (!HidUartIndexToHidDeviceIndex(deviceIndex, vid, pid))
	{
        return HID_UART_DEVICE_NOT_FOUND;
    }

	HID_UART_STATUS status = HID_UART_UNKNOWN_ERROR;

	// Check parameters
	if (deviceString &&
		options >= HID_UART_GET_VID_STR && options <= HID_UART_GET_PRODUCT_STR)
	{
		// Get the product string
		BYTE hidStatus = HidDevice_GetHidString(deviceIndex, vid, pid, (BYTE)options, deviceString, HID_UART_DEVICE_STRLEN);

		// Translate HID status to HID_UART status
		status = _HidUart_TranslateHIDStatus_To_HID_UARTStatus(hidStatus);
	}
	else
	{
		status = HID_UART_INVALID_PARAMETER;
	}

	return status;
}

// HidUart_GetOpenedString
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetOpenedString(HID_UART_DEVICE device, char* deviceString, const DWORD options)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check parameters
		if (deviceString &&
			options >= HID_UART_GET_VID_STR && options <= HID_UART_GET_PRODUCT_STR)
		{
			// Get the product string
			BYTE hidStatus = HidDevice_GetString(hidUart->hid, (BYTE)options, deviceString, HID_UART_DEVICE_STRLEN);

			// Translate HID status to HID_UART status
			status = _HidUart_TranslateHIDStatus_To_HID_UARTStatus(hidStatus);
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_GetIndexedString
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetIndexedString(const DWORD deviceNum, const WORD vid, const WORD pid, const DWORD stringIndex, char* deviceString)
{
	DWORD deviceIndex = deviceNum;

	if (!HidUartIndexToHidDeviceIndex(deviceIndex, vid, pid))
	{
        return HID_UART_DEVICE_NOT_FOUND;
    }

	HID_UART_STATUS status = HID_UART_UNKNOWN_ERROR;

	// Check parameters
	if (deviceString)
	{
		// Get the indexed string
		BYTE hidStatus = HidDevice_GetHidIndexedString(deviceIndex, vid, pid, stringIndex, deviceString, HID_UART_DEVICE_STRLEN);

		// Translate HID status to HID_UART status
		status = _HidUart_TranslateHIDStatus_To_HID_UARTStatus(hidStatus);
	}
	else
	{
		status = HID_UART_INVALID_PARAMETER;
	}

	return status;
}

// HidUart_GetOpenedIndexedString
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetOpenedIndexedString(HID_UART_DEVICE device, const DWORD stringIndex, char* deviceString)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check parameters
		if (deviceString)
		{
			// Get the indexed string
			BYTE hidStatus = HidDevice_GetIndexedString(hidUart->hid, stringIndex, deviceString, HID_UART_DEVICE_STRLEN);

			// Translate HID status to HID_UART status
			status = _HidUart_TranslateHIDStatus_To_HID_UARTStatus(hidStatus);
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_GetAttributes
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetAttributes(const DWORD deviceNum, const WORD vid, const WORD pid, WORD* deviceVid, WORD* devicePid, WORD* deviceReleaseNumber)
{
	DWORD deviceIndex = deviceNum;

	if (!HidUartIndexToHidDeviceIndex(deviceIndex, vid, pid))
	{
        return HID_UART_DEVICE_NOT_FOUND;
    }

	HID_UART_STATUS status = HID_UART_UNKNOWN_ERROR;

	// Check parameters
	if (deviceVid && devicePid && deviceReleaseNumber)
	{
		// Get the attributes
		BYTE hidStatus = HidDevice_GetHidAttributes(deviceIndex, vid, pid, deviceVid, devicePid, deviceReleaseNumber);

		// Translate HID status to HID_UART status
		status = _HidUart_TranslateHIDStatus_To_HID_UARTStatus(hidStatus);
	}
	else
	{
		status = HID_UART_INVALID_PARAMETER;
	}

	return status;
}

// HidUart_GetOpenedAttributes
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetOpenedAttributes(HID_UART_DEVICE device, WORD* pDeviceVid, WORD* pDevicePid, WORD* pDeviceReleaseNumber)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check parameters
		if (pDeviceVid && pDevicePid && pDeviceReleaseNumber)
		{
			// Get the attributes
			BYTE hidStatus = HidDevice_GetAttributes(hidUart->hid, pDeviceVid, pDevicePid, pDeviceReleaseNumber);

			// Translate HID status to HID_UART status
			status = _HidUart_TranslateHIDStatus_To_HID_UARTStatus(hidStatus);
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_Open
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_Open(HID_UART_DEVICE* pdevice, const DWORD deviceNum, const WORD vid, const WORD pid)
{
	DWORD deviceIndex = deviceNum;

	if (!HidUartIndexToHidDeviceIndex(deviceIndex, vid, pid))
	{
        return HID_UART_DEVICE_NOT_FOUND;
    }

    if (!pdevice)
    {
        return HID_UART_INVALID_PARAMETER;
    }

    // Create a new device object and add it to the device list
    CHIDtoUART* hidUart = DeviceList.Construct();

    // Check device object
    if (!DeviceList.Validate(hidUart))
    {
        return HID_UART_INVALID_DEVICE_OBJECT;
    }

    HID_UART_STATUS status = HID_UART_UNKNOWN_ERROR;

    // Open the device by index (use max input report buffer)
    BYTE hidStatus = HidDevice_Open(&hidUart->hid, deviceIndex, vid, pid, MAX_REPORT_REQUEST_XP);

    // Openned successfully
    if (hidStatus == HID_DEVICE_SUCCESS)
    {
		BYTE partNumber;
		BYTE version;

		// Get part number for future use
        status = GetPartNumber(hidUart, &partNumber, &version);

        if (status == HID_UART_SUCCESS)
        {
            // Only support CP2110 and CP2114
            if (partNumber == HID_UART_PART_CP2110 ||
                    partNumber == HID_UART_PART_CP2114)
            {
                // Save part number
                // Used to determine device functionality
                hidUart->partNumber = partNumber;

                // Automatically enable the UART on open
                status = SetUartEnable(hidUart, TRUE);

                // For CP2114, also issue the GET_DEVICE_VERSIONS report to get firmware and config format versions
                if ((status == HID_UART_SUCCESS) && (partNumber == HID_UART_PART_CP2114))
                {
                	BYTE apiVersion, firmwareVersion, configFormat;
                	status = CP2114_GetVersions(hidUart, &apiVersion, &firmwareVersion, &configFormat);
                	if (status == HID_UART_SUCCESS)
                	{
                		hidUart->cp2114DeviceApiVersion = apiVersion;
                		hidUart->cp2114FirmwareVersion = firmwareVersion;
                		hidUart->cp2114ConfigFormat = configFormat;
                	}
                }
            }
            else
            {
                status = HID_UART_DEVICE_NOT_SUPPORTED;
            }
        }
        if (status != HID_UART_SUCCESS)
        {
            HidDevice_Close(hidUart->hid);
        }
    }
    else if (hidStatus == HID_DEVICE_NOT_FOUND)
    {
        status = HID_UART_DEVICE_NOT_FOUND; // Failed to open device because the device was not found
    }
    else
    {
        status = HID_UART_DEVICE_ACCESS_ERROR; // Could not access device (i.e. already opened)
    }

    // Device opened and initialized successfully
    if (status == HID_UART_SUCCESS)
    {
        // Read timeout doesn't matter since we specify it explicitly on every read.
        // Set interrupt write timeouts to the default write timeout (1000 ms)
        HidDevice_SetTimeouts(hidUart->hid, 0 /* read TO */, hidUart->writeTimeout);

        // Return the device object pointer to the user
        *pdevice = hidUart;
    }
    else
    {
        // Delete the device object and
        // remove the device reference from the device list
        DeviceList.Destruct(hidUart);
    }

    return status;
}

// HidUart_Close
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_Close(HID_UART_DEVICE device)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Close the device
		BYTE hidStatus = HidDevice_Close(hidUart->hid);

		status = _HidUart_TranslateHIDStatus_To_HID_UARTStatus(hidStatus);

		// Deallocate the device object, remove the device reference
		// from the device list
		DeviceList.Destruct(hidUart);
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_IsOpened
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_IsOpened(const HID_UART_DEVICE device, BOOL* pbIsOpened)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointers
		if (pbIsOpened)
		{
			// Check if the device is opened
			*pbIsOpened = HidDevice_IsOpened(hidUart->hid);

			status = HID_UART_SUCCESS;
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_SetUartEnable
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetUartEnable(HID_UART_DEVICE device, const BOOL enable)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		status = SetUartEnable(hidUart, enable);
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_GetUartEnable
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetUartEnable(HID_UART_DEVICE device, BOOL* enable)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointer
		if (enable)
		{
			status = GetUartEnable(hidUart, enable);
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_Read
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_Read(HID_UART_DEVICE device, BYTE* buffer, const DWORD numBytesToRead, DWORD* pNumBytesRead)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	if (!DeviceList.Validate(hidUart))
	{
		return HID_UART_INVALID_DEVICE_OBJECT;
	}
	if (!buffer || !pNumBytesRead)
	{
		return HID_UART_INVALID_PARAMETER;
	}

	*pNumBytesRead = 0;

	if (numBytesToRead < HID_UART_MIN_READ_SIZE || numBytesToRead > HID_UART_MAX_READ_SIZE)
	{
		return HID_UART_INVALID_REQUEST_LENGTH;
	}

	// Get the start time to implement read timeouts
	ULONGLONG startTime   = SiLabs_Utils_GetStartTime();
	DWORD elapsTime   = 0;
	DWORD readTimeout = hidUart->readTimeout;

	// Keep calling QueueReadData until enough data received or time is up.
	do
	{
		// Check if there is enough data to satisfy the read
		if (hidUart->queue.Size() >= numBytesToRead)
		{
			break;
		}

		// Read the max number of interrupt reports (HID read timeout set to 1 ms),
		// Parse data out of each report
		// Add contiguous UART data to hidUart->queue
		if (!QueueReadData(hidUart, numBytesToRead, readTimeout - elapsTime))
		{
			status = HID_UART_READ_ERROR;
			break;
		}

		elapsTime = (DWORD) SiLabs_Utils_ElapsedTimeSince(startTime);
	} while (readTimeout > elapsTime);

	if (status != HID_UART_READ_ERROR)
	{
		// Copy requested data to user buffer
		hidUart->queue.Dequeue((BYTE*)buffer, numBytesToRead, pNumBytesRead);

		if (*pNumBytesRead < numBytesToRead)
		{
			// The loop ended because enough time passed.
			// Still the caller will get whatever data was received.
			status = HID_UART_READ_TIMED_OUT;
		}
		else
		{
			// The loop ended because enough data was received
			status = HID_UART_SUCCESS;
		}
	}

	return status;
}

// HidUart_Write
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_Write(HID_UART_DEVICE device, BYTE* buffer, const DWORD numBytesToWrite, DWORD* numBytesWritten)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointers
		if (buffer && numBytesWritten)
		{
			*numBytesWritten = 0;

			// Check parameter value
			if (numBytesToWrite >= HID_UART_MIN_WRITE_SIZE &&
				numBytesToWrite <= HID_UART_MAX_WRITE_SIZE)
			{
				// Interrupt report buffer
				BYTE report[INTERRUPT_REPORT_LENGTH];

				// Keep track of the number of bytes written so far
				DWORD bytesWritten = 0;

				// Get the starting time for write timeout
				ULONGLONG startTime = SiLabs_Utils_GetStartTime();

				// Break the write up into 63 byte interrupt reports
				// while-loop can exit with status:
				// HID_UART_UNKNOWN_ERROR
				// HID_UART_WRITE_ERROR
				while (SiLabs_Utils_ElapsedTimeSince(startTime) < hidUart->writeTimeout)
				{
					// Check if all bytes have been written
					if (bytesWritten == numBytesToWrite)
					{
						break;
					}

					// Prepare the interrupt report buffer for the next write
					DWORD reportLen = numBytesToWrite - bytesWritten;
					if( reportLen > 63)
                                        {
                                            reportLen = 63;
                                        }


					memcpy(report + 1, (BYTE*)buffer + bytesWritten, reportLen);
					report[0] = (BYTE)reportLen;

					// Send the interrupt report (HID write timeout set to writeTimeout)
					BYTE hidStatus = HidDevice_SetOutputReport_Interrupt(hidUart->hid, report, INTERRUPT_REPORT_LENGTH);

					if (hidStatus == HID_DEVICE_SUCCESS)
					{
						// Update number of bytes written
						bytesWritten += reportLen;
					}
					// Check for write error (a timeout in this case is
					// also considered a write error)
					else
					{
						status = HID_UART_WRITE_ERROR;
						break;
					}
				}

				// Return the number of bytes written
				*numBytesWritten = bytesWritten;

				if (status != HID_UART_WRITE_ERROR)
				{
					// Check if the write operation succeeded or timed out
					if (bytesWritten < numBytesToWrite)
					{
						status = HID_UART_WRITE_TIMED_OUT;
					}
					else
					{
						status = HID_UART_SUCCESS;
					}
				}
			}
			else
			{
				status = HID_UART_INVALID_REQUEST_LENGTH;
			}
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_FlushBuffers
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_FlushBuffers(HID_UART_DEVICE device, const BOOL bFlushTransmit, const BOOL bFlushReceive)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Send flush buffers feature report
		if (SetFlushBuffers(hidUart, bFlushTransmit, bFlushReceive) == HID_UART_SUCCESS)
		{
			if (bFlushReceive)
			{
				// Flush the HID input report buffer
				(void) HidDevice_FlushBuffers(hidUart->hid);

				// Flush the internal UART read queue
				hidUart->queue.Clear();
			}

			status = HID_UART_SUCCESS;
		}
		else
		{
			status = HID_UART_DEVICE_IO_FAILED;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_CancelIo
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_CancelIo(HID_UART_DEVICE device)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Cancel pending reads/writes issued in this thread
		if (HidDevice_CancelIo(hidUart->hid))
		{
			status = HID_UART_SUCCESS;
		}
		else
		{
			status = HID_UART_DEVICE_IO_FAILED;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_SetTimeouts
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetTimeouts(HID_UART_DEVICE device, const DWORD ReadTimeoutInMilliseconds, const DWORD WriteTimeoutInMilliseconds)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Set HidUart_Read() and HidUart_Write() timeouts
		// These are the overall read and write timeouts
		hidUart->readTimeout	= ReadTimeoutInMilliseconds;
		hidUart->writeTimeout	= WriteTimeoutInMilliseconds;

		// Read timeout doesn't matter since we specify it explicitly on every read.
		// Set the HID get/set interrupt report timeouts
		HidDevice_SetTimeouts(hidUart->hid, 0 /* read TO */, WriteTimeoutInMilliseconds);

		status = HID_UART_SUCCESS;
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_GetTimeouts
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetTimeouts(HID_UART_DEVICE device, DWORD* ReadTimeoutInMilliseconds, DWORD* pWriteTimeoutInMilliseconds)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointers
		if (ReadTimeoutInMilliseconds && pWriteTimeoutInMilliseconds)
		{
			// Retrieve the HidUart_Read() and HidUart_Write() timeouts
			*ReadTimeoutInMilliseconds = hidUart->readTimeout;
			*pWriteTimeoutInMilliseconds = hidUart->writeTimeout;

			status = HID_UART_SUCCESS;
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_GetUARTStatus
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetUartStatus(HID_UART_DEVICE device, WORD* transmitFifoSize, WORD* receiveFifoSize, BYTE* errorStatus, BYTE* lineBreakStatus)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointers
		if (transmitFifoSize && receiveFifoSize && errorStatus && lineBreakStatus)
		{
			// Retrieve the UART status from the device
			status = GetUartStatus(hidUart, transmitFifoSize, receiveFifoSize, errorStatus, lineBreakStatus);
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_SetUARTConfig
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetUartConfig(HID_UART_DEVICE device, const DWORD baudRate, const BYTE dataBits, const BYTE parity, const BYTE stopBits, const BYTE flowControl)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check parameters
		if (dataBits	>= HID_UART_FIVE_DATA_BITS	&& dataBits		<= HID_UART_EIGHT_DATA_BITS			&&
			parity		>= HID_UART_NO_PARITY		&& parity		<= HID_UART_SPACE_PARITY			&&
			stopBits	>= HID_UART_SHORT_STOP_BIT	&& stopBits		<= HID_UART_LONG_STOP_BIT			&&
			flowControl	>= HID_UART_NO_FLOW_CONTROL	&& flowControl	<= HID_UART_RTS_CTS_FLOW_CONTROL)
		{
			status = SetUartConfig(hidUart, baudRate, dataBits, parity, stopBits, flowControl);
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_GetUARTConfig
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetUartConfig(HID_UART_DEVICE device, DWORD* pBaudRate, BYTE* pDataBits, BYTE* pParity, BYTE* pStopBits, BYTE* pFlowControl)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointers
		if (pBaudRate && pDataBits && pParity && pStopBits && pFlowControl)
		{
			status = GetUartConfig(hidUart, pBaudRate, pDataBits, pParity, pStopBits, pFlowControl);
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_StartBreak
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_StartBreak(HID_UART_DEVICE device, const BYTE DurationInMilliseconds)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check parameter
		if (DurationInMilliseconds >= 0 && DurationInMilliseconds <= 125)
		{
			status = SetStartBreak(hidUart, DurationInMilliseconds);
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_StopBreak
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_StopBreak(HID_UART_DEVICE device)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		status = SetStopBreak(hidUart);
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_Reset
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_Reset(HID_UART_DEVICE device)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		status = SetReset(hidUart);
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_ReadLatch
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_ReadLatch(HID_UART_DEVICE device, WORD* pLatchValue)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointer
		if (pLatchValue)
		{
			status = GetReadLatch(hidUart, pLatchValue);
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_WriteLatch
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_WriteLatch(HID_UART_DEVICE device, const WORD LatchValue, const WORD LatchMask)
{
	HID_UART_STATUS		status = HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		status = SetWriteLatch(hidUart, LatchValue, LatchMask);
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_GetPartNumber
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetPartNumber(HID_UART_DEVICE device, BYTE* pPartNumber, BYTE* pVersion)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointers
		if (pPartNumber && pVersion)
		{
			status = GetPartNumber(hidUart, pPartNumber, pVersion);
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_GetLibraryVersion
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetLibraryVersion(BYTE* major, BYTE* pMinor, BOOL* pIsReleaseBuild)
{
	HID_UART_STATUS status = HID_UART_UNKNOWN_ERROR;

	// Check pointers
	if (major && pMinor && pIsReleaseBuild)
	{
		*major		= HID_UART_VERSION_MAJOR;
		if (pMinor) *pMinor = HID_UART_VERSION_MINOR;
		if (pIsReleaseBuild) *pIsReleaseBuild = HID_UART_VERSION_RELEASE;

		status = HID_UART_SUCCESS;
	}
	else
	{
		status = HID_UART_INVALID_PARAMETER;
	}

	return status;
}

// HidUart_GetHidLibraryVersion
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetHidLibraryVersion(BYTE* pMajor, BYTE* pMinor, BOOL* pIsReleaseBuild)
{
	HID_UART_STATUS status = HID_UART_UNKNOWN_ERROR;

	// Check pointers
	if (pMajor && pMinor && pIsReleaseBuild)
	{
		// Get HID DLL library version
		BYTE hidStatus = HidDevice_GetHidLibraryVersion(pMajor, pMinor, pIsReleaseBuild);

		// Translate HID status to HID_UART status
		status = _HidUart_TranslateHIDStatus_To_HID_UARTStatus(hidStatus);
	}
	else
	{
		status = HID_UART_INVALID_PARAMETER;
	}

	return status;
}

// HidUart_GetHidGuid
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetHidGuid(void* pHIDGuid)
{
	HID_UART_STATUS status = HID_UART_UNKNOWN_ERROR;

	// Check pointers
	if (pHIDGuid)
	{
		// Return the HID GUID
		HidDevice_GetHidGuid(pHIDGuid);

		status = HID_UART_SUCCESS;
	}
	else
	{
		status = HID_UART_INVALID_PARAMETER;
	}

	return status;
}

#ifdef _WIN32
// HidUart_GetHidGuidSafe
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetHidGuidSafe(GUID* pHIDGuid)
{
	return HidUart_GetHidGuid(pHIDGuid);
}
#endif	// _WIN32

// HidUart_SetLock
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetLock(HID_UART_DEVICE device, const WORD lock)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		status = SetLock(hidUart, lock);
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_GetLock
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetLock(HID_UART_DEVICE device, WORD* lock)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointers
		if (lock)
		{
			status = GetLock(hidUart, lock);
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_SetUsbConfig
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetUsbConfig(HID_UART_DEVICE device, const WORD vid, const WORD pid, const BYTE power, const BYTE powerMode, const WORD releaseVersion, const BYTE flushBuffers, const BYTE mask)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check parameters
		if (powerMode >= HID_UART_BUS_POWER && powerMode <= HID_UART_SELF_POWER_VREG_EN)
		{
			// Check power parameter if bus powered
			if (powerMode == HID_UART_BUS_POWER && power > HID_UART_BUS_POWER_MAX)
			{
				status = HID_UART_INVALID_PARAMETER;
			}
			else
			{
				status = SetUsbConfig(hidUart, vid, pid, power, powerMode, releaseVersion, flushBuffers, mask);
			}
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_GetUsbConfig
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetUsbConfig(HID_UART_DEVICE device, WORD* pVid, WORD* pPid, BYTE* pPower, BYTE* pPowerMode, WORD* pReleaseVersion, BYTE* pFlushBuffers)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointers
		if (pVid && pPid && pPower && pPowerMode && pReleaseVersion && pFlushBuffers)
		{
			status = GetUsbConfig(hidUart, pVid, pPid, pPower, pPowerMode, pReleaseVersion, pFlushBuffers);
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_SetManufacturingString
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetManufacturingString(HID_UART_DEVICE device, char* manufacturingString, const BYTE strlen)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointers
		if (manufacturingString)
		{
			// Check parameters
			if (strlen <= HID_UART_MFG_STRLEN)
			{
				status = SetManufacturingString(hidUart, manufacturingString, strlen);
			}
			else
			{
				status = HID_UART_INVALID_PARAMETER;
			}
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_GetManufacturingString
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetManufacturingString(HID_UART_DEVICE device, char* manufacturingString, BYTE* strlen)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointers
		if (manufacturingString && strlen)
		{
			status = GetManufacturingString(hidUart, manufacturingString, strlen);
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_SetProductString
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetProductString(HID_UART_DEVICE device, char* productString, const BYTE strlen)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointers
		if (productString)
		{
			// Check parameters
			if (strlen <= HID_UART_PRODUCT_STRLEN)
			{
				status = SetProductString(hidUart, productString, strlen);
			}
			else
			{
				status = HID_UART_INVALID_PARAMETER;
			}
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_GetProductString
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetProductString(HID_UART_DEVICE device, char* productString, BYTE* strlen)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointers
		if (productString && strlen)
		{
			status = GetProductString(hidUart, productString, strlen);
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_SetSerialString
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_SetSerialString(HID_UART_DEVICE device, char* serialString, const BYTE strlen)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointers
		if (serialString)
		{
			// Check parameters
			if (strlen <= HID_UART_SERIAL_STRLEN)
			{
				status = SetSerialString(hidUart, serialString, strlen);
			}
			else
			{
				status = HID_UART_INVALID_PARAMETER;
			}
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidUart_GetSerialString
_Use_decl_annotations_
HID_TO_UART_API HID_UART_STATUS WINAPI
HidUart_GetSerialString(HID_UART_DEVICE device, char* serialString, BYTE* strlen)
{
	HID_UART_STATUS		status		= HID_UART_UNKNOWN_ERROR;
	CHIDtoUART*			hidUart		= ((CHIDtoUART*)device);

	// Check device object
	if (DeviceList.Validate(hidUart))
	{
		// Check pointers
		if (serialString && strlen)
		{
			status = GetSerialString(hidUart, serialString, strlen);
		}
		else
		{
			status = HID_UART_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_UART_INVALID_DEVICE_OBJECT;
	}

	return status;
}
