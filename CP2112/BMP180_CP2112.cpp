#include "Utilities.h"
#include <string>
#include <list>
#include <limits.h>
#include <stdio.h>
DWORD  numDevices;
#include "SLABCP2112.h"
#define VID									0x10C4
#define PID									0xEA90

std::list<HID_SMBUS_DEVICE> deviceList;
HID_SMBUS_DEVICE		m_hidSmbus;
#include "SLABCP2112.h"

void UpdateDeviceInformation(bool connected);
BOOL Connect(DWORD	deviceNum);
BOOL Disconnect(HID_SMBUS_DEVICE m_hidSmbus);

using namespace std;
int main(int argc, char *argv[])
{
    DWORD				i;
    string				selText;
    HidSmbus_GetNumDevices(&numDevices, VID, PID);
    
    if (numDevices == 0)
    {
        printf("No device found\n");
        return 1;
    }
    else if (numDevices > 1)
    {
        printf("More than one device found\n");
        return 1;
    }
    
    printf("Dispositivo encontrado\n");
    for (i = 0; i < numDevices; i++)
    {
        printf("Device %d Information: \n", i);
        Connect(i);
    }
    list<HID_SMBUS_DEVICE>::iterator it;
    for (it = deviceList.begin(); it != deviceList.end(); ++it)
    {
        Disconnect(*it);
    }
    deviceList.clear();

    return 0;
}

BOOL Connect(DWORD	deviceNum)
{
    
    BOOL		connected = FALSE;
    string     serial;
    HID_SMBUS_STATUS status = HidSmbus_Open(&m_hidSmbus, deviceNum, VID, PID);

    printf("HidSmbus_Open(): %s \n" ,HidSmbus_DecodeErrorStatus(status));
    // Attempt to open the device
    if (status == HID_SMBUS_SUCCESS)
    {
        connected = TRUE;
        deviceList.push_back(m_hidSmbus);
    }
    else
    {
        printf("Connection Error: %s \n" ,HidSmbus_DecodeErrorStatus(status));
    }

    // Update the device information now that we are connected to it
    // (this will give us the part number and version if connected)
    UpdateDeviceInformation(true);
    // Update all device settings for all tabs

    //SetFromDevice();

    return connected;
}

void UpdateDeviceInformation(bool connected)
{
    BOOL					opened;
    HID_SMBUS_DEVICE_STR	deviceString;
    WORD					vid;
    WORD					pid;
    WORD					releaseNumber;
    BYTE					partNumber;
    BYTE					version;

    char devicePathString[PATH_MAX];
    string				vidString;
    string				pidString;
    string				releaseNumberString;
    string				partNumberString;
    string				versionString;
    string				serialString;
    string				pathString;
    string				manufacturerString;
    string				productString;

    // If we're already connected to the device, then we can call the
    // opened version of the device information functions
    if (connected == TRUE &&
        HidSmbus_IsOpened(m_hidSmbus, &opened) == HID_SMBUS_SUCCESS &&
        opened == TRUE)
    {
        // Update device information (opened)

        if (HidSmbus_GetOpenedAttributes(m_hidSmbus, &vid, &pid, &releaseNumber) == HID_SMBUS_SUCCESS)
        {
            printf("VID = 0x%04X \n",vid);
            printf("PID = 0x%04X \n",pid);
            printf("Release Number = %X.%02X \n", (UINT)(releaseNumber >> 8), (UINT)((BYTE)releaseNumber));
        }
        if (HidSmbus_GetPartNumber(m_hidSmbus, &partNumber, &version) == HID_SMBUS_SUCCESS)
        {
            printf("Part Number = %d \n",partNumber);
            printf("Version = %d \n",version);
        }
        if (HidSmbus_GetOpenedString(m_hidSmbus, deviceString, HID_SMBUS_GET_SERIAL_STR) == HID_SMBUS_SUCCESS)
        {
            printf("Serial = %s \n",deviceString);
        }
        if (HidSmbus_GetOpenedString(m_hidSmbus, devicePathString, HID_SMBUS_GET_PATH_STR) == HID_SMBUS_SUCCESS)
        {
            printf("Device Path = %s \n",devicePathString);
        }
        if (HidSmbus_GetOpenedString(m_hidSmbus, deviceString, HID_SMBUS_GET_MANUFACTURER_STR) == HID_SMBUS_SUCCESS)
        {
            printf("Manufacturer = %s \n",deviceString);
        }
        if (HidSmbus_GetOpenedString(m_hidSmbus, deviceString, HID_SMBUS_GET_PRODUCT_STR) == HID_SMBUS_SUCCESS)
        {
            printf("Product = %s \n",deviceString);
        }
    }
}
BOOL Disconnect(HID_SMBUS_DEVICE m_hidSmbus)
    {
    bool disconnected = false;

    // Disconnect from the current device
    HID_SMBUS_STATUS status = HidSmbus_Close(m_hidSmbus);
    m_hidSmbus = NULL;

    printf("==================== HidSmbus_Close(): %s \n",HidSmbus_DecodeErrorStatus(status));
    // Output an error message if the close failed
    if (status != HID_SMBUS_SUCCESS)
    {
        printf("Connection Error: %s \n" ,HidSmbus_DecodeErrorStatus(status));
    }
    else
    {
        disconnected = TRUE;
    }
    return disconnected;
}

/*int opened = 0;
    while (opened==0)
    {
        HidSmbus_Open(&connectedDevice,0,VID,PID);
        opened=HidDevice_IsOpened(connectedDevice);
    }
    printf("%d\n",opened);
    HidSmbus_SetSmbusConfig(connectedDevice,100000, 0X02,0,100,100,0,2);
    HidSmbus_GetGpioConfig(connectedDevice,0x20,0x20,0x06,0xFF);
    HidSmbus_WriteLatch(connectedDevice,0,0x20);


    

    printf("Puerto Configurado \n");

    /*HidSmbus_WriteRequest(connectedDevice, 0x20 , 0x2C ,1);
    int miliseconds = 10;
    clock_t starttime = clock();
    while (clock() < starttime+miliseconds)
    {
        ;
    }
    BYTE status = 0;
    int bytestoRead = 22;
    BYTE buffer[62]={0};
    BYTE valdata[22]={0};
    BYTE req[] = {0xAA};
   
    HidSmbus_AddressReadRequest(connectedDevice,0xEE,bytestoRead,1,req);
    HidSmbus_ForceReadResponse(connectedDevice,bytestoRead);
   /*for (int byteslec = 0; byteslec < bytestoRead;)
    {
        BYTE leidos = 0;
        HidSmbus_GetReadResponse(connectedDevice,status,buffer,bytestoRead,&leidos);
        printf("STATUS: %x\n",status);
        printf("Bytes Leidos: %x\n",leidos);
        for (int i = 0; i < leidos; i++)
        {
            int index = byteslec + i;
            if ((index>=0) && (index<bytestoRead))
            {
                valdata[index]=buffer[i];
            }
            byteslec += leidos;
        }
    }
    for (int i = 0; i < 22; i++)
    {
        printf("%x\n",valdata[i]);
    }
    printf("STATUS",status);
    printf("%x\n",status);

    printf("Se envio la respuesta \n");

    HidSmbus_Close(connectedDevice);
*/ 