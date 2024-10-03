// Copyright (c) 2015-2017 by Silicon Laboratories Inc.  All rights reserved.
// The program contained in this listing is proprietary to Silicon Laboratories,
// headquartered in Austin, Texas, U.S.A. and is subject to worldwide copyright
// protection, including protection under the United States Copyright Act of 1976
// as an unpublished work, pursuant to Section 104 and Section 408 of Title XVII
// of the United States code.  Unauthorized copying, adaptation, distribution,
// use, or display is prohibited by this law.

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#else
#include "OsDep.h"
#endif
#include "stdio.h"
#include "SLAB_USB_SPI.h"
#include "util.h"
#include "smt.h"

const WORD g_LockAll =
CP213x_LOCK_PRODUCT_STR_1 |
CP213x_LOCK_PRODUCT_STR_2 |
CP213x_LOCK_SERIAL_STR |
CP213x_LOCK_PIN_CONFIG |
CP213x_LOCK_VID |
CP213x_LOCK_PID |
CP213x_LOCK_POWER |
CP213x_LOCK_POWER_MODE |
CP213x_LOCK_RELEASE_VERSION |
CP213x_LOCK_MFG_STR_1 |
CP213x_LOCK_MFG_STR_2 |
CP213x_LOCK_TRANSFER_PRIORITY;

void AbortOnErr(USB_SPI_STATUS status, std::string funcName)
{
	if (status != USB_SPI_ERRCODE_SUCCESS)
	{
		char msg[128];
		sprintf(msg, /*SIZEOF_ARRAY( msg),*/ "%s returned 0x%x", funcName.c_str(), status);
		throw CDllErr(msg);
	}
}

