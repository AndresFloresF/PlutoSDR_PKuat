#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include "SLABHIDDevice.h"
#include "SLABCP2112.h"

int main(int argc, char *argv[])
{
    int *connectedDevice;
    const __u_short VID = 0x10C4;
    const __u_short PID = 0xEA90;
    const __u_short I2C_BMP180 = 0x20;
    int numDevices = 0;
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
    
    printf("Device found\n");

    HidSmbus_Open(connectedDevice,0,VID,PID);
    HidSmbus_SetSmbusConfig(connectedDevice,100000, 0X02,0,100,100,0,2);
    HidSmbus_GetGpioConfig(connectedDevice,0x20,0x20,0x06,0xFF);
    HidSmbus_WriteLatch(connectedDevice,0,0x20);

    

    printf("Puerto Configurado \n");

    HidSmbus_WriteRequest(connectedDevice, 0x20 , unsigned char byteArray[] {0xF4, 0X2E},2);
    delay(5);
    HidSmbus_AddressReadRequest(connectedDevice,0x20,);


    HidSmbus_Close(connectedDevice);

    return 0;
}
