// Copyright (c) 2015-2016 by Silicon Laboratories Inc.  All rights reserved.
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
#include <cstring>
#include <cstdlib>

#include "utf8.h"

#ifdef _WIN32
#include <Windows.h>
#include "CP210xManufacturingDLL.h"
#else
#include "OsDep.h"
#include "CP210xManufacturing.h"
#endif
#include "stdio.h"
#include "util.h"
#include "smt.h"

// These defs are also used to calculate offsets to the various config elements
#define CP2102N_PREAMBLE_SIZE   55  // Config bytes before stuff we care about
#define CP2102N_LANGDESC_SIZE   4   // Language description
#define CP2102N_MANF_SIZE       131 // Manufacturer Description string
#define CP2102N_PROD_SIZE       259 // Product Description string
#define CP2102N_SERNUM_SIZE     131 // Serial Number string
#define CP2102N_POSTAMBLE_SIZE  96  // 96 more bytes of other stuff we don't care about
#define CP2102N_CHECKSUM_SIZE   2   // two bytes of checksum
#define CP2102N_TOTAL_CONFIG_SIZE     \
           (CP2102N_PREAMBLE_SIZE   + \
            CP2102N_LANGDESC_SIZE   + \
            CP2102N_MANF_SIZE       + \
            CP2102N_PROD_SIZE       + \
            CP2102N_SERNUM_SIZE     + \
            CP2102N_POSTAMBLE_SIZE  + \
            CP2102N_CHECKSUM_SIZE)

typedef enum  {
   CP2102N_SERIAL_NUMBER
  ,CP2102N_MANUFACTURER_STRING
  ,CP2102N_PRODUCT_STRING
} cp2102n_usb_string_desc_t;

void AbortOnErr( CP210x_STATUS status, std::string funcName)
{
    if( status != CP210x_SUCCESS)
    {
        char msg[ 128];
        sprintf( msg, /*SIZEOF_ARRAY( msg),*/ "%s returned 0x%x", funcName.c_str(), status);
        throw CDllErr( msg);
    }
}

//---------------------------------------------------------------------------------
DWORD LibSpecificNumDevices(const CVidPid &, const CVidPid &)
{
    uint32_t DevCnt;
    AbortOnErr(CP210x_GetNumDevices(&DevCnt), "CP210x_GetNumDevices");
    return (DWORD)DevCnt;
}

//---------------------------------------------------------------------------------
class CCP210xDev
{
public:
    CCP210xDev( const CVidPid &FilterVidPid, DWORD devIndex);
    ~CCP210xDev();
    HANDLE            handle() const { return m_H; }
    bool              isLocked() const;
    void              lock() const;
    void              reset() const;
    CDevType          getDevType() const;
    CVidPid           getVidPid() const;
    BYTE              getPowerMode() const;
    BYTE              getMaxPower() const;
    WORD              getDevVer() const;
    WORD              getFlushBufCfg() const;
    std::vector<BYTE> getSerNum( bool isAscii) const;
    std::vector<BYTE> getManufacturer( bool isAscii) const;
    std::vector<BYTE> getProduct( bool isAscii) const;
    void              setVidPid( WORD vid, WORD pid) const;
    void              setPowerMode( BYTE val) const;
    void              setMaxPower( BYTE val) const;
    void              setDevVer( WORD val) const;
    void              setFlushBufCfg( WORD val) const;
    void              setSerNum( const std::vector<BYTE> &str, bool isAscii) const;
    void              setManufacturer( const std::vector<BYTE> &str, bool isAscii) const;
    void              setProduct( const std::vector<BYTE> &str, bool isAscii) const;

    HANDLE m_H;
};

CCP210xDev::CCP210xDev( const CVidPid &, DWORD devIndex)
{
    AbortOnErr( CP210x_Open( devIndex, &m_H), "CP210x_Open");
}

CCP210xDev::~CCP210xDev()
{
    CP210x_STATUS status = CP210x_Close( m_H);
    if( status != CP210x_SUCCESS)
    {
        std::cerr << "CP210x_Close failed\n";
    }
}

bool CCP210xDev::isLocked() const
{
    BYTE lock;
    AbortOnErr( CP210x_GetLockValue( m_H, &lock), "CP210x_GetLockValue");
    return lock != 0;
}

void CCP210xDev::lock() const
{
    AbortOnErr( CP210x_SetLockValue( m_H), "CP210x_SetLockValue");
}

void  CCP210xDev::reset() const
{
    AbortOnErr( CP210x_Reset( m_H), "CP210x_Reset");
}

CDevType CCP210xDev::getDevType() const
{
    BYTE partNum;
    AbortOnErr( CP210x_GetPartNumber( m_H, &partNum ), "CP210x_GetPartNumber");
    return CDevType( partNum);
}

CVidPid CCP210xDev::getVidPid() const
{
    WORD vid, pid;
    AbortOnErr( CP210x_GetDeviceVid( m_H, &vid ), "CP210x_GetDeviceVid");
    AbortOnErr( CP210x_GetDevicePid( m_H, &pid ), "CP210x_GetDevicePid");
    return CVidPid( vid, pid);
}

BYTE CCP210xDev::getPowerMode() const
{
    BOOL SelfPower;
    AbortOnErr( CP210x_GetSelfPower( m_H, &SelfPower), "CP210x_GetSelfPower");
    return SelfPower ? 1 : 0;
}

BYTE CCP210xDev::getMaxPower() const
{
    BYTE MaxPower;
    AbortOnErr( CP210x_GetMaxPower( m_H, &MaxPower), "CP210x_GetMaxPower");
    return MaxPower;
}

WORD CCP210xDev::getDevVer() const
{
    WORD devVer;
    AbortOnErr( CP210x_GetDeviceVersion( m_H, &devVer), "CP210x_GetDeviceVersion");
    return devVer;
}

WORD CCP210xDev::getFlushBufCfg() const
{
    WORD flushBufCfg;
    AbortOnErr( CP210x_GetFlushBufferConfig( m_H, &flushBufCfg), "CP210x_GetFlushBufferConfig");
    return flushBufCfg;
}

std::vector<BYTE> CCP210xDev::getSerNum( bool isAscii) const
{
    std::vector<BYTE> str( MAX_UCHAR);
    BYTE CchStr = 0;
    AbortOnErr( CP210x_GetDeviceSerialNumber( m_H, str.data(), &CchStr, isAscii), "CP210x_GetDeviceSerialNumber");
    str.resize( (size_t)CchStr * (isAscii ? 1 : 2));
    return str;
}

std::vector<BYTE> CCP210xDev::getManufacturer( bool isAscii) const
{
    std::vector<BYTE> str( MAX_UCHAR);
    BYTE CchStr = 0;
    AbortOnErr( CP210x_GetDeviceManufacturerString( m_H, str.data(), &CchStr, isAscii), "CP210x_GetDeviceManufacturerString");
    str.resize((size_t)CchStr * (isAscii ? 1 : 2));
    return str;
}

