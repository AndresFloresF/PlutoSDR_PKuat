/////////////////////////////////////////////////////////////////////////////
// CP213xDevice.h
/////////////////////////////////////////////////////////////////////////////

#ifndef CP213x_DEVICE_H
#define CP213x_DEVICE_H

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "libusb.h"
#include "SLAB_USB_SPI.h"
#include "SLAB_USB_SPI_private.h"
#include <vector>

class CCP213xDevice;

typedef struct BulkTransferStruct {
    int waitObject;
    USB_SPI_STATUS status;
    DWORD* pLengthTransferred;
} BulkTransferStruct;

typedef struct AsyncReadThreadParam {
    CCP213xDevice* cp213xDevice;
    libusb_device_handle* hDevice;
    BOOL shouldTerminate;
    DWORD blockSize;
    DWORD totalSize;
    BOOL release;
    int  returnCode;
    BOOL userRead;
    std::vector<BYTE> buffer;
} AsyncReadThreadParam;

/////////////////////////////////////////////////////////////////////////////
// CCP213xDevice Class
/////////////////////////////////////////////////////////////////////////////

void BulkTransferCallback(struct libusb_transfer *transfer);

class CCP213xDevice
{
// Static Methods
public:
    static USB_SPI_STATUS GetNumDevices(DWORD* lpdwNumDevices, DWORD VID, DWORD PID);
    static USB_SPI_STATUS Open(DWORD dwDevice, CCP213xDevice** devObj, DWORD VID, DWORD PID);

    virtual ~CCP213xDevice();

// Public Methods
public:
    USB_SPI_STATUS Close();
    BOOL IsOpened();
    BOOL IsAsyncReadInProgress();
    void CancelAsyncRead();
    AsyncReadThreadParam* GetAsyncReadThreadParam();
    pthread_t* GetAsyncReadThread();
    void SetupAsyncReadThreadParam(DWORD blockSize, DWORD totalSize, BOOL release, int returnCode);
    void SetCurrentAsyncTransfer(libusb_transfer* transfer);
    libusb_transfer* GetCurrentAsyncTransfer();
    USB_SPI_STATUS Reset();
    HANDLE GetHandle();
    BYTE GetOutEndpointAddress();
    BYTE GetInEndpointAddress();
    DWORD GetOutEndpointMaxTransferSize();
    DWORD GetInEndpointMaxTransferSize();

    USB_SPI_STATUS GetVid(LPWORD wVid);
    USB_SPI_STATUS GetPid(LPWORD wPid);    
    USB_SPI_STATUS GetDeviceSerialNumber(LPVOID lpSerial, LPBYTE lpbLength, BOOL bConvertToASCII = true);
    USB_SPI_STATUS GetSelfPower(LPBOOL lpbSelfPower);
    USB_SPI_STATUS GetMaxPower(LPBYTE lpbMaxPower);
    USB_SPI_STATUS GetDeviceVersion(BYTE* majorVersion, BYTE* minorVersion);
    USB_SPI_STATUS GetDeviceDescriptor(PDEVICE_DESCRIPTOR pDescriptor);
    USB_SPI_STATUS GetStringDescriptor(BYTE index,BYTE stringDescriptor[STRING_DESCRIPTOR_SIZE]);
    USB_SPI_STATUS GetProductString(LPSTR lpProduct, BYTE* lpbLength, BOOL bConvertToASCII = true);
    USB_SPI_STATUS GetSerialString(LPSTR lpProduct, BYTE* lpbLength, BOOL bConvertToASCII);
        
    USB_SPI_STATUS SetVid(WORD wVid);
    USB_SPI_STATUS SetPid(WORD wPid);
    USB_SPI_STATUS SetProductString(LPVOID lpvProduct, BYTE bLength, BOOL bConvertToUnicode = true);
    USB_SPI_STATUS SetSerialNumber(LPVOID lpvSerialNumber, BYTE bLength, BOOL bConvertToUnicode = true);
    USB_SPI_STATUS SetSelfPower(BOOL bSelfPower);
    USB_SPI_STATUS SetMaxPower(BYTE bMaxPower);
    USB_SPI_STATUS SetDeviceVersion(WORD wVersion);
   
    USB_SPI_STATUS BulkTransfer(BYTE EndpointAddress, BYTE* Buffer, DWORD Length, DWORD* LengthTransferred, DWORD Timeout);
    USB_SPI_STATUS BulkTransfer_Asynch(BYTE EndpointAddress, BYTE* Buffer, DWORD Length, DWORD* LengthTransferred, DWORD Timeout, int WaitObject);
    USB_SPI_STATUS ControlTransfer(SETUP_PACKET SetupPacket, BYTE* Buffer, DWORD* LengthTransferred);

// Protected Members
protected:
    libusb_device_handle* m_handle;
    BYTE m_partNumber;
    unsigned long   inEpMaxTransferSize;
    unsigned long   outEpMaxTransferSize;
    BYTE    inEpPipeID;
    BYTE    outEpPipeID;
    
    BYTE maxSerialStrLen;
    BYTE maxProductStrLen;
    
    pthread_t m_thread;
    libusb_transfer* m_currentTransfer;
    AsyncReadThreadParam m_asyncReadThreadParam;
};

#endif // CP213x_DEVICE_H
