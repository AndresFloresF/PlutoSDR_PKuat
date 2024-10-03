//
//  cp2615.h
//  cp2615cfg
//
//  Created by Brant Merryman on 3/28/17.
//
//

#ifndef cp2615_h
#define cp2615_h

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "util.h"
#include "Types.h"
#include "SLABCP2112.h"
#include "CErr.h"

#define RD_SIZE 64

#define SLAVE_ADDRESS 0x30
#define USER_PROFILE_SLAVE_ADDRESS 0x32

#define RD_SEGMENT_SZ 512
#define WR_SEGMENT_SZ 32

#define CONFIG_PREFIX_SIZE 44

#define BITRATE 100000

#define HID_SMBUS_CHECKSUM_ERROR      0x20
#define HID_SMBUS_USER_PROFILE_ERROR  0x21
#define HID_SMBUS_SIZE_MISMATCH_ERROR 0x22

#define INVALID_HID_SMBUS_DEVICE ((HID_SMBUS_DEVICE)-1)

#ifdef _WIN32
#include <Windows.h>

void usleep(__int64 usec);
#endif

extern WORD kEraseCmd;
extern WORD kFlashKeys;

class CDevHandleSet // accumulates and holds open handles, then closes them all in dtor
{
public:
  CDevHandleSet( unsigned long elemCnt ) : m_H( elemCnt)
  {
    std::fill( m_H.begin(), m_H.end(), INVALID_HID_SMBUS_DEVICE);
  }
  HID_SMBUS_DEVICE get( DWORD DevIndex, WORD vid, WORD pid )
  {

    if( DevIndex >= static_cast<DWORD>( m_H.size()))
    {
      throw CSyntErr( "dev index: too large");
    }

    if( m_H[ DevIndex] == INVALID_HID_SMBUS_DEVICE)
    {
      HID_SMBUS_DEVICE h;
      HID_SMBUS_STATUS status = HidSmbus_Open(&h, DevIndex, vid, pid);
        //= CP210x_Open( DevIndex, &h);
      if( status != HID_SMBUS_SUCCESS)
      {
        writeUlong( status); // write the open err code as the final status
        throw CDllErr("HidSmbus_Open failed.");
      }
      m_H[ DevIndex] = h;
    }
    return m_H[ DevIndex];
  }
  void closeAll()
  {
    for( size_t i = 0; i < m_H.size(); i++)
    {
      if( m_H[ i] != INVALID_HID_SMBUS_DEVICE)
      {
        HID_SMBUS_STATUS status = HID_SMBUS_SUCCESS;
          //= CP210x_Close( m_H[ i]);
        if( status != HID_SMBUS_SUCCESS)
        {
          std::cerr << "CP210x_Close index " << i << " returned " << status << "\n";
        }
      }
    }
  }
private:
  std::vector<HID_SMBUS_DEVICE> m_H;
};

extern CDevHandleSet g_DevHandles;

WORD ComputeChecksum_cp2615(const BYTE *pAddr, WORD length);

void SetConfigMode(HID_SMBUS_DEVICE device);
void SetNormalMode(HID_SMBUS_DEVICE device);
HID_SMBUS_STATUS WaitForTransferStatusComplete(HID_SMBUS_DEVICE device);

#endif /* cp2615_hpp */



