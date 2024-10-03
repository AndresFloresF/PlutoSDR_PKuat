/////////////////////////////////////////////////////////////////////////////
// CP213xDevice.cpp
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "CP213xDevice.h"
#include "CP2130Device.h"
#include "CP213xSupportFunctions.h"

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <iostream>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Dynamic Library Initializer/Finalizer (Constructor/Destructor)
////////////////////////////////////////////////////////////////////////////////

static libusb_context* libusbContext;

__attribute__((constructor))
static void Initializer() {
    // Called before main() executes
    libusb_init(&libusbContext);
}

__attribute__((destructor))
static void Finalizer() {
    // Called after main() returns
    libusb_exit(libusbContext);
}

////////////////////////////////////////////////////////////////////////////////
CCP213xDevice::~CCP213xDevice() {
}

/////////////////////////////////////////////////////////////////////////////
// CCP210xDevice Class - Static Methods
/////////////////////////////////////////////////////////////////////////////

USB_SPI_STATUS CCP213xDevice::GetNumDevices(DWORD* lpdwNumDevices, DWORD vid, DWORD pid) {
    USB_SPI_STATUS status;
    DWORD devCount = 0;
    libusb_device** list;

    // Enumerate all USB devices, returning the number
    // of devices and a list of devices
    const ssize_t count = libusb_get_device_list(libusbContext, &list);
    // A negative count indicates an error
    if (count >= 0) {
        for( ssize_t i = 0; i < count; i++){
            libusb_device_descriptor desc;
            status =  (0 == libusb_get_device_descriptor(list[i], &desc)) ? USB_SPI_ERRCODE_SUCCESS : USB_SPI_ERRCODE_UNKNOWN_ERROR;
            if(status == USB_SPI_ERRCODE_SUCCESS){
                if(desc.idVendor == vid && desc.idProduct == pid) {
                    devCount ++;
                }
            }
        }
        *lpdwNumDevices = devCount;
        libusb_free_device_list(list, 1); // Unreference all devices to free the device list
        status = USB_SPI_ERRCODE_SUCCESS;
    } else {
        status = USB_SPI_ERRCODE_UNKNOWN_ERROR;
    }
    return status;
}

