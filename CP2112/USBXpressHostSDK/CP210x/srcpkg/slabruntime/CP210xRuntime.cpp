/**
 *. CP210x Runtime
 * © 2019 Silicon Labs, All Rights Reserved.
 *
 */



#include "CP210xRuntime.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <libusb-1.0/libusb.h>


/* Part number definitions */
#define CP2101_PARTNUM		0x01
#define CP2102_PARTNUM		0x02
#define CP2103_PARTNUM		0x03
#define CP2104_PARTNUM		0x04
#define CP2105_PARTNUM		0x05
#define CP2108_PARTNUM		0x08
#define CP210x_PARTNUM_CP2102N_QFN28	0x20
#define CP210x_PARTNUM_CP2102N_QFN24	0x21
#define CP210x_PARTNUM_CP2102N_QFN20	0x22

static libusb_context* libusbContext;

__attribute__((constructor))
static void Initializer()
{
    libusb_init(&libusbContext);
}

__attribute__((destructor))
static void Finalizer()
{
    libusb_exit(libusbContext);
}

bool isCP210x(libusb_device * device)
{
	libusb_device_descriptor devDesc;
	if (0 == libusb_get_device_descriptor(device, &devDesc)) {
		if (LIBUSB_CLASS_PER_INTERFACE == devDesc.bDeviceClass ) {
			if ((1 == devDesc.iManufacturer) && (2 == devDesc.iProduct) && (3 <= devDesc.iSerialNumber)) {
				libusb_config_descriptor *pconfigDesc;
				if (0 == libusb_get_config_descriptor(device, 0, &pconfigDesc)) {
                    if (pconfigDesc->bNumInterfaces && pconfigDesc->interface->num_altsetting) {
                        if (LIBUSB_CLASS_VENDOR_SPEC == pconfigDesc->interface->altsetting->bInterfaceClass) {
                            // it is a 210x
							return true;
                        }
                    }
				}
			}
		}
	}
	return false;
}

int getPartNumber(const HANDLE * handle, BYTE * ppartNum)
{
	libusb_device_handle* h;

	h = (libusb_device_handle *) handle;
	const int ret = libusb_control_transfer(h, 0xC0, 0xFF, 0x370B, 0x0000, ppartNum, 1, 7000);
	
	
	return 1 == ret ? 0 : -1;
}

bool isCP2102N(const HANDLE * handle)
{
	BYTE partNum;
	BOOL fResult = false;
	int rc = getPartNumber(handle, &partNum);
	if (0 == rc) {
		switch (partNum) {
			case CP210x_PARTNUM_CP2102N_QFN28:
			case CP210x_PARTNUM_CP2102N_QFN24:
			case CP210x_PARTNUM_CP2102N_QFN20:
				fResult = true;
				break;
			default:
				break;
		}
	}
	
	return fResult;
}


CP210x_STATUS CP210xRT_GetNumDevices(uint32_t * lpdwNumDevices)
{
	
	libusb_device** list;
	const ssize_t numDevices = libusb_get_device_list(libusbContext, &list);
	size_t NumOfCP210xDevices = 0;
	for (ssize_t i = 0; i < numDevices; i++) {
		libusb_device *device = list[i];
		if (isCP210x(device)) {
			++NumOfCP210xDevices;
		}
	}
	
	libusb_free_device_list(list,1);
	
	*lpdwNumDevices = NumOfCP210xDevices;
	return CP210x_SUCCESS;
}



CP210x_STATUS CP210xRT_Open(const uint32_t DeviceIndex, HANDLE ** pHandle)
{
	CP210x_STATUS status = CP210x_SUCCESS;
	libusb_device** list;
	uint32_t i, icp210x;
	libusb_device_handle* h;

	const ssize_t numDevices = libusb_get_device_list(libusbContext, &list);
	
	if (DeviceIndex < numDevices && NULL != pHandle) {
		for (i = 0, icp210x = 0; i < numDevices; ++i) {
			libusb_device *device = list[i];
			if (isCP210x(device)) {
				if (icp210x == DeviceIndex) {
					// open it.
					int rc = libusb_open(device, &h);
					if (0 == rc) {
						*pHandle = (HANDLE *) h;
					} else {
						status = CP210x_COMMAND_FAILED;
					}
					break;
				} else {
					++icp210x;
				}
			}
		}
		
	} else {
		status = CP210x_INVALID_PARAMETER;
	}
	
	libusb_free_device_list(list,1);
	
	return status;
}


CP210x_STATUS CP210xRT_Close(const HANDLE * handle)
{
	libusb_device_handle* h;
	
	h = (libusb_device_handle *) handle;
	
	libusb_close(h);
	
	return CP210x_SUCCESS;
	
}

CP210x_STATUS CP210xRT_GetReceiverMaxTimeout(const HANDLE * handle, uint16_t * pMaxTimeoutInMicroseconds)
{
	CP210x_STATUS status = CP210x_INVALID_HANDLE;
	libusb_device_handle* h;
	
	h = (libusb_device_handle *) handle;

	if (!isCP2102N(handle)) {
		return CP210x_FUNCTION_NOT_SUPPORTED;
	}


	uint16_t timeout;
    int rc = libusb_control_transfer(h, 0xC0, 0x18, 0x00, 0x0000, (unsigned char *) &timeout, sizeof(timeout), 7000);
	
	if (rc == sizeof(timeout)) {
		status = CP210x_SUCCESS;
		*pMaxTimeoutInMicroseconds = timeout;
	} else {
		printf("rc: %d sizeof pTimeout: %lu\n", rc, sizeof(timeout));
		status = CP210x_DEVICE_IO_FAILED;		
	}


	return status;
}


CP210x_STATUS CP210xRT_SetReceiverMaxTimeout(const HANDLE * handle, const uint16_t maxTimeoutInMicroseconds)
{
	CP210x_STATUS status = CP210x_INVALID_HANDLE;
	libusb_device_handle* h;

	if (!isCP2102N(handle)) {
		return CP210x_FUNCTION_NOT_SUPPORTED;
	}

	h = (libusb_device_handle *) handle;

    int rc = libusb_control_transfer(h, 0x40, 0x17, maxTimeoutInMicroseconds, 0x00, 0, 0, 7000);
	
	if (rc == 0) {
		status = CP210x_SUCCESS;
	} else {
		status = CP210x_DEVICE_IO_FAILED;		
	}


	return status;
}