//---------------------------------------------------------------------------------
DWORD LibSpecificNumDevices(const CVidPid &oldVidPid, const CVidPid &newVidPid)
{
	DWORD newDevCnt;
#ifdef _WIN32
	AbortOnErr(CP213x_GetNumDevices(&newDevCnt), "CP213x_GetNumDevices");
#else
	AbortOnErr(CP213x_GetNumDevices(&newDevCnt, newVidPid.m_Vid, newVidPid.m_Pid), "CP213x_GetNumDevices");
#endif
	DWORD oldDevCnt = 0;
	if (oldVidPid.m_Vid != newVidPid.m_Vid || oldVidPid.m_Pid != newVidPid.m_Pid)
	{
#ifdef _WIN32
		AbortOnErr(CP213x_GetNumDevices(&oldDevCnt), "CP213x_GetNumDevices");
#else
		AbortOnErr(CP213x_GetNumDevices(&oldDevCnt, oldVidPid.m_Vid, oldVidPid.m_Pid), "CP213x_GetNumDevices");
#endif
	}
	return newDevCnt + oldDevCnt;
}
//---------------------------------------------------------------------------------
class CCP213xDev
{
public:
	CCP213xDev(const CVidPid &FilterVidPid, DWORD devIndex);
	~CCP213xDev();
	CP213x_DEVICE     handle() const { return m_H; }
	bool              isLocked() const;
	void              lock() const;
	void              reset() const;
	CDevType          getDevType() const;
	CVidPid           getVidPid() const;
	BYTE              getPowerMode() const;
	BYTE              getMaxPower() const;
	WORD              getDevVer() const;
	BYTE              getXferPri() const;
	std::vector<BYTE> getSerNum(bool isAscii) const;
	std::vector<BYTE> getManufacturer(bool isAscii) const;
	std::vector<BYTE> getProduct(bool isAscii) const;
	void              setSerNum(const std::vector<BYTE> &str, bool isAscii) const;
	void              setManufacturer(const std::vector<BYTE> &str, bool isAscii) const;
	void              setProduct(const std::vector<BYTE> &str, bool isAscii) const;
private:
	CP213x_DEVICE m_H;
};
CCP213xDev::CCP213xDev(const CVidPid &FilterVidPid, DWORD devIndex)
{
#ifdef _WIN32
	FilterVidPid; // suppress "variable not referenced warnings"
	AbortOnErr(CP213x_OpenByIndex(devIndex, &m_H), "CP213x_OpenByIndex");
#else
	AbortOnErr(CP213x_Open(devIndex, &m_H, FilterVidPid.m_Vid, FilterVidPid.m_Pid), "CP213x_OpenByIndex");
#endif
}
CCP213xDev::~CCP213xDev()
{
	USB_SPI_STATUS status = CP213x_Close(m_H);
	if (status != USB_SPI_ERRCODE_SUCCESS)
	{
		std::cerr << "CP213x_Close failed\n";
	}
}
bool CCP213xDev::isLocked() const
{
	WORD lock;
	AbortOnErr(CP213x_GetLock(m_H, &lock), "CP213x_GetLock");
	return (lock & g_LockAll) == 0;
}
void CCP213xDev::lock() const
{
	AbortOnErr(CP213x_SetLock(m_H, g_LockAll), "CP213x_SetLock");
}
void  CCP213xDev::reset() const
{
	AbortOnErr(CP213x_Reset(m_H), "CP213x_Reset");
}
CDevType CCP213xDev::getDevType() const
{
	return CDevType(0); // FIXME - cp2130 doesn't have PartNum
}
CVidPid CCP213xDev::getVidPid() const
{
	WORD vid; WORD pid; BYTE maxPower; BYTE powerMode; WORD devVer; BYTE transferPriority;
	AbortOnErr(CP213x_GetUsbConfig(m_H, &vid, &pid, &maxPower, &powerMode, &devVer, &transferPriority), "CP213x_GetUsbConfig");
	return CVidPid(vid, pid);
}
BYTE CCP213xDev::getPowerMode() const
{
	WORD vid; WORD pid; BYTE maxPower; BYTE powerMode; WORD devVer; BYTE transferPriority;
	AbortOnErr(CP213x_GetUsbConfig(m_H, &vid, &pid, &maxPower, &powerMode, &devVer, &transferPriority), "CP213x_GetUsbConfig");
	return powerMode;
}
BYTE CCP213xDev::getMaxPower() const
{
	WORD vid; WORD pid; BYTE maxPower; BYTE powerMode; WORD devVer; BYTE transferPriority;
	AbortOnErr(CP213x_GetUsbConfig(m_H, &vid, &pid, &maxPower, &powerMode, &devVer, &transferPriority), "CP213x_GetUsbConfig");
	return maxPower;
}
WORD CCP213xDev::getDevVer() const
{
	WORD vid; WORD pid; BYTE maxPower; BYTE powerMode; WORD devVer; BYTE transferPriority;
	AbortOnErr(CP213x_GetUsbConfig(m_H, &vid, &pid, &maxPower, &powerMode, &devVer, &transferPriority), "CP213x_GetUsbConfig");
	return devVer;
}
BYTE CCP213xDev::getXferPri() const
{
	WORD vid; WORD pid; BYTE maxPower; BYTE powerMode; WORD devVer; BYTE transferPriority;
	AbortOnErr(CP213x_GetUsbConfig(m_H, &vid, &pid, &maxPower, &powerMode, &devVer, &transferPriority), "CP213x_GetUsbConfig");
	return transferPriority;
}
std::vector<BYTE> CCP213xDev::getSerNum(bool) const
{
	std::vector<BYTE> str(MAX_UCHAR);
	BYTE CchStr = 0;
	AbortOnErr(CP213x_GetSerialString(m_H, reinterpret_cast<char *>(str.data()), &CchStr), "CP213x_GetSerialString");
	str.resize(CchStr);
	return str;
}
std::vector<BYTE> CCP213xDev::getManufacturer(bool) const
{
	std::vector<BYTE> str(MAX_UCHAR);
	BYTE CchStr = 0;
	AbortOnErr(CP213x_GetManufacturingString(m_H, reinterpret_cast<char *>(str.data()), &CchStr), "CP213x_GetManufacturingString");
	str.resize(CchStr);
	return str;
}
std::vector<BYTE> CCP213xDev::getProduct(bool) const
{
	std::vector<BYTE> str(MAX_UCHAR);
	BYTE CchStr = 0;
	AbortOnErr(CP213x_GetProductString(m_H, reinterpret_cast<char *>(str.data()), &CchStr), "CP213x_GetProductString");
	str.resize(CchStr);
	return str;
}
void CCP213xDev::setSerNum(const std::vector<BYTE> &str, bool) const
{
	AbortOnErr(CP213x_SetSerialString(m_H, reinterpret_cast<char *>(const_cast<BYTE *>(str.data())), static_cast<BYTE>(str.size())), "CP213x_SetSerialString");
}
void CCP213xDev::setManufacturer(const std::vector<BYTE> &str, bool) const
{
	AbortOnErr(CP213x_SetManufacturingString(m_H, reinterpret_cast<char *>(const_cast<BYTE *>(str.data())), static_cast<BYTE>(str.size())), "CP213x_SetManufacturingString");
}
void CCP213xDev::setProduct(const std::vector<BYTE> &str, bool) const
{
	AbortOnErr(CP213x_SetProductString(m_H, reinterpret_cast<char *>(const_cast<BYTE *>(str.data())), static_cast<BYTE>(str.size())), "CP213x_SetProductString");
}
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
struct CCP2113xParms : public CDevParms<CCP213xDev>
{
	CCP2113xParms() : m_ManufStr(false /*supportsUnicode*/)
	{
		m_PinCfgSpecified = false;
		m_XferPriSpecified = false;
	}
	virtual void readParm(const std::string &parmName);
	void program(const CCP213xDev &dev, const std::vector<BYTE> *pSerNum) const;
	void verify(const CCP213xDev &dev, CSerNumSet &serNumSet) const;
protected:
	bool              m_PinCfgSpecified;
	std::vector<BYTE> m_PinCfg;
	CManufacturerString<CCP213xDev>  m_ManufStr;
	bool              m_XferPriSpecified;
	BYTE              m_XferPri = 0;
};
void CCP2113xParms::readParm(const std::string &parmName)
{
	if (parmName == "PinConfig")
	{
		setSpecified(m_PinCfgSpecified, parmName);
		readByteArrayParmExact(m_PinCfg, SIZE_PIN_CONFIG);
		readKeyword("}"); // end of parameter list
		return;
	}
	if (m_ManufStr.readParm(parmName))
	{
		return;
	}
	if (parmName == "TransferPriority")
	{
		setSpecified(m_XferPriSpecified, parmName);
		m_XferPri = readUcharParm();
		readKeyword("}"); // end of parameter list
		return;
	}
	CDevParms::readParm(parmName);
}
void CCP2113xParms::program(const CCP213xDev &dev, const std::vector<BYTE> *pSerNum) const
{
	CDevParms::program(dev, pSerNum);

	if (m_PinCfgSpecified)
	{
		AbortOnErr(CP213x_SetPinConfig(dev.handle(), const_cast<BYTE*>(m_PinCfg.data())), "CP213x_SetPinConfig");
	}

	m_ManufStr.program(dev);

	BYTE mask = 0;
	if (m_VidPidSpecified)
	{
		mask |= CP213x_SET_VID;
		mask |= CP213x_SET_PID;
	}
	if (m_PowerModeSpecified)
	{
		mask |= CP213x_SET_POWER_MODE;
	}
	if (m_MaxPowerSpecified)
	{
		mask |= CP213x_SET_POWER_MODE;
	}
	if (m_DevVerSpecified)
	{
		mask |= CP213x_SET_RELEASE_VERSION;
	}
	if (m_XferPriSpecified)
	{
		mask |= CP213x_SET_TRANSFER_PRIORITY;
	}
	AbortOnErr(CP213x_SetUsbConfig(dev.handle(), m_Vid, m_Pid, m_MaxPower, m_PowerMode, m_DevVer, m_XferPri, mask), "CP213x_SetUsbConfig");

	return;	// void
}
void CCP2113xParms::verify(const CCP213xDev &dev, CSerNumSet &serNumSet) const
{
	CDevParms::verify(dev, serNumSet);
	if (m_XferPriSpecified)
	{
		if (m_XferPri != dev.getXferPri())
		{
			throw CCustErr("Failed TransferPriority verification");
		}
	}
	if (m_PinCfgSpecified)
	{
		std::vector<BYTE> pinConfig(m_PinCfg.size());
		AbortOnErr(CP213x_GetPinConfig(dev.handle(), pinConfig.data()), "CP213x_GetPinConfig");
		if (m_PinCfg != pinConfig)
		{
			throw CCustErr("Failed PinConfig verification");
		}
	}
	m_ManufStr.verify(dev);
}
//---------------------------------------------------------------------------------
void LibSpecificMain(const CDevType &devType, const CVidPid &vidPid, int argc, const char * argv[])
{
	if (devType.Value() == 0) // FIXME - cp2130 doesn't have PartNum
	{
		DevSpecificMain<CCP213xDev, CCP2113xParms>(devType, vidPid, argc, argv);
	}
	else
	{
		throw CSyntErr("Unsupported PartNum");
	}
}
