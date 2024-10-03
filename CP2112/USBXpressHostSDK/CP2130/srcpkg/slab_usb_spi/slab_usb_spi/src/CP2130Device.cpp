/*
 * CP2130Device.cpp
 *
 *  Created on: June 19,2014
 *      Author: Steven Cooney
 */

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "CP2130Device.h"
#include "CP213xSupportFunctions.h"

CCP2130Device::CCP2130Device(libusb_device_handle* h) {
    // Get the endpoint data
    libusb_config_descriptor* config;
    int status = libusb_get_active_config_descriptor(libusb_get_device(h), &config);
    if (status != 0) {
        libusb_close(h);
        h = NULL;
        return;
    }
    for (int i = 0; i < config->interface->altsetting->bNumEndpoints; i++) {
        const int endPointAddress = config->interface->altsetting->endpoint[i].bEndpointAddress;
        const int maxTransferSize = config->interface->altsetting->endpoint[i].wMaxPacketSize;
        if ((endPointAddress & 0x80) == 0x80) {
            inEpPipeID = endPointAddress;
            inEpMaxTransferSize = maxTransferSize;
        } else if ((endPointAddress & 0x80) == 0x00) {
            outEpPipeID = endPointAddress;
            outEpMaxTransferSize = maxTransferSize;
        }
    }

    m_handle = h;
    m_partNumber = 0x30;
    m_thread = 0;    
}
