/*
 * CP2104Device.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: strowlan
 */

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "CP2104Device.h"
#include "CP210xSupportFunctions.h"

CCP2104Device::CCP2104Device(libusb_device_handle* h) {
    m_handle = h;
    m_partNumber = 0x04;
    maxSerialStrLen = CP210x_MAX_SERIAL_STRLEN;
    maxProductStrLen = CP210x_MAX_PRODUCT_STRLEN;
}

CP210x_STATUS CCP2104Device::GetDeviceInterfaceString(BYTE bInterfaceNumber, LPVOID lpInterface, LPBYTE lpbLength, BOOL bConvertToASCII) {
    UNREFERENCED_PARAMETER(bInterfaceNumber);
    UNREFERENCED_PARAMETER(lpInterface);
    UNREFERENCED_PARAMETER(lpbLength);
    UNREFERENCED_PARAMETER(bConvertToASCII);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2104Device::GetFlushBufferConfig(LPWORD lpwFlushBufferConfig) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];

    // Validate parameter
    if (!ValidParam(lpwFlushBufferConfig)) {
        return CP210x_INVALID_PARAMETER;
    }

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);
    
    if (libusb_control_transfer(m_handle, 0xC0, 0xFF, 0x370D, 0, setup, 1, 0) == 1) {
        *lpwFlushBufferConfig = setup[0];
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2104Device::GetDeviceMode(LPBYTE lpbDeviceModeECI, LPBYTE lpbDeviceModeSCI) {
    UNREFERENCED_PARAMETER(lpbDeviceModeECI);
    UNREFERENCED_PARAMETER(lpbDeviceModeSCI);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2104Device::GetBaudRateConfig(BAUD_CONFIG* baudConfigData) {
    UNREFERENCED_PARAMETER(baudConfigData);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2104Device::GetPortConfig(PORT_CONFIG* PortConfig) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];
    uint16_t transferSize = 13;

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);
    
    if (libusb_control_transfer(m_handle, 0xC0, 0xFF, 0x370C, 0, setup, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
        PortConfig->Mode = (uint16_t) (setup[0] << 8) + setup[1];
        //PortConfig->Reset.LowPower = (setup[10] << 8) + setup[11];
        PortConfig->Reset_Latch = (uint16_t) (setup[4] << 8) + setup[5];
        //PortConfig->Suspend.Mode = (setup[14] << 8) + setup[15];
        //PortConfig->Suspend.LowPower = (setup[16] << 8) + setup[17];
        PortConfig->Suspend_Latch = (uint16_t) (setup[10] << 8) + setup[11];
        PortConfig->EnhancedFxn = setup[12];

        // Mask out reserved bits in EnhancedFxn
        PortConfig->EnhancedFxn &= ~(EF_SERIAL_DYNAMIC_SUSPEND | EF_RESERVED_1);
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2104Device::GetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig) {
  UNREFERENCED_PARAMETER(DualPortConfig);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2104Device::GetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig) {
  UNREFERENCED_PARAMETER(QuadPortConfig);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2104Device::GetLockValue(LPBYTE lpbLockValue) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];

    // Validate parameter
    if (!ValidParam(lpbLockValue)) {
        return CP210x_INVALID_PARAMETER;
    }

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);
    
    if (libusb_control_transfer(m_handle, 0xC0, 0xFF, 0x370A, 0, setup, 1, 0) == 1) {
        if (setup[0] == 0xFF)
            *lpbLockValue = 0x00;
        else
            *lpbLockValue = 0x01;
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2104Device::SetInterfaceString(BYTE bInterfaceNumber, LPVOID lpvInterface, BYTE bLength, BOOL bConvertToUnicode) {
    UNREFERENCED_PARAMETER(bInterfaceNumber);
    UNREFERENCED_PARAMETER(lpvInterface);
    UNREFERENCED_PARAMETER(bLength);
    UNREFERENCED_PARAMETER(bConvertToUnicode);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2104Device::SetFlushBufferConfig(WORD wFlushBufferConfig) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x370D, wFlushBufferConfig, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2104Device::SetDeviceMode(BYTE bDeviceModeECI, BYTE bDeviceModeSCI) {
    UNREFERENCED_PARAMETER(bDeviceModeECI);
    UNREFERENCED_PARAMETER(bDeviceModeSCI);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2104Device::SetBaudRateConfig(BAUD_CONFIG* baudConfigData) {
    UNREFERENCED_PARAMETER(baudConfigData);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2104Device::SetPortConfig(PORT_CONFIG* PortConfig) {
    CP210x_STATUS status;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];
    BYTE Temp_EnhancedFxn;
    uint16_t transferSize = 13;
    uint16_t Temp_Reset_Latch = PortConfig->Reset_Latch;
    uint16_t Temp_Suspend_Latch = PortConfig->Suspend_Latch;
    // Change user Port_Config structure to match firmware, and check reserved bits are zero
    if (PortConfig->EnhancedFxn & (EF_SERIAL_DYNAMIC_SUSPEND | EF_RESERVED_1))
        return CP210x_INVALID_PARAMETER;

    Temp_EnhancedFxn = PortConfig->EnhancedFxn; // save user settings into temp variable to send out

    if (Temp_EnhancedFxn & EF_WEAKPULLUP) {
        Temp_EnhancedFxn |= 0x30; // Set both Weak Pullup bits
    } else {
        Temp_EnhancedFxn &= ~0x30; // Clear both Weak Pullup bits
    }
    // overwrite reset latch if in RS485 mode
    if (PortConfig->EnhancedFxn & EF_GPIO_2_RS485) {
        if (PortConfig->EnhancedFxn & EF_RS485_INVERT) {
            Temp_Reset_Latch &= ~PORT_GPIO_2_ON;
            Temp_Suspend_Latch &= ~PORT_GPIO_2_ON;
        }
        else {
            Temp_Reset_Latch |= PORT_GPIO_2_ON;
            Temp_Suspend_Latch |= PORT_GPIO_2_ON;
        }
    }
    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);
    
    setup[0] = (PortConfig->Mode & 0xFF00) >> 8;
    setup[1] = (PortConfig->Mode & 0x00FF);
    setup[2] = 0x00; //(PortConfig->Reset.LowPower & 0xFF00) >> 8;
    setup[3] = 0x00; //(PortConfig->Reset.LowPower & 0x00FF);
    setup[4] = (Temp_Reset_Latch & 0xFF00) >> 8;
    setup[5] = (Temp_Reset_Latch & 0x00FF);
    setup[6] = (PortConfig->Mode & 0xFF00) >> 8;
    setup[7] = (PortConfig->Mode & 0x00FF);
    setup[8] = 0x00; //(PortConfig->Suspend.LowPower & 0xFF00) >> 8;
    setup[9] = 0x00; //(PortConfig->Suspend.LowPower & 0x00FF);
    setup[10] = (Temp_Suspend_Latch & 0xFF00) >> 8;
    setup[11] = (Temp_Suspend_Latch & 0x00FF);
    setup[12] = Temp_EnhancedFxn;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x370C, 0, setup, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2104Device::SetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig) {
    UNREFERENCED_PARAMETER(DualPortConfig);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2104Device::SetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig) {
    UNREFERENCED_PARAMETER(QuadPortConfig);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2104Device::SetLockValue() {
    CP210x_STATUS status;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x370A, 0xF0, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}