std::vector<BYTE> CCP210xDev::getProduct( bool isAscii) const
{
    std::vector<BYTE> str( MAX_UCHAR);
    BYTE CchStr = 0;
    AbortOnErr( CP210x_GetDeviceProductString( m_H, str.data(), &CchStr, isAscii), "CP210x_GetDeviceProductString");
    str.resize((size_t)CchStr * (isAscii ? 1 : 2));
    return str;
}

void CCP210xDev::setVidPid( WORD vid, WORD pid) const
{
    AbortOnErr( CP210x_SetVid( m_H, vid), "CP210x_SetVid");
    AbortOnErr( CP210x_SetPid( m_H, pid), "CP210x_SetPid");
}

void CCP210xDev::setPowerMode( BYTE val) const
{
    AbortOnErr( CP210x_SetSelfPower( m_H, val ? TRUE : FALSE ), "CP210x_SetSelfPower");
}

void CCP210xDev::setMaxPower( BYTE val) const
{
    AbortOnErr( CP210x_SetMaxPower( m_H, val), "CP210x_SetMaxPower");
}

void CCP210xDev::setDevVer( WORD val) const
{
    AbortOnErr( CP210x_SetDeviceVersion( m_H, val), "CP210x_SetDeviceVersion");
}

void CCP210xDev::setFlushBufCfg( WORD val) const
{
    AbortOnErr( CP210x_SetFlushBufferConfig( m_H, val), "CP210x_SetFlushBufferConfig");
}

void CCP210xDev::setSerNum( const std::vector<BYTE> &str, bool isAscii) const
{
    BYTE CchStr = static_cast<BYTE> ( str.size() / (isAscii ? 1 : 2));
    AbortOnErr( CP210x_SetSerialNumber( m_H, const_cast<BYTE*>( str.data()), CchStr, isAscii), "CP210x_SetSerialNumber");
}

void CCP210xDev::setManufacturer( const std::vector<BYTE> &str, bool isAscii) const
{
    BYTE CchStr = static_cast<BYTE> ( str.size() / (isAscii ? 1 : 2));
    AbortOnErr( CP210x_SetManufacturerString( m_H, const_cast<BYTE*>( str.data()), CchStr, isAscii), "CP210x_SetManufacturerString");
}

void CCP210xDev::setProduct( const std::vector<BYTE> &str, bool isAscii) const
{
    BYTE CchStr = static_cast<BYTE> ( str.size() / (isAscii ? 1 : 2));
    AbortOnErr( CP210x_SetProductString( m_H, const_cast<BYTE*>( str.data()), CchStr, isAscii), "CP210x_SetProductString");
}

//---------------------------------------------------------------------------------
// CP2102N has non-standard get/set lock functions.

#pragma pack(push, 1)
// The original complete copy of the below struct is called Config_t in the CP2102N repo.
// "Used to store default settings and customized values passed from host PC."
union CCP2102NConfig {
    struct { // just the top of it, as far as we need
        WORD configSize;
        BYTE configVersion;
        // Allows Bootloader entry from VCP vendor command/HID report.  Bootloader always runs
        // if flash address 0x0 is 0xFF.
        BYTE enableBootloader;
        // Allows configuration updates.  If a bad burn happens, this will become
        // 0xff allowing a recovery.
        BYTE enableConfigUpdate;
    } Fields;
    BYTE Raw[CP2102N_TOTAL_CONFIG_SIZE];
};
#pragma pack(pop)

#define CP2102N_CONFIG_VERSION   1
#define CP2102N_CONFIG_UNLOCKED  0xff

void setCP2102N_USBString(CCP2102NConfig & config, cp2102n_usb_string_desc_t stringToSet, const char * value);

class CCP2102NDev : public CCP210xDev
{
public:
    CCP2102NDev( const CVidPid &FilterVidPid, DWORD devIndex) : CCP210xDev( FilterVidPid, devIndex) {}
    bool              isLocked() const;
    void              lock() const;
    void              setSerNum( const std::vector<BYTE> &str, bool isAscii) const;
};

bool CCP2102NDev::isLocked() const
{
    CCP2102NConfig Config = {0};
    AbortOnErr( CP210x_GetConfig( m_H, &Config.Raw[0], static_cast<WORD>( sizeof( Config))), "CP210x_GetConfig");
    if( Config.Fields.configVersion != CP2102N_CONFIG_VERSION)
    {
        throw CCustErr( "CP2102N returned unknown config version");
    }
    if( Config.Fields.configSize < sizeof( Config.Fields))
    {
        throw CCustErr( "CP2102N returned invalid config size");
    }
    return Config.Fields.enableConfigUpdate != CP2102N_CONFIG_UNLOCKED;
}

void CCP2102NDev::lock() const
{
    CCP2102NConfig Config;
    AbortOnErr( CP210x_GetConfig( m_H, &Config.Raw[0], static_cast<WORD>( sizeof( Config))), "CP210x_GetConfig");
    Config.Fields.enableConfigUpdate = 0;
    AbortOnErr( CP210x_SetConfig( m_H, &Config.Raw[0], static_cast<WORD>( sizeof( Config))), "CP210x_SetConfig");

    CCP2102NConfig finalConfig;
    AbortOnErr( CP210x_GetConfig( m_H, &finalConfig.Raw[0], static_cast<WORD>( sizeof( finalConfig))), "CP210x_GetConfig");
    if( memcmp( &Config.Raw[0], &finalConfig.Raw[0], sizeof( finalConfig)))
    {
        throw CCustErr( "CP2102N config verification failed after locking");
    }
}

void CCP2102NDev::setSerNum( const std::vector<BYTE> &str, bool isAscii) const
{
    // For CP2102N do nothing because serial number, etc. is contained in the config.
	(void)(str);
	(void)(isAscii);
}

//---------------------------------------------------------------------------------
// Here is a bunch of customization parametersfound in cp210x devices. They are included
// into each individual cp210x device that supports the parameter.
//---------------------------------------------------------------------------------
struct CFlushBufferConfig
{
    CFlushBufferConfig() { m_Specified  = false; }
    bool readParm( const std::string &parmName);
    void program( const CCP210xDev &dev) const;
    void verify( const CCP210xDev &dev) const;
private:
    bool m_Specified;
    WORD m_Config = 0;
};

bool CFlushBufferConfig::readParm( const std::string &parmName)
{
    if( parmName == "FlushBufferConfig")
    {
        setSpecified( m_Specified, parmName);
        m_Config = readUshortParm();
        readKeyword( "}"); // end of parameter list
        return true;
    }
    return false;
}

void CFlushBufferConfig::program( const CCP210xDev &dev) const
{
    if( !m_Specified) { return; }
    dev.setFlushBufCfg( m_Config);
}

void CFlushBufferConfig::verify( const CCP210xDev &dev) const
{
    if( !m_Specified) { return; }
    if( m_Config != dev.getFlushBufCfg())
    {
        throw CCustErr( "Failed FlushBufferConfig verification");
    }
}

