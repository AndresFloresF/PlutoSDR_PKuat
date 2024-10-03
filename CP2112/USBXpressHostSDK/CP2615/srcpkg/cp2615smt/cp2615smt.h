//
//  cp2615smt.h
//  cp2615smt
//
//  Created by Brant Merryman on 3/27/17.
//
//

#ifndef cp2615smt_h
#define cp2615smt_h

#include <string>
#include <sstream>
#include <iostream>

#include "Types.h"

#ifdef _WIN32
#include <Windows.h>
#pragma warning(disable : 4996) // TODO - I'd be happy to use never functions if everybody upgraded to VS2015
#else
#include "OsDep.h"
#endif
#include <errno.h> // for errno
#include <stdio.h>

bool isSpecified( int argc, const char * argv[], const std::string &parmName);
bool isSpecified( int argc, const char * argv[], const std::string &parmName, std::string &paramValue);

WORD axtoi(const char *);

HID_SMBUS_STATUS setConfig(DWORD DevIndex, WORD vid, WORD pid, std::istream & is);
HID_SMBUS_STATUS setUserProfile(DWORD DevIndex, WORD vid, WORD pid, std::istream & is);
HID_SMBUS_STATUS verifyConfig(DWORD DevIndex, WORD vid, WORD pid, std::istream & is );
HID_SMBUS_STATUS verifyUserProfile(DWORD DevIndex, WORD vid, WORD pid, std::istream & is );

HID_SMBUS_STATUS readConfig(DWORD DevIndex, WORD vid, WORD pid, BYTE ** outConfig, size_t * outConfigSz);

void SetNormalMode(HID_SMBUS_DEVICE device);
void SetConfigMode(HID_SMBUS_DEVICE device);



#endif /* cp2615smt_h */