USB_SPI_STATUS CCP213xDevice::Open(DWORD dwDevice, CCP213xDevice** devObj, DWORD vid, DWORD pid) {
    USB_SPI_STATUS status = USB_SPI_ERRCODE_UNKNOWN_ERROR;
    libusb_device** list;
    libusb_device_descriptor desc;
    DWORD devCount = 0;
    ssize_t i = 0;
    
    // Enumerate all USB devices, returning the number
    // of devices and a list of devices
    const ssize_t count = libusb_get_device_list(libusbContext, &list);
    
    *devObj = NULL;

    // A negative count indicates an error
    if (count >= 0) {
        libusb_device_handle* h = (libusb_device_handle *) NULL;
        
        for(i = 0; i < count; i++) {
            status =  (0 == libusb_get_device_descriptor(list[i], &desc)) ? USB_SPI_ERRCODE_SUCCESS : USB_SPI_ERRCODE_UNKNOWN_ERROR;
            if(status == 0){
                if(desc.idVendor == vid && desc.idProduct == pid) {                    
                    if(devCount == dwDevice){
                        status = (0 == libusb_open(list[i], &h)) ? USB_SPI_ERRCODE_SUCCESS : USB_SPI_ERRCODE_UNKNOWN_ERROR;
                       break;
                    }
                    devCount++;
                }
            }
            status = USB_SPI_ERRCODE_UNKNOWN_ERROR;
        }

        if (status == 0 && h) {
            // There is only one CP213x type device, so just make a new object
            // for it. Note, if we have more devices we will need to ask the device
            // for it's device type (0x30, etc) and create an object based on that
            // device
            *devObj = (CCP213xDevice*)new CCP2130Device(h);
            if( !(*devObj)) {
                libusb_close( h);
                status = USB_SPI_ERRCODE_UNKNOWN_ERROR;
            }
        } else {
            status = USB_SPI_ERRCODE_DEVICE_NOT_FOUND;
        }
        libusb_free_device_list(list, 1); // Unreference all devices to free the device list
    } else {
        status = USB_SPI_ERRCODE_UNKNOWN_ERROR;
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////
// CCP210xDevice Class - Public Methods
/////////////////////////////////////////////////////////////////////////////

BOOL CCP213xDevice::IsOpened(){
    return m_handle != NULL;
}

USB_SPI_STATUS CCP213xDevice::Reset() {
    libusb_reset_device(m_handle);
    return USB_SPI_ERRCODE_SUCCESS;
}

USB_SPI_STATUS CCP213xDevice::Close() {
    libusb_close(m_handle);
    m_handle = NULL;
    return USB_SPI_ERRCODE_SUCCESS;
}

BOOL CCP213xDevice::IsAsyncReadInProgress() {
    return (m_thread != 0);
}

void CCP213xDevice::CancelAsyncRead() {
    m_asyncReadThreadParam.shouldTerminate = true;
    m_thread = 0;
}

pthread_t* CCP213xDevice::GetAsyncReadThread() {
    return &m_thread;
}

void CCP213xDevice::SetupAsyncReadThreadParam(DWORD blockSize, DWORD totalSize, BOOL release, int returnCode) {
    m_asyncReadThreadParam.shouldTerminate = false;
    m_asyncReadThreadParam.cp213xDevice = this;
    m_asyncReadThreadParam.hDevice = m_handle;
    m_asyncReadThreadParam.blockSize = blockSize;
    m_asyncReadThreadParam.totalSize = totalSize;
    m_asyncReadThreadParam.release = release;
    m_asyncReadThreadParam.returnCode = returnCode;
}

AsyncReadThreadParam* CCP213xDevice::GetAsyncReadThreadParam() {
    return &m_asyncReadThreadParam;
}

void CCP213xDevice::SetCurrentAsyncTransfer(libusb_transfer* transferptr){
    m_currentTransfer = transferptr;
}

libusb_transfer* CCP213xDevice::GetCurrentAsyncTransfer(){
    return m_currentTransfer;
}

HANDLE CCP213xDevice::GetHandle() {
    return this;
}

BYTE CCP213xDevice::GetOutEndpointAddress() {
    return outEpPipeID;
}

BYTE CCP213xDevice::GetInEndpointAddress() {
    return inEpPipeID;
}

DWORD CCP213xDevice::GetOutEndpointMaxTransferSize() {
    return outEpMaxTransferSize;
}

DWORD CCP213xDevice::GetInEndpointMaxTransferSize() {
    return inEpMaxTransferSize;
}

USB_SPI_STATUS CCP213xDevice::GetDeviceVersion( BYTE* majorVersion, BYTE* minorVersion) {
    USB_SPI_STATUS status = USB_SPI_ERRCODE_INVALID_HANDLE;
    libusb_device_descriptor devDesc;

    // Validate parameter
    if (!ValidParam(majorVersion,minorVersion)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

    // Get descriptor that contains the index of the USB_STRING_DESCRIPTOR containing the Product String
    if (libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc) == 0) {
        *majorVersion = devDesc.bcdDevice & 0xFF00;
        *minorVersion = devDesc.bcdDevice & 0x00FF;
        status = USB_SPI_ERRCODE_SUCCESS;
    } else {
        status = USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
    }

    return status;
}

USB_SPI_STATUS CCP213xDevice::GetDeviceDescriptor(PDEVICE_DESCRIPTOR pDescriptor) {
    USB_SPI_STATUS status = USB_SPI_ERRCODE_INVALID_HANDLE;
    libusb_device_descriptor devDesc;
    
    if (libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc) == 0) {
        pDescriptor->bLength = devDesc.bLength;
        pDescriptor->bDescriptorType = devDesc.bDescriptorType;
        pDescriptor->bcdUSB = devDesc.bcdUSB;
        pDescriptor->bDeviceClass = devDesc.bDeviceClass;
        pDescriptor->bDeviceSubClass = devDesc.bDeviceSubClass;
        pDescriptor->bDeviceProtocol = devDesc.bDeviceProtocol;
        pDescriptor->bMaxPacketSize0 = devDesc.bMaxPacketSize0;
        pDescriptor->idVendor = devDesc.idVendor;
        pDescriptor->idProduct = devDesc.idProduct;
        pDescriptor->bcdDevice = devDesc.bcdDevice;
        pDescriptor->iManufacturer = devDesc.iManufacturer;
        pDescriptor->iProduct = devDesc.iProduct;
        pDescriptor->iSerialNumber = devDesc.iSerialNumber;
        pDescriptor->bNumConfigurations = devDesc.bNumConfigurations;
        status = USB_SPI_ERRCODE_SUCCESS;
    } else {
        status = USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
    }

    return status;
}

USB_SPI_STATUS CCP213xDevice::GetStringDescriptor(BYTE index, BYTE stringDescriptor[STRING_DESCRIPTOR_SIZE]) {
    USB_SPI_STATUS status = USB_SPI_ERRCODE_INVALID_HANDLE;
    status = (0==libusb_get_string_descriptor_ascii(m_handle,index,stringDescriptor,STRING_DESCRIPTOR_SIZE)) ? USB_SPI_ERRCODE_SUCCESS : USB_SPI_ERRCODE_UNKNOWN_ERROR;
    //status = libusb_get_string_descriptor_ascii(m_handle,index,LANGIDENG,stringDescriptor,STRING_DESCRIPTOR_SIZE);
    if(status >= 0){
        status = USB_SPI_ERRCODE_SUCCESS;
    }
    return status;  
}


USB_SPI_STATUS CCP213xDevice::ControlTransfer(SETUP_PACKET SetupPacket, BYTE* Buffer, DWORD* LengthTransferred) {
    USB_SPI_STATUS status = USB_SPI_ERRCODE_HWIF_DEVICE_ERROR;
    if (m_handle == NULL) {
        return USB_SPI_ERRCODE_INVALID_HANDLE;
    }
    *LengthTransferred = libusb_control_transfer(m_handle,
            SetupPacket.RequestType,
            SetupPacket.Request,
            SetupPacket.Value,
            SetupPacket.Index,
            Buffer,
            SetupPacket.Length,
            0);
    if (*LengthTransferred >= 0) {
        status = USB_SPI_ERRCODE_SUCCESS;
    }
    return status;
}

USB_SPI_STATUS CCP213xDevice::BulkTransfer(BYTE EndpointAddress, BYTE* Buffer, DWORD Length, DWORD* LengthTransferred, DWORD Timeout) {
    USB_SPI_STATUS status = USB_SPI_ERRCODE_HWIF_DEVICE_ERROR;
    int result, bytesTransferred;
    if (m_handle == NULL) {
        return USB_SPI_ERRCODE_INVALID_HANDLE;
    }
    result = libusb_bulk_transfer(m_handle,
            EndpointAddress,
            Buffer,
            Length,
            &bytesTransferred,
            Timeout);
    *LengthTransferred = bytesTransferred;
    if (result == 0) {
        status = USB_SPI_ERRCODE_SUCCESS;
    }
    return status;
}

#if defined(__linux__)
USB_SPI_STATUS CCP213xDevice::BulkTransfer_Asynch(BYTE EndpointAddress, BYTE* Buffer, DWORD Length, DWORD* LengthTransferred, DWORD Timeout, int WaitObject) {
    USB_SPI_STATUS status = USB_SPI_ERRCODE_SUCCESS;
    int result;
    if (m_handle == NULL) {
        return USB_SPI_ERRCODE_INVALID_HANDLE;
    }
    struct libusb_transfer *transfer = libusb_alloc_transfer(0);
    BulkTransferStruct* context = (BulkTransferStruct*)malloc(sizeof(BulkTransferStruct));
    if ((transfer == NULL) || (context == NULL)) {
        if (transfer != NULL) libusb_free_transfer(transfer);
        if (context != NULL) free(context);
        return USB_SPI_ERRCODE_ALLOC_FAILURE;
    }
    context->pLengthTransferred = LengthTransferred;
    context->waitObject = WaitObject;
    libusb_fill_bulk_transfer(transfer, m_handle, EndpointAddress, Buffer, Length, BulkTransferCallback, (void*)context, Timeout);
    result = libusb_submit_transfer(transfer);
    if (result != 0) {
        libusb_free_transfer(transfer);
        free(context);
        status = USB_SPI_ERRCODE_HWIF_DEVICE_ERROR;
    }
    return status;
}

void BulkTransferCallback(struct libusb_transfer *transfer) {
    uint64_t u = 1;
    BulkTransferStruct* context = (BulkTransferStruct*)transfer->user_data;
    if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
        context->status = USB_SPI_ERRCODE_SUCCESS;
        *context->pLengthTransferred = transfer->actual_length;
    } else {
        context->status = USB_SPI_ERRCODE_HWIF_DEVICE_ERROR;
        u = 2; // Signal a timeout
    }
    write(context->waitObject, &u, sizeof(uint64_t));
    libusb_free_transfer(transfer);
    free(context);
    
    return; // void
}
#endif // defined(__linux__)