//---------------------------------------------------------------------------------
struct CDeviceMode
{
    CDeviceMode() { m_Specified  = false; }
    bool readParm( const std::string &parmName);
    void program( const CCP210xDev &dev) const;
    void verify( const CCP210xDev &dev) const;
private:
    bool m_Specified;
    BYTE m_ModeECI = 0;
    BYTE m_ModeSCI = 0;
};

bool CDeviceMode::readParm( const std::string &parmName)
{
    if( parmName == "DeviceMode")
    {
        setSpecified( m_Specified, parmName);
        m_ModeECI = readUcharParm();
        m_ModeSCI = readUcharParm();
        readKeyword( "}"); // end of parameter list
        return true;
    }
    return false;
}

void CDeviceMode::program( const CCP210xDev &dev) const
{
    if( !m_Specified) { return; }
    AbortOnErr( CP210x_SetDeviceMode( dev.handle(), m_ModeECI, m_ModeSCI), "CP210x_SetDeviceMode");
}

void CDeviceMode::verify( const CCP210xDev &dev) const
{
    if( !m_Specified) { return; }
    BYTE ModeECI;
    BYTE ModeSCI;
    AbortOnErr( CP210x_GetDeviceMode( dev.handle(), &ModeECI, &ModeSCI), "CP210x_GetDeviceMode");
    if( m_ModeECI != ModeECI || m_ModeSCI != ModeSCI)
    {
        throw CCustErr( "Failed DeviceMode verification");
    }
}

//---------------------------------------------------------------------------------
struct CInterfaceString
{
    CInterfaceString() { m_Specified  = false; }
    bool readParm( BYTE ifc, const std::string &parmName);
    void program( BYTE ifc, const CCP210xDev &dev) const;
    void verify( BYTE ifc, const CCP210xDev &dev) const;
private:
    bool m_Specified;
    bool m_IsAscii = false;
    std::vector<BYTE>  m_str;
};

bool CInterfaceString::readParm( BYTE ifc, const std::string &parmName)
{
    char ifcDigit = '0' + ifc;
    if( parmName == std::string( "InterfaceStringAscii") + ifcDigit)
    {
        setSpecified( m_Specified, parmName);
        m_IsAscii    = true;
        readByteArrayParm( m_str, MAX_UCHAR);
        readKeyword( "}"); // end of parameter list
        return true;
    }
    else if( parmName == std::string("InterfaceStringUnicode") + ifcDigit)
    {
        setSpecified( m_Specified, parmName);
        m_IsAscii    = false;
        readByteArrayParm( m_str, MAX_UCHAR);
        readKeyword( "}"); // end of parameter list
        return true;
    }
    return false;
}

void CInterfaceString::program( BYTE ifc, const CCP210xDev &dev) const
{
    if( !m_Specified) { return; }
    BYTE CchStr = static_cast<BYTE> ( m_str.size() / (m_IsAscii ? 1 : 2));
    AbortOnErr( CP210x_SetInterfaceString( dev.handle(), ifc, const_cast<BYTE*>( m_str.data()), CchStr, m_IsAscii), "CP210x_SetInterfaceString");
}

void CInterfaceString::verify( BYTE ifc, const CCP210xDev &dev) const
{
    if( !m_Specified) { return; }
    std::vector<BYTE> str( MAX_UCHAR);
    BYTE CchStr = 0;
    AbortOnErr( CP210x_GetDeviceInterfaceString( dev.handle(), ifc, str.data(), &CchStr, m_IsAscii), "CP210x_GetDeviceInterfaceString");
    str.resize((size_t)CchStr * (m_IsAscii ? 1 : 2));
    if( m_str != str)
    {
        throw CCustErr( "Failed InterfaceString verification");
    }
}

//---------------------------------------------------------------------------------
struct CBaudRateConfig
{
    CBaudRateConfig() { m_Specified = false; memset(m_Config, 0, sizeof m_Config); }
    bool readParm( const std::string &parmName);
    void program( const CCP210xDev &dev) const;
    void verify( const CCP210xDev &dev) const;
private:
    bool m_Specified;
    BAUD_CONFIG m_Config[ NUM_BAUD_CONFIGS];
};

bool CBaudRateConfig::readParm( const std::string &parmName)
{
    if( parmName == "BaudRateConfig")
    {
        setSpecified( m_Specified, parmName);
        readKeyword( "{");
        for( DWORD i = 0; i < SIZEOF_ARRAY( m_Config); i++)
        {
            m_Config[ i].BaudGen       = readUshort();
            m_Config[ i].Timer0Reload  = readUshort();
            m_Config[ i].Prescaler     = readUchar();
            m_Config[ i].BaudRate      = readUlong();
        }
        readKeyword( "}");
        readKeyword( "}"); // end of parameter list
        return true;
    }
    return false;
}

void CBaudRateConfig::program( const CCP210xDev &dev) const
{
    if( !m_Specified) { return; }
    AbortOnErr( CP210x_SetBaudRateConfig( dev.handle(), const_cast<BAUD_CONFIG*>(&m_Config[ 0])), "CP210x_SetBaudRateConfig");
}

void CBaudRateConfig::verify( const CCP210xDev &dev) const
{
    if( !m_Specified) { return; }
    BAUD_CONFIG Config[ NUM_BAUD_CONFIGS];
    AbortOnErr( CP210x_GetBaudRateConfig( dev.handle(), &Config[ 0]), "CP210x_GetBaudRateConfig");
    for( DWORD i = 0; i < SIZEOF_ARRAY( m_Config); i++)
    {
        if( m_Config[ i].BaudGen       != Config[ i].BaudGen      ||
            m_Config[ i].Timer0Reload  != Config[ i].Timer0Reload ||
            m_Config[ i].Prescaler     != Config[ i].Prescaler    ||
            m_Config[ i].BaudRate      != Config[ i].BaudRate)
        {
            throw CCustErr( "Failed BaudRateConfig verification");
        }
    }
}

//---------------------------------------------------------------------------------
struct CPortConfig
{
    CPortConfig() { m_Specified = false; memset(&m_PortCfg, 0, sizeof m_PortCfg); }
    bool readParm( const std::string &parmName);
    void program( const CCP210xDev &dev) const;
    void verify( const CCP210xDev &dev) const;
private:
    bool m_Specified;
    PORT_CONFIG m_PortCfg;
};

bool CPortConfig::readParm( const std::string &parmName)
{
    if( parmName == "PortConfig")
    {
        setSpecified( m_Specified, parmName);
        readKeyword( "{");
        m_PortCfg.Mode           = readUshort();
        m_PortCfg.Reset_Latch    = readUshort();
        m_PortCfg.Suspend_Latch  = readUshort();
        m_PortCfg.EnhancedFxn    = readUchar();
        readKeyword( "}");
        readKeyword( "}"); // end of parameter list
        return true;
    }
    return false;
}

void CPortConfig::program( const CCP210xDev &dev) const
{
    if( !m_Specified) { return; }
    AbortOnErr( CP210x_SetPortConfig( dev.handle(), const_cast<PORT_CONFIG*>(&m_PortCfg)), "CP210x_SetPortConfig");
}

