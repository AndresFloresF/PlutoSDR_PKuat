#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
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

    

    return 0;
}
