#include <string>
#include <iostream>
#include <pthread.h>
#include <vector>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "libusb.h"
#include "SLAB_USB_SPI.h"
#include "ReadThread.h"
#include "CP213xDevice.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void callBackReadThread(libusb_transfer* transferPacketptr) {
    int status = transferPacketptr->status;
    AsyncReadThreadParam* paramObj = (AsyncReadThreadParam*)transferPacketptr->user_data;
            
    if (status == LIBUSB_TRANSFER_COMPLETED) {
        int bytesTransfered = transferPacketptr->actual_length;
        
        //paramObj->buffer = &buffVect;
        pthread_mutex_lock(&lock);  // lock the critical section
        paramObj->userRead = false;
        paramObj->totalSize -= bytesTransfered;
        for(int i = 0; i < bytesTransfered; i += 1) {
            int x = transferPacketptr->buffer[i]; 
            paramObj->buffer.push_back(x);
        }
        pthread_mutex_unlock(&lock);  // unlock the critical section
    } else {
        printf("Thread returned libusb error: %d\n", status);
    }
    
    libusb_free_transfer(transferPacketptr);  
}

void *readThread(void *arg)
{
    int status;
    AsyncReadThreadParam* paramObj = (AsyncReadThreadParam*)arg;
    int timeoutMs = 100000;
    int completed = false;
    CCP213xDevice* dev = (CCP213xDevice*)paramObj->cp213xDevice;
    DWORD internalBlockSize = dev->GetInEndpointMaxTransferSize() * 2;
    BYTE* pReadBuf = (BYTE*)malloc(internalBlockSize);//(BYTE*)malloc(paramObj->blockSize);
    
    //printf("Starting Thread\n");
        
    while (1) {
        if (paramObj->shouldTerminate) {
            break;
        }
        
        if (paramObj->totalSize <= 0) {
            break;
        }
        
        libusb_transfer* transferPacketptr = libusb_alloc_transfer(0);
        dev->SetCurrentAsyncTransfer(transferPacketptr);

        libusb_fill_bulk_transfer(transferPacketptr, paramObj->hDevice, dev->GetInEndpointAddress(), pReadBuf, internalBlockSize,
                callBackReadThread, paramObj, timeoutMs);
        status = libusb_submit_transfer(transferPacketptr);	
        if (status == 0) {
            //while (paramObj->returnCode != 0 || paramObj->release == false) {
                status = libusb_handle_events_completed(0, &completed);
            //}
        }
    }
    
    dev->SetCurrentAsyncTransfer(NULL);
    free(pReadBuf);
        
    //printf("Ending Thread\n");
    pthread_exit(NULL);
}