void CPortConfig::verify( const CCP210xDev &dev) const
{
    if( !m_Specified) { return; }
    PORT_CONFIG PortCfg;
    AbortOnErr( CP210x_GetPortConfig( dev.handle(), &PortCfg), "CP210x_GetPortConfig");
    if( m_PortCfg.Mode          != PortCfg.Mode ||
        m_PortCfg.Reset_Latch   != PortCfg.Reset_Latch ||
        m_PortCfg.Suspend_Latch != PortCfg.Suspend_Latch ||
        m_PortCfg.EnhancedFxn   != PortCfg.EnhancedFxn)
    {
        throw CCustErr( "Failed PortConfig verification");
    }
}

//---------------------------------------------------------------------------------
struct CDualPortConfig
{
    CDualPortConfig() { m_Specified = false; memset(&m_PortCfg, 0, sizeof m_PortCfg); }
    bool readParm( const std::string &parmName);
    void program( const CCP210xDev &dev) const;
    void verify( const CCP210xDev &dev) const;
private:
    bool m_Specified;
    DUAL_PORT_CONFIG m_PortCfg;
};

bool CDualPortConfig::readParm( const std::string &parmName)
{
    if( parmName == "DualPortConfig")
    {
        setSpecified( m_Specified, parmName);
        readKeyword( "{");
        m_PortCfg.Mode           = readUshort();
        m_PortCfg.Reset_Latch    = readUshort();
        m_PortCfg.Suspend_Latch  = readUshort();
        m_PortCfg.EnhancedFxn_ECI    = readUchar();
        m_PortCfg.EnhancedFxn_SCI    = readUchar();
        m_PortCfg.EnhancedFxn_Device = readUchar();
        readKeyword( "}");
        readKeyword( "}"); // end of parameter list
        return true;
    }
    return false;
}

void CDualPortConfig::program( const CCP210xDev &dev) const
{
    if( !m_Specified) { return; }
    AbortOnErr( CP210x_SetDualPortConfig( dev.handle(), const_cast<DUAL_PORT_CONFIG*>(&m_PortCfg)), "CP210x_SetDualPortConfig");
}

void CDualPortConfig::verify( const CCP210xDev &dev) const
{
    if( !m_Specified) { return; }
    DUAL_PORT_CONFIG PortCfg;
    AbortOnErr( CP210x_GetDualPortConfig( dev.handle(), &PortCfg), "CP210x_GetDualPortConfig");
    if( m_PortCfg.Mode               != PortCfg.Mode ||
        m_PortCfg.Reset_Latch        != PortCfg.Reset_Latch ||
        m_PortCfg.Suspend_Latch      != PortCfg.Suspend_Latch ||
        m_PortCfg.EnhancedFxn_ECI    != PortCfg.EnhancedFxn_ECI ||
        m_PortCfg.EnhancedFxn_SCI    != PortCfg.EnhancedFxn_SCI ||
        m_PortCfg.EnhancedFxn_Device != PortCfg.EnhancedFxn_Device)
    {
        throw CCustErr( "Failed DualPortConfig verification");
    }
}

//---------------------------------------------------------------------------------
struct CQuadPortConfig
{
    CQuadPortConfig() { m_Specified = false; memset(&m_PortCfg, 0, sizeof m_PortCfg); }
    bool readParm( const std::string &parmName);
    void program( const CCP210xDev &dev) const;
    void verify( const CCP210xDev &dev) const;
private:
    bool m_Specified;
    QUAD_PORT_CONFIG m_PortCfg;
};

void readQuadPortState( QUAD_PORT_STATE &qps)
{
    qps.Mode_PB0     = readUshort();
    qps.Mode_PB1     = readUshort();
    qps.Mode_PB2     = readUshort();
    qps.Mode_PB3     = readUshort();
    qps.Mode_PB4     = readUshort();
    qps.LowPower_PB0 = readUshort();
    qps.LowPower_PB1 = readUshort();
    qps.LowPower_PB2 = readUshort();
    qps.LowPower_PB3 = readUshort();
    qps.LowPower_PB4 = readUshort();
    qps.Latch_PB0    = readUshort();
    qps.Latch_PB1    = readUshort();
    qps.Latch_PB2    = readUshort();
    qps.Latch_PB3    = readUshort();
    qps.Latch_PB4    = readUshort();
}

bool CQuadPortConfig::readParm( const std::string &parmName)
{
    if( parmName == "QuadPortConfig")
    {
        setSpecified( m_Specified, parmName);
        readKeyword( "{");
        readQuadPortState( m_PortCfg.Reset_Latch);
        readQuadPortState( m_PortCfg.Suspend_Latch);
        m_PortCfg.IPDelay_IFC0        = readUchar();
        m_PortCfg.IPDelay_IFC1        = readUchar();
        m_PortCfg.IPDelay_IFC2        = readUchar();
        m_PortCfg.IPDelay_IFC3        = readUchar();
        m_PortCfg.EnhancedFxn_IFC0    = readUchar();
        m_PortCfg.EnhancedFxn_IFC1    = readUchar();
        m_PortCfg.EnhancedFxn_IFC2    = readUchar();
        m_PortCfg.EnhancedFxn_IFC3    = readUchar();
        m_PortCfg.EnhancedFxn_Device  = readUchar();
        m_PortCfg.ExtClk0Freq         = readUchar();
        m_PortCfg.ExtClk1Freq         = readUchar();
        m_PortCfg.ExtClk2Freq         = readUchar();
        m_PortCfg.ExtClk3Freq         = readUchar();
        readKeyword( "}");
        readKeyword( "}"); // end of parameter list
        return true;
    }
    return false;
}

void CQuadPortConfig::program( const CCP210xDev &dev) const
{
    if( !m_Specified) { return; }
    AbortOnErr( CP210x_SetQuadPortConfig( dev.handle(), const_cast<QUAD_PORT_CONFIG*>(&m_PortCfg)), "CP210x_SetQuadPortConfig");
}

bool isEqualQuadPortState( const QUAD_PORT_STATE &qps1, const QUAD_PORT_STATE &qps2)
{
    if( qps1.Mode_PB0     != qps2.Mode_PB0     ||
        qps1.Mode_PB1     != qps2.Mode_PB1     ||
        qps1.Mode_PB2     != qps2.Mode_PB2     ||
        qps1.Mode_PB3     != qps2.Mode_PB3     ||
        qps1.Mode_PB4     != qps2.Mode_PB4     ||
        qps1.LowPower_PB0 != qps2.LowPower_PB0 ||
        qps1.LowPower_PB1 != qps2.LowPower_PB1 ||
        qps1.LowPower_PB2 != qps2.LowPower_PB2 ||
        qps1.LowPower_PB3 != qps2.LowPower_PB3 ||
        qps1.LowPower_PB4 != qps2.LowPower_PB4 ||
        qps1.Latch_PB0    != qps2.Latch_PB0    ||
        qps1.Latch_PB1    != qps2.Latch_PB1    ||
        (qps1.Latch_PB2 & 0xFFFC)    != (qps2.Latch_PB2  & 0xFFFC)    ||
        qps1.Latch_PB3    != qps2.Latch_PB3    ||
        qps1.Latch_PB4    != qps2.Latch_PB4)
    {
        return false;
    }
    return true;
}

