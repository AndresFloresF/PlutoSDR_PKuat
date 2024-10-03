/*
 * CP210xSupportFunctions.h
 *
 *  Created on: Oct 30, 2012
 *      Author: strowlan
 */

#ifndef CP210XSUPPORTFUNCTIONS_H_
#define CP210XSUPPORTFUNCTIONS_H_

#include <string.h>
#include "Types.h"

#define		CP210x_MAX_SETUP_LENGTH				512

BOOL _CP2130_ValidParam(LPVOID lpVoidPointer);
BOOL _CP2130_ValidParam(LPWORD lpwPointer);
BOOL _CP2130_ValidParam(LPBYTE lpbPointer);
BOOL _CP2130_ValidParam(LPVOID lpVoidPointer);
BOOL _CP2130_ValidParam(LPVOID lpVoidPointer, LPBYTE lpbPointer);
void _CP2130_CopyToString(BYTE* setup, LPVOID string, BYTE* bLength, BOOL bConvertToUnicode);
void _CP2130_ConvertToUnicode(BYTE* dest, BYTE* source, BYTE bLength);
void ZeroMemory(void* buf, unsigned int bufSize);

#define ValidParam _CP2130_ValidParam
#define CopyToString _CP2130_CopyToString
#define ConvertToUnicode _CP2130_ConvertToUnicode

#endif /* CP210XSUPPORTFUNCTIONS_H_ */