USB_SPI_STATUS CCP213xDevice::GetProductString(LPSTR lpProduct, BYTE* lpbLength, BOOL bConvertToASCII) {
    USB_SPI_STATUS status = USB_SPI_ERRCODE_INVALID_HANDLE;
    libusb_device_descriptor devDesc;
    
    // Validate parameter
    if (!ValidParam(lpProduct, lpbLength)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

     //Get descriptor that contains the index of the USB_STRING_DESCRIPTOR containing the Product String
    if (libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc) == 0) {
        int length;

        if (bConvertToASCII) {
            length = libusb_get_string_descriptor_ascii(m_handle, devDesc.iProduct, (unsigned char*) lpProduct, maxProductStrLen);
            if (length > 0) {
                *lpbLength = (BYTE) (length & 0xFF);
                status = USB_SPI_ERRCODE_SUCCESS;
            } else {
                status = USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
            }
        } else {
            length = libusb_get_string_descriptor(m_handle, devDesc.iProduct, 0x0000, (unsigned char*) lpProduct, maxProductStrLen);
            if (length > 0) {
                *lpbLength = (BYTE) (length & 0xFF);
                status = USB_SPI_ERRCODE_SUCCESS;
            } else {
                status = USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
            }
        }
    } else {
        // status = tbd;    // libusb_get_device_descriptor() failed
    }

    return status;
}