void CQuadPortConfig::verify( const CCP210xDev &dev) const
{
    if( !m_Specified) { return; }
    QUAD_PORT_CONFIG PortCfg;
    AbortOnErr( CP210x_GetQuadPortConfig( dev.handle(), &PortCfg), "CP210x_GetQuadPortConfig");
    if( !isEqualQuadPortState( m_PortCfg.Reset_Latch, PortCfg.Reset_Latch ) ||
        !isEqualQuadPortState( m_PortCfg.Suspend_Latch, PortCfg.Suspend_Latch ) ||
        m_PortCfg.IPDelay_IFC0        != PortCfg.IPDelay_IFC0       ||
        m_PortCfg.IPDelay_IFC1        != PortCfg.IPDelay_IFC1       ||
        m_PortCfg.IPDelay_IFC2        != PortCfg.IPDelay_IFC2       ||
        m_PortCfg.IPDelay_IFC3        != PortCfg.IPDelay_IFC3       ||
        m_PortCfg.EnhancedFxn_IFC0    != PortCfg.EnhancedFxn_IFC0   ||
        m_PortCfg.EnhancedFxn_IFC1    != PortCfg.EnhancedFxn_IFC1   ||
        m_PortCfg.EnhancedFxn_IFC2    != PortCfg.EnhancedFxn_IFC2   ||
        m_PortCfg.EnhancedFxn_IFC3    != PortCfg.EnhancedFxn_IFC3   ||
        m_PortCfg.EnhancedFxn_Device  != PortCfg.EnhancedFxn_Device ||
        m_PortCfg.ExtClk0Freq         != PortCfg.ExtClk0Freq        ||
        m_PortCfg.ExtClk1Freq         != PortCfg.ExtClk1Freq        ||
        m_PortCfg.ExtClk2Freq         != PortCfg.ExtClk2Freq        ||
        m_PortCfg.ExtClk3Freq         != PortCfg.ExtClk3Freq)
    {
        throw CCustErr( "Failed QuadPortConfig verification");
    }
}

//---------------------------------------------------------------------------------
// This CConfig struct is used only for the CP2102N
struct CConfig
{
    CConfig() { m_Specified = false; memset(&m_CP2102NConfig, 0, sizeof m_CP2102NConfig); }
    bool readParm( const std::string &parmName);
    void program( const CCP210xDev &dev) const;
    void verify( const CCP210xDev &dev) const;
private:
    bool m_Specified;
    CCP2102NConfig m_CP2102NConfig;

  friend class CCP2102NParms;
};

bool CConfig::readParm( const std::string &parmName)
{
    // The only parameter supported is 'Config'
    if( parmName == "Config")
    {
		// Throws exception if m_Specified is already true
        setSpecified( m_Specified, parmName);

        std::vector<BYTE> Config;

		// Jeff said it's best to always read/write the whole config, hence "Exact" read
        readByteArrayParmExact( Config, sizeof(m_CP2102NConfig));

        readKeyword( "}"); // end of parameter list
        std::memcpy( &m_CP2102NConfig.Raw[0], &Config[0], sizeof(m_CP2102NConfig)); // Copy into the formatted storage

        // Few sanity checks
        if(m_CP2102NConfig.Fields.configVersion != CP2102N_CONFIG_VERSION)
        {
            throw CUsageErr( "CP2102N Config::configVersion is invalid");
        }
        if(m_CP2102NConfig.Fields.configSize != sizeof(CCP2102NConfig))
        {
            throw CUsageErr( "CP2102N Config::configSize is invalid");
        }
        if(m_CP2102NConfig.Fields.enableConfigUpdate != CP2102N_CONFIG_UNLOCKED)
        {
            // The user isn't trying to lock the config by enableConfigUpdate. We don't allow this,
            // we lock it explicitly after verification, by writing same data with enableConfigUpdate
            // clear.
            throw CUsageErr( "CP2102N Config::enableConfigUpdate attempts to lock, use --lock instead");
        }
        return true;
    }
    return false;
}

void CConfig::program( const CCP210xDev &dev) const
{
	if (!m_Specified)
	{
		return;
	}
    AbortOnErr( CP210x_SetConfig( dev.handle(), const_cast<BYTE*>( &m_CP2102NConfig.Raw[0]), static_cast<WORD>( sizeof(m_CP2102NConfig.Raw))), "CP210x_SetConfig");
}

void CConfig::verify( const CCP210xDev &dev) const
{
    if( !m_Specified)
	{
		return;
	}
    CCP2102NConfig deviceConfig;

	AbortOnErr( CP210x_GetConfig( dev.handle(), &deviceConfig.Raw[0], static_cast<WORD>( sizeof( deviceConfig.Raw))), "CP210x_GetConfig");

	ASSERT(m_CP2102NConfig.Fields.enableConfigUpdate == CP2102N_CONFIG_UNLOCKED);

	// A little hack to workaround locked configurations.
	// If the Config on the chip is locked, the dumb array comparison will fail because of enableConfigUpdate.
	// But it wouldn't be a valid failure. So, hack the "unlocked" value into it before comparing.
	deviceConfig.Fields.enableConfigUpdate = CP2102N_CONFIG_UNLOCKED;

#if 1 // Compare byte-by-byte and display each discrepancy
	bool failed = false;
	for (size_t i = 0; i < sizeof(deviceConfig); i++)
	{
		if (deviceConfig.Raw[i] != m_CP2102NConfig.Raw[i])
		{
			printf("ERROR: config[%02lX]: DeviceConfig = %02X  ConfigFile = %02X\n", (long unsigned int)i, deviceConfig.Raw[i], m_CP2102NConfig.Raw[i]);
			failed = true;
		}
	}
	if(failed)
		throw CCustErr("Failed Config verification");
#else // ORIGINAL
    if( memcmp( &deviceConfig.Raw[0], &m_CP2102NConfig.Raw[0], sizeof( deviceConfig)))
    {
        throw CCustErr( "Failed Config verification");
    }
#endif
}

//---------------------------------------------------------------------------------
// Base class for all cp210x devices, contains ommmon customization parameters
//---------------------------------------------------------------------------------
template< class TDev >
struct CCP210xParms : public CDevParms<TDev>
{
    virtual bool supportsUnicode() const { return true; }
    virtual void readParm( const std::string &parmName);
    void program( const TDev &dev, const std::vector<BYTE> *pSerNum) const;
    void verify( const TDev &dev, CSerNumSet &serNumSet) const;
};

template< class TDev >
void CCP210xParms<TDev>::readParm( const std::string &parmName)
{
    CDevParms<TDev>::readParm( parmName);
}

