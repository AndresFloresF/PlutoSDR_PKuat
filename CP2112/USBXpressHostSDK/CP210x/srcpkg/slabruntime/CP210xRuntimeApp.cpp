#include "CP210xRuntime.h"
#include <stdio.h>
#include <unistd.h>

#define FATAL_ERR(a) gerror_description=a ; goto error

const char * gerror_description = NULL;


int main(int argc, const char * argv[])
{
	uint32_t numberOfDevices;
	HANDLE hdevice;
	uint16_t maxTimeoutInMicroseconds = 0;
	CP210x_STATUS status;
	uint32_t i;
	
	status = CP210xRT_GetNumDevices(&numberOfDevices);
	if (CP210x_SUCCESS != status) {
		FATAL_ERR("CP210xRT_GetNumDevices failed.");
	}
	printf("Found %d CP210x Devices.\n", numberOfDevices);
	
	for (i = 0; i < numberOfDevices; ++i) {
		HANDLE * pHandle = &hdevice;
		status = CP210xRT_Open(i, &pHandle);
		if (CP210x_SUCCESS != status) {
			FATAL_ERR("CP320xRT_Open failed.");
		} else {
			printf("Device %d opened.\n", i);
		}

		status = CP210xRT_GetReceiverMaxTimeout(pHandle, &maxTimeoutInMicroseconds);
		if (CP210x_SUCCESS != status) {
			printf("CP210xRT_GetReceiverMaxTimeout failed.\n");
		} else {
			printf("CP210xRT_GetReceiverMaxTimeout SUCCESS Timeout is %d\n", maxTimeoutInMicroseconds);
		}

		if (2000 == maxTimeoutInMicroseconds) {
			maxTimeoutInMicroseconds = 1000;
		} else {
			maxTimeoutInMicroseconds = 2000;
		}

		printf("Setting Max Timeout to %d....\n", maxTimeoutInMicroseconds);
		
		status = CP210xRT_SetReceiverMaxTimeout(pHandle, maxTimeoutInMicroseconds);
		if (CP210x_SUCCESS != status) {
			printf("CP210xRT_SetReceiverMaxTimeout failed.\n");
		} else {
			printf("CP210xRT_SetReceiverMaxTimeout SUCCESS\n");
		}



		printf("Re-reading Max Timeout....\n");

		
		status = CP210xRT_GetReceiverMaxTimeout(pHandle, &maxTimeoutInMicroseconds);
		if (CP210x_SUCCESS != status) {
			printf("CP210xRT_GetReceiverMaxTimeout failed.\n");
		} else {
			printf("CP210xRT_GetReceiverMaxTimeout SUCCESS Timeout is %d\n", maxTimeoutInMicroseconds);
		}
		
		
		status = CP210xRT_Close(pHandle);
		if (CP210x_SUCCESS != status) {
			FATAL_ERR("CP210xRT_Close failed.");
		} else {
			printf("Device %d closed.\n", i);
		}
	}
		
	printf("All devices tested. Done.");

	return 0;
error:
	if (gerror_description)
		printf("Fatal Error: %s\n", gerror_description);
	else 
		printf("Fatal error.\n");
	return -1;
}



