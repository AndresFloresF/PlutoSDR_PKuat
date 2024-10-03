/*
 * CP2101Device.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: strowlan
 */

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "CP2101Device.h"
#include "CP210xSupportFunctions.h"

CCP2101Device::CCP2101Device(libusb_device_handle* h) {
    m_handle = h;
    m_partNumber = 0x01;
    maxSerialStrLen = CP210x_MAX_SERIAL_STRLEN;
    maxProductStrLen = CP210x_MAX_PRODUCT_STRLEN;
}

CP210x_STATUS CCP2101Device::GetDeviceManufacturerString(LPVOID lpManufacturer, LPBYTE lpbLength, BOOL bConvertToASCII) {
    UNREFERENCED_PARAMETER(lpManufacturer);
    UNREFERENCED_PARAMETER(lpbLength);
    UNREFERENCED_PARAMETER(bConvertToASCII);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetDeviceInterfaceString(BYTE bInterfaceNumber, LPVOID lpInterface, LPBYTE lpbLength, BOOL bConvertToASCII) {
    UNREFERENCED_PARAMETER(bInterfaceNumber);
    UNREFERENCED_PARAMETER(lpInterface);
    UNREFERENCED_PARAMETER(lpbLength);
    UNREFERENCED_PARAMETER(bConvertToASCII);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetFlushBufferConfig(LPWORD lpwFlushBufferConfig) {
    UNREFERENCED_PARAMETER(lpwFlushBufferConfig);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetDeviceMode(LPBYTE lpbDeviceModeECI, LPBYTE lpbDeviceModeSCI) {
    UNREFERENCED_PARAMETER(lpbDeviceModeECI);
    UNREFERENCED_PARAMETER(lpbDeviceModeSCI);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetBaudRateConfig(BAUD_CONFIG* baudConfigData) {
    UNREFERENCED_PARAMETER(baudConfigData);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetPortConfig(PORT_CONFIG* PortConfig) {
    UNREFERENCED_PARAMETER(PortConfig);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig) {
    UNREFERENCED_PARAMETER(DualPortConfig);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig) {
    UNREFERENCED_PARAMETER(QuadPortConfig);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetLockValue(LPBYTE lpbLockValue) {
    UNREFERENCED_PARAMETER(lpbLockValue);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetManufacturerString(LPVOID lpvManufacturer, BYTE bLength, BOOL bConvertToUnicode) {
    UNREFERENCED_PARAMETER(lpvManufacturer);
    UNREFERENCED_PARAMETER(bLength);
    UNREFERENCED_PARAMETER(bConvertToUnicode);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetInterfaceString(BYTE bInterfaceNumber, LPVOID lpvInterface, BYTE bLength, BOOL bConvertToUnicode) {
    UNREFERENCED_PARAMETER(bInterfaceNumber);
    UNREFERENCED_PARAMETER(lpvInterface);
    UNREFERENCED_PARAMETER(bLength);
    UNREFERENCED_PARAMETER(bConvertToUnicode);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetFlushBufferConfig(WORD wFlushBufferConfig) {
    UNREFERENCED_PARAMETER(wFlushBufferConfig);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetDeviceMode(BYTE bDeviceModeECI, BYTE bDeviceModeSCI) {
    UNREFERENCED_PARAMETER(bDeviceModeECI);
    UNREFERENCED_PARAMETER(bDeviceModeSCI);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetBaudRateConfig(BAUD_CONFIG* baudConfigData) {
    UNREFERENCED_PARAMETER(baudConfigData);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetPortConfig(PORT_CONFIG* PortConfig) {
    UNREFERENCED_PARAMETER(PortConfig);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig) {
    UNREFERENCED_PARAMETER(DualPortConfig);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig) {
    UNREFERENCED_PARAMETER(QuadPortConfig);
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetLockValue() {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}