template< class TDev >
void CCP210xParms<TDev>::program( const TDev &dev, const std::vector<BYTE> *pSerNum) const
{
    CDevParms<TDev>::program( dev, pSerNum);

    if( CDevParms<TDev>::m_VidPidSpecified)
    {
        dev.setVidPid( CDevParms<TDev>::m_Vid, CDevParms<TDev>::m_Pid);
    }
    if( CDevParms<TDev>::m_PowerModeSpecified)
    {
        dev.setPowerMode( CDevParms<TDev>::m_PowerMode);
    }
    if( CDevParms<TDev>::m_MaxPowerSpecified)
    {
        dev.setMaxPower( CDevParms<TDev>::m_MaxPower);
    }
    if( CDevParms<TDev>::m_DevVerSpecified)
    {
        dev.setDevVer( CDevParms<TDev>::m_DevVer);
    }
}

template< class TDev >
void CCP210xParms<TDev>::verify( const TDev &dev, CSerNumSet &serNumSet) const
{
    CDevParms<TDev>::verify( dev, serNumSet);
}

//---------------------------------------------------------------------------------
// CP2101 devices
//---------------------------------------------------------------------------------
struct CCP2101Parms : public CCP210xParms<CCP210xDev>
{
    virtual void readParm( const std::string &parmName);
    virtual void program( const CCP210xDev &dev, const std::vector<BYTE> *pSerNum) const;
    virtual void verify( const CCP210xDev &dev, CSerNumSet &serNumSet) const;
};

void CCP2101Parms::readParm( const std::string &parmName) 
{
    CCP210xParms::readParm( parmName);
}

void CCP2101Parms::program( const CCP210xDev &dev, const std::vector<BYTE> *pSerNum) const
{
    CCP210xParms::program( dev, pSerNum);
}

void CCP2101Parms::verify( const CCP210xDev &dev, CSerNumSet &serNumSet) const
{
    CCP210xParms::verify( dev, serNumSet);
}

//---------------------------------------------------------------------------------
// CP2102 devices
//---------------------------------------------------------------------------------

struct CCP2102Parms : public CCP210xParms<CCP210xDev>
{
    virtual void readParm( const std::string &parmName);
    virtual void program( const CCP210xDev &dev, const std::vector<BYTE> *pSerNum) const;
    virtual void verify( const CCP210xDev &dev, CSerNumSet &serNumSet) const;
private:
    CBaudRateConfig m_BaudRateCfg;
};

void CCP2102Parms::readParm( const std::string &parmName)
{
    if( m_BaudRateCfg.readParm( parmName))
    {
        return;
    }
    CCP210xParms::readParm( parmName);
}

void CCP2102Parms::program( const CCP210xDev &dev, const std::vector<BYTE> *pSerNum) const
{
    CCP210xParms::program( dev, pSerNum);
    m_BaudRateCfg.program( dev);
}

void CCP2102Parms::verify( const CCP210xDev &dev, CSerNumSet &serNumSet) const
{
    CCP210xParms::verify( dev, serNumSet);
    m_BaudRateCfg.verify( dev);
}

//---------------------------------------------------------------------------------
// CP2102N devices
//---------------------------------------------------------------------------------
class CCP2102NParms : public CCP210xParms<CCP2102NDev>
{
    virtual void readParm( const std::string &parmName);
    virtual void program( const CCP2102NDev &dev, const std::vector<BYTE> *pSerNum) const;
    virtual void verify( const CCP2102NDev &dev, CSerNumSet &serNumSet) const;
private:
    CConfig m_Cfg;
};

void CCP2102NParms::readParm( const std::string &parmName)
{
    if( m_Cfg.readParm( parmName))
    {
        return;
    }
    CCP210xParms::readParm( parmName);
}

void CCP2102NParms::program(const CCP2102NDev &dev, const std::vector<BYTE> * pSerNum) const
{
    const CCP2102NConfig & cp2102nconfigReference = m_Cfg.m_CP2102NConfig;
    CCP2102NConfig & cp2102nconfig = const_cast<CCP2102NConfig &>(cp2102nconfigReference);

    unsigned int offset_UseInternalSerialNumber = (CP2102N_PREAMBLE_SIZE + CP2102N_LANGDESC_SIZE + CP2102N_MANF_SIZE + CP2102N_PROD_SIZE);
    if (m_Cfg.m_CP2102NConfig.Raw[offset_UseInternalSerialNumber])
    {
        printf("CP2102N config specifies using Internal Serial Number\n");
    }
    else
    {
        if (pSerNum)
        {
            // Overwrite config with specified serial number
            size_t szSerialNumber = pSerNum->size() + 1;	// +1 should not really be necessary
            char* serialNumber = (char*)malloc(szSerialNumber);
            if (serialNumber != NULL) {
                memset(serialNumber, 0, szSerialNumber);
                strncpy(serialNumber, (const char*)&(*pSerNum)[0], pSerNum->size());
                setCP2102N_USBString(cp2102nconfig, CP2102N_SERIAL_NUMBER, serialNumber);
                free(serialNumber);
            }
        }
        else
        {
            printf("*************************************************************************************\n");
            printf("**********************************     WARNING:    **********************************\n");
            printf("*************************************************************************************\n");
            printf("* The '--serial-nums' option is not present, and the 'Use Internal Serial Number'   *\n");
            printf("* element of the CP2102N config file is not set.                                    *\n");
            printf("* Therefore device(s) will be programmed with the S/N contained in the config file, *\n");
            printf("* and all devices programmed with this config file will have the same serial number.*\n");
            printf("* IT IS NOT RECOMMENDED FOR MULTIPLE USB DEVICES TO HAVE THE SAME SERIAL NUMBER.    *\n");
            printf("* Recommended that one of the following methods be used to ensure unique S/Ns:      *\n");
            printf("*   - Enable the 'Use Internal Serial Number' element in the CP2102N config file.   *\n");
            printf("*   - Use the '--serial-nums { GUID }' option.                                      *\n");
            printf("*   - Use the '--serial-nums { xxx yyy zzz }' option, with unique xxx, yyy and zzz. *\n");
            printf("*************************************************************************************\n");
        }
    }
    CCP210xParms::program(dev, pSerNum);
    m_Cfg.program(dev);
}

void CCP2102NParms::verify(const CCP2102NDev &dev, CSerNumSet &serNumSet) const
{
    CCP210xParms::verify(dev, serNumSet);

    // Check the state of the config file's 'Use Internal Serial Number' element
    const CCP2102NConfig & cp2102nconfigReference = m_Cfg.m_CP2102NConfig;
    CCP2102NConfig & cp2102nconfig = const_cast<CCP2102NConfig &>(cp2102nconfigReference);
    unsigned int offset_UseInternalSerialNumber = (CP2102N_PREAMBLE_SIZE + CP2102N_LANGDESC_SIZE + CP2102N_MANF_SIZE + CP2102N_PROD_SIZE);
    bool useInternal = m_Cfg.m_CP2102NConfig.Raw[offset_UseInternalSerialNumber] ? true : false;

    // MCUSW-418: Overwrite config S/N only if not using the internal S/N, and the --serial-nums' option was specified
    if (!useInternal && serNumSet.m_matchedSN.size())
    {
        char * serialNumber = (char*)malloc(CP2102N_SERNUM_SIZE);
        if (serialNumber != NULL) {
            memset(serialNumber, 0, CP2102N_SERNUM_SIZE);
            strncpy(serialNumber, (const char*)&(serNumSet.m_matchedSN[0]), serNumSet.m_matchedSN.size());
            setCP2102N_USBString(cp2102nconfig, CP2102N_SERIAL_NUMBER, serialNumber);
            free(serialNumber);
        }
    }
    else
    {
        printf("WARNING: '--serial-nums' option not specified - device S/N will be compared with config file S/N.\n");
    }
	m_Cfg.verify(dev);
}

