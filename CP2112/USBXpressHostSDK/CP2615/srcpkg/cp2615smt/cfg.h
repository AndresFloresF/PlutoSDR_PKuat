// Copyright (c) 2015-2016 by Silicon Laboratories Inc.  All rights reserved.
// The program contained in this listing is proprietary to Silicon Laboratories,
// headquartered in Austin, Texas, U.S.A. and is subject to worldwide copyright
// protection, including protection under the United States Copyright Act of 1976
// as an unpublished work, pursuant to Section 104 and Section 408 of Title XVII
// of the United States code.  Unauthorized copying, adaptation, distribution,
// use, or display is prohibited by this law.

#ifndef __SLABCFG_H__
#define __SLABCFG_H__ 1

#ifdef _WIN32
#include <Windows.h>
#else
#include "OsDep.h"
#endif

#include "CErr.h"

//-----------------------------------------------------------------------
// These functions must be implemented by in the device-specific code

// Read from stdin the function parameters, execute the function and write results to stdout
void exec_DllCall( std::string &funcName);
// On end of stdin, release all resources left acquired by earlier exec_DllCall calls
void OnExit();

#endif // __SLABCFG_H__