USB_SPI_STATUS CCP213xDevice::GetSerialString(LPSTR lpProduct, BYTE* lpbLength, BOOL bConvertToASCII) {
    USB_SPI_STATUS status = USB_SPI_ERRCODE_INVALID_HANDLE;
    int length;
    int index;
    libusb_device_descriptor devDesc;
    
    // Validate parameter
    if (!ValidParam(lpProduct, lpbLength)) {
        return USB_SPI_ERRCODE_INVALID_PARAMETER;
    }

     //Get descriptor that contains the index of the USB_STRING_DESCRIPTOR containing the Product String
    if (libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc) == 0) {
        index = devDesc.iSerialNumber;
    }

    if (bConvertToASCII) {
        length = libusb_get_string_descriptor_ascii(m_handle, index, (unsigned char*) lpProduct, maxProductStrLen);
        if (length > 0) {
            *lpbLength = (BYTE) (length & 0xFF);
            status = USB_SPI_ERRCODE_SUCCESS;
        } else {
            status = USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
        }
    } else {
        length = libusb_get_string_descriptor(m_handle, index, 0x0000, (unsigned char*) lpProduct, maxProductStrLen);
        if (length > 0) {
            *lpbLength = (BYTE) (length & 0xFF);
            status = USB_SPI_ERRCODE_SUCCESS;
        } else {
            status = USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT;
        }
    }

    return status;
}


