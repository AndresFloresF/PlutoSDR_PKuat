//
//  cp2615.cpp
//  cp2615cfg
//
//  Created by Brant Merryman on 3/28/17.
//
//

#include "cp2615.h"
#include <cassert>

CDevHandleSet g_DevHandles( 1024 );
WORD kEraseCmd  = 0xFFFC;
WORD kFlashKeys = 0xA5F1;

#ifdef _WIN32
#include <Windows.h>

void usleep(__int64 usec)
{
  HANDLE timer;
  LARGE_INTEGER ft;

  ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time

  timer = CreateWaitableTimer(NULL, TRUE, NULL);
  if (timer != NULL) {
     SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
     WaitForSingleObject(timer, INFINITE);
     CloseHandle(timer);
  }
}

#endif

WORD ComputeChecksum_cp2615(const BYTE *pAddr, WORD length)
{
  WORD checksum = 0;

  assert(pAddr && length);

  while (length--)
  {
    checksum += *pAddr;
    ++pAddr;
  }

  return checksum;
}


/*!
 SetConfigMode
 */

void SetConfigMode(HID_SMBUS_DEVICE device)
{
    // set config mode.

  HID_SMBUS_STATUS status = HidSmbus_SetGpioConfig(device, 0x07, 0x05, 0, 0);

  if (HID_SMBUS_SUCCESS != status) { throw CDllErr("HidSmbus_SetGpioConfig failed."); }

  status = HidSmbus_WriteLatch(device, 0x7, 0x7);
  if (HID_SMBUS_SUCCESS != status) { throw CDllErr("HidSmbus_WriteLatch failed."); }

  usleep(100000);

  status = HidSmbus_WriteLatch(device, 0x0, 0x3);
  if (HID_SMBUS_SUCCESS != status) { throw CDllErr("HidSmbus_WriteLatch failed."); }

  usleep(100000);

  status = HidSmbus_WriteLatch(device, 0x2, 0x2);
  if (HID_SMBUS_SUCCESS != status) { throw CDllErr("HidSmbus_WriteLatch failed."); }

  usleep(200000);

  status = HidSmbus_SetSmbusConfig(device, BITRATE, 0x02, false, 1000, 1000, false, 0);
  if (HID_SMBUS_SUCCESS != status) { throw CDllErr("HidSmbus_SetSmbusConfig failed."); }

}

/*!
 SetNormalMode
 */

void SetNormalMode(HID_SMBUS_DEVICE device)
{
  HID_SMBUS_STATUS status;
  status = HidSmbus_WriteLatch(device, 0x1, 0x7);
  if (HID_SMBUS_SUCCESS != status) { throw CDllErr("HidSmbus_WriteLatch failed."); }

  usleep(100000);

  status = HidSmbus_WriteLatch(device, 0x2, 0x2);
  if (HID_SMBUS_SUCCESS != status) { throw CDllErr("HidSmbus_WriteLatch failed."); }

  usleep(200000);

  status = HidSmbus_SetGpioConfig(device, 0, 0, 0, 0);
  if (HID_SMBUS_SUCCESS != status) { throw CDllErr("HidSmbus_SetGpioConfig failed."); }
}

/*!
 WaitForTransferStatusComplete

 This will wait until the transfer is complete. If there is an error, it will throw an exception.
 */

HID_SMBUS_STATUS WaitForTransferStatusComplete(HID_SMBUS_DEVICE device)
{
  HID_SMBUS_S0 status = 0;
  HID_SMBUS_S1 detailedStatus = 0;
  WORD retries = 0;
  WORD bytesRead = 0;

  HID_SMBUS_STATUS hidstatus;



  for (bool keepGoing = true; keepGoing;) {
    hidstatus = HidSmbus_TransferStatusRequest(device);

    if (HID_SMBUS_SUCCESS != hidstatus) { throw CDllErr("HidSmbus_TransferStatusRequest failed."); }

    hidstatus = HidSmbus_GetTransferStatusResponse(   device
                                                   ,&status
                                                   ,&detailedStatus
                                                   ,&retries
                                                   ,&bytesRead);

    if (HID_SMBUS_SUCCESS != hidstatus) { return HID_SMBUS_DEVICE_IO_FAILED; }

    switch (status) {
      case HID_SMBUS_S0_IDLE:
        return HID_SMBUS_READ_ERROR;
        break;
      case HID_SMBUS_S0_BUSY:
        usleep(10);
        break;
      case HID_SMBUS_S0_COMPLETE:
        keepGoing = false;
        break;
      case HID_SMBUS_S0_ERROR:
        return HID_SMBUS_READ_TIMED_OUT;
        break;
    }
  }
  
  return HID_SMBUS_SUCCESS;
}