/*
  setCP2102N_USBString(): Integrates a string into the config for the CP2102N.
 */
void setCP2102N_USBString(CCP2102NConfig & config, cp2102n_usb_string_desc_t stringToSet, const char * value)
{
    unsigned int kPreamble  = CP2102N_PREAMBLE_SIZE;
    unsigned int kLang_desc = CP2102N_LANGDESC_SIZE;
    unsigned int kMfrDesc	= CP2102N_MANF_SIZE;
    unsigned int kProdDesc  = CP2102N_PROD_SIZE;
    unsigned int kSerDesc	= CP2102N_SERNUM_SIZE;
    unsigned int kPostAmble = CP2102N_POSTAMBLE_SIZE;
    unsigned int kChecksum  = CP2102N_CHECKSUM_SIZE;

    unsigned int stringPos;
    unsigned int destLen;

    switch (stringToSet)
    {
        case CP2102N_SERIAL_NUMBER:
            stringPos = kPreamble + kLang_desc + kMfrDesc + kProdDesc;
            destLen = kSerDesc;
            break;
        case CP2102N_PRODUCT_STRING:
            stringPos = kPreamble + kLang_desc + kMfrDesc;
            destLen = kProdDesc;
            break;
        case CP2102N_MANUFACTURER_STRING:
            stringPos = kPreamble + kLang_desc;
            destLen = kMfrDesc;
            break;
        default:
	        throw CSyntErr("setCP2102N_USBString(): Invalid string enumeration");
    }

    unsigned int checksumPos = kPreamble + kLang_desc + kMfrDesc + kProdDesc + kSerDesc + kPostAmble;

    // Clear the destination area of the config
    memset(config.Raw + stringPos, 0, destLen);

    // Convert the value param into a USB descriptor.
    std::string strValue(value);
    std::vector<unsigned short> myStrValue16;
    utf8::utf8to16(strValue.begin(), strValue.end(), back_inserter(myStrValue16));

    unsigned short expandedStrLen = (myStrValue16.size() & 0xFFFF) * 2;
    unsigned short descriptorLen = expandedStrLen + 2;	// +2 for bLength, bDescriptorType

    // The '+1' below accounts for the first byte that indicates UTF16 (0) or packed UTF16 (1)
    if (((unsigned int)descriptorLen > 255) || ((unsigned int)(descriptorLen + 1) > destLen))
    {
	    // throw an exception. The desc is over the size limit.
	    throw CSyntErr("USB String Descriptor is too large.");
    }

    // The first byte of the string indicates the format (this byte is not transmitted over USB).
    config.Raw[stringPos] = 0x00;	// 0: unpacked UTF-16LE format

    // string[1]= bLength
    config.Raw[stringPos + 1] = (BYTE)(descriptorLen & 0x00FF);

    // string[2]= bDescriptorType
    config.Raw[stringPos + 2] = (BYTE)0x03;	// 0x03: String descriptor

    // Copy string
    BYTE * b = (BYTE *)&myStrValue16[0];
    int iDest = stringPos + 3;
    for (int i = 0; i < expandedStrLen; ++i)
    {
	    config.Raw[iDest] = b[i];
	    ++iDest;
    }

    // Compute and write the checksum.
    unsigned short chksumLength = (unsigned short)((0xFFFF & sizeof(config.Raw)) - kChecksum);
    unsigned short chksum = fletcher16(&config.Raw[0], chksumLength);
    config.Raw[checksumPos] = (chksum & 0xFF00) >> 8;
    config.Raw[checksumPos+1] = chksum & 0xFF;
}

//---------------------------------------------------------------------------------
// CP2103 devices
//---------------------------------------------------------------------------------
struct CCP2103Parms : public CCP210xParms<CCP210xDev>
{
    virtual void readParm( const std::string &parmName);
    virtual void program( const CCP210xDev &dev, const std::vector<BYTE> *pSerNum) const;
    virtual void verify( const CCP210xDev &dev, CSerNumSet &serNumSet) const;
private:
    CBaudRateConfig  m_BaudRateCfg;
    CPortConfig      m_PortCfg;
};

void CCP2103Parms::readParm( const std::string &parmName)
{
    if( m_PortCfg.readParm( parmName))
    {
        return;
    }
    if( m_BaudRateCfg.readParm( parmName))
    {
        return;
    }
    CCP210xParms::readParm( parmName);
}

void CCP2103Parms::program( const CCP210xDev &dev, const std::vector<BYTE> *pSerNum) const
{
    CCP210xParms::program( dev, pSerNum);
    m_BaudRateCfg.program( dev);
    m_PortCfg.program( dev);
}

void CCP2103Parms::verify( const CCP210xDev &dev, CSerNumSet &serNumSet) const
{
    CCP210xParms::verify( dev, serNumSet);
    m_BaudRateCfg.verify( dev);
    m_PortCfg.verify( dev);
}

//---------------------------------------------------------------------------------
// CP2104 devices
//---------------------------------------------------------------------------------
struct CCP2104Parms : public CCP210xParms<CCP210xDev>
{
    virtual void readParm( const std::string &parmName);
    virtual void program( const CCP210xDev &dev, const std::vector<BYTE> *pSerNum) const;
    virtual void verify( const CCP210xDev &dev, CSerNumSet &serNumSet) const;
private:
    CPortConfig          m_PortCfg;
    CFlushBufferConfig   m_FlushBufferConfig;
};

void CCP2104Parms::readParm( const std::string &parmName)
{
    if( m_PortCfg.readParm( parmName))
    {
        return;
    }
    if( m_FlushBufferConfig.readParm( parmName))
    {
        return;
    }
    CCP210xParms::readParm( parmName);
}

void CCP2104Parms::program( const CCP210xDev &dev, const std::vector<BYTE> *pSerNum) const
{
    CCP210xParms::program( dev, pSerNum);
    m_PortCfg.program( dev);
    m_FlushBufferConfig.program( dev);
}

void CCP2104Parms::verify( const CCP210xDev &dev, CSerNumSet &serNumSet) const
{
    CCP210xParms::verify( dev, serNumSet);
    m_PortCfg.verify( dev);
    m_FlushBufferConfig.verify( dev);
}

//---------------------------------------------------------------------------------
// CP2105 devices
//---------------------------------------------------------------------------------
struct CCP2105Parms : public CCP210xParms<CCP210xDev>
{
    virtual void readParm( const std::string &parmName);
    virtual void program( const CCP210xDev &dev, const std::vector<BYTE> *pSerNum) const;
    virtual void verify( const CCP210xDev &dev, CSerNumSet &serNumSet) const;
private:
    CFlushBufferConfig  m_FlushBufferConfig;
    CDeviceMode         m_DeviceMode;
    CDualPortConfig     m_PortCfg;
    CInterfaceString    m_IfcStr[ 2];
};

void CCP2105Parms::readParm( const std::string &parmName)
{
    if( m_FlushBufferConfig.readParm( parmName))
    {
        return;
    }
    if( m_DeviceMode.readParm( parmName))
    {
        return;
    }
    if( m_PortCfg.readParm( parmName))
    {
        return;
    }
    for( BYTE i = 0; i < SIZEOF_ARRAY( m_IfcStr); i++)
    {
        if( m_IfcStr[ i].readParm( i, parmName))
        {
            return;
        }
    }
    CCP210xParms::readParm( parmName);
}

void CCP2105Parms::program( const CCP210xDev &dev, const std::vector<BYTE> *pSerNum) const
{
    CCP210xParms::program( dev, pSerNum);
    m_FlushBufferConfig.program( dev);
    m_DeviceMode.program( dev);
    m_PortCfg.program( dev);
    for( BYTE i = 0; i < SIZEOF_ARRAY( m_IfcStr); i++)
    {
        m_IfcStr[ i].program( i, dev);
    }
}

void CCP2105Parms::verify( const CCP210xDev &dev, CSerNumSet &serNumSet) const
{
    CCP210xParms::verify( dev, serNumSet);
    m_FlushBufferConfig.verify( dev);
    m_DeviceMode.verify( dev);
    m_PortCfg.verify( dev);
    for( BYTE i = 0; i < SIZEOF_ARRAY( m_IfcStr); i++)
    {
        m_IfcStr[ i].verify( i, dev);
    }
}

//---------------------------------------------------------------------------------
// CP2108 devices
//---------------------------------------------------------------------------------
struct CCP2108Parms : public CCP210xParms<CCP210xDev>
{
    CCP2108Parms() : m_ManufStr( true /*supportsUnicode*/) {}
    virtual void readParm( const std::string &parmName);
    virtual void program( const CCP210xDev &dev, const std::vector<BYTE> *pSerNum) const;
    virtual void verify( const CCP210xDev &dev, CSerNumSet &serNumSet) const;
private:
    CFlushBufferConfig               m_FlushBufferConfig;
    CManufacturerString<CCP210xDev>  m_ManufStr;
    CQuadPortConfig                  m_PortCfg;
    CInterfaceString                 m_IfcStr[ 4];
};

void CCP2108Parms::readParm( const std::string &parmName) 
{
    if( m_FlushBufferConfig.readParm( parmName))
    {
        return;
    }
    if( m_ManufStr.readParm( parmName))
    {
        return;
    }
    if( m_PortCfg.readParm( parmName))
    {
        return;
    }
    for( BYTE i = 0; i < SIZEOF_ARRAY( m_IfcStr); i++)
    {
        if( m_IfcStr[ i].readParm( i, parmName))
        {
            return;
        }
    }
    CCP210xParms::readParm( parmName);
}

void CCP2108Parms::program( const CCP210xDev &dev, const std::vector<BYTE> *pSerNum) const
{
    CCP210xParms::program( dev, pSerNum);
    m_FlushBufferConfig.program( dev);
    m_ManufStr.program( dev);
    m_PortCfg.program( dev);
    for( BYTE i = 0; i < SIZEOF_ARRAY( m_IfcStr); i++)
    {
        m_IfcStr[ i].program( i, dev);
    }
}

void CCP2108Parms::verify( const CCP210xDev &dev, CSerNumSet &serNumSet) const
{
    CCP210xParms::verify( dev, serNumSet);
    m_FlushBufferConfig.verify( dev);
    m_ManufStr.verify( dev);
    m_PortCfg.verify( dev);
    for( BYTE i = 0; i < SIZEOF_ARRAY( m_IfcStr); i++)
    {
        m_IfcStr[ i].verify( i, dev);
    }
}

//---------------------------------------------------------------------------------
// CP2109 devices
//---------------------------------------------------------------------------------
struct CCP2109Parms : public CCP210xParms<CCP210xDev>
{
    virtual void readParm( const std::string &parmName);
    virtual void program( const CCP210xDev &dev, const std::vector<BYTE> *pSerNum) const;
    virtual void verify( const CCP210xDev &dev, CSerNumSet &serNumSet) const;
private:
    CBaudRateConfig m_BaudRateCfg;
};

void CCP2109Parms::readParm( const std::string &parmName) 
{
    if( m_BaudRateCfg.readParm( parmName))
    {
        return;
    }
    CCP210xParms::readParm( parmName);
}

void CCP2109Parms::program( const CCP210xDev &dev, const std::vector<BYTE> *pSerNum) const
{
    CCP210xParms::program( dev, pSerNum);
    m_BaudRateCfg.program( dev);
}

void CCP2109Parms::verify( const CCP210xDev &dev, CSerNumSet &serNumSet) const
{
    CCP210xParms::verify( dev, serNumSet);
    m_BaudRateCfg.verify( dev);
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
void LibSpecificMain( const CDevType &devType, const CVidPid &vidPid, int argc, const char * argv[])
{
    if( devType.Value() == CP210x_CP2101_VERSION)
    {
        DevSpecificMain<CCP210xDev,CCP2101Parms> ( devType, vidPid, argc, argv);
    }
    else if( devType.Value() == CP210x_CP2102_VERSION)
    {
        DevSpecificMain<CCP210xDev,CCP2102Parms> ( devType, vidPid, argc, argv);
    }
    else if( devType.Value() == CP210x_CP2102N_QFN28_VERSION ||
             devType.Value() == CP210x_CP2102N_QFN24_VERSION ||
             devType.Value() == CP210x_CP2102N_QFN20_VERSION)
    {
        DevSpecificMain<CCP2102NDev,CCP2102NParms> ( devType, vidPid, argc, argv);
    }
    else if( devType.Value() == CP210x_CP2103_VERSION)
    {
        DevSpecificMain<CCP210xDev,CCP2103Parms> ( devType, vidPid, argc, argv);
    }
    else if( devType.Value() == CP210x_CP2104_VERSION)
    {
        DevSpecificMain<CCP210xDev,CCP2104Parms> ( devType, vidPid, argc, argv);
    }
    else if( devType.Value() == CP210x_CP2105_VERSION)
    {
        DevSpecificMain<CCP210xDev,CCP2105Parms> ( devType, vidPid, argc, argv);
    }
    else if( devType.Value() == CP210x_CP2108_VERSION)
    {
        DevSpecificMain<CCP210xDev,CCP2108Parms> ( devType, vidPid, argc, argv);
    }
    else if( devType.Value() == CP210x_CP2109_VERSION)
    {
        DevSpecificMain<CCP210xDev,CCP2109Parms> ( devType, vidPid, argc, argv);
    }
    else
    {
        throw CSyntErr( "Unsupported PartNum");
    }
}
