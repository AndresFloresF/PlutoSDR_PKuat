  //
  //  cp2615smt.cpp
  //  cp2615smt
  //
  //  Created by Brant Merryman on 3/27/17.
  //
  //

#if defined(_WIN32)
#include <WinSock2.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>

#include "util.h"
#include "smt.h"
#include "cp2615.h"

#include "cp2615smt.h"
#include <cassert>
#include <stdlib.h>

void printProgDesc();
void printCmdLineHelp();

void LibSpecificMain( const CDevType &devType, const CVidPid &vidPid, int argc, const char * argv[])
{
  WORD vid = vidPid.m_Vid;
  WORD pid = vidPid.m_Pid;
  std::string devices;
  DWORD actualNumDevices = 0;
  DWORD programmedNumDevices = 0;
  DWORD verifiedNumDevices = 0;

#if defined(_WIN32)
    UNREFERENCED_PARAMETER(devType);
#endif // defined(_WIN32)

    { // --list
        if (isSpecified(argc, argv, "--list")) {
            throw CUsageErr("\nERROR: '--list' option is not supported for the CP2615 device.");
        }
    }
    { // --device-count
        if (isSpecified(argc, argv, "--device-count", devices)) {

            DWORD numDevices = (DWORD)atoi(devices.c_str());
            // check for correct number of devices

            HID_SMBUS_STATUS status = HidSmbus_GetNumDevices(&actualNumDevices, vid, pid);
            if (HID_SMBUS_SUCCESS != status) {
                char msg[128];
                sprintf(msg, "\nERROR: 'HidSmbus_GetNumDevices()' returned error 0x%02X", status);
                throw CUsageErr(msg);
            }
            if (numDevices != actualNumDevices) {
                char msg[128];
                sprintf(msg, "\n\nERROR: Specified '--device-count' (%d) doesn't match the number of devices returned by HidSmbus_GetNumDevices (%d)",
                    numDevices, actualNumDevices);
                throw CUsageErr(msg);
            }
        }
        else {
            throw CUsageErr("\nERROR: '--device-count' must be specified.");
        }
    }

    { // --set-config
      std::string config_file;
      if (isSpecified(argc, argv, "--set-config", config_file)) {

        for (unsigned int idevice = 0; idevice < actualNumDevices; ++idevice) {
          std::filebuf fb;
          if (fb.open (config_file.c_str(),std::ios::in))
          {
            std::istream is(&fb);
            HID_SMBUS_STATUS status = setConfig(idevice, vid, pid, is);
            fb.close();
            if (HID_SMBUS_SUCCESS != status) {
              throw CUsageErr("\nERROR: set-config failed.");
            }
            else{
              programmedNumDevices++;
            }
          }
        }

        if(actualNumDevices == programmedNumDevices) {
          printf("\nProgrammed %u devices: OK", actualNumDevices);
        }
        else {
          printf("\nFail! Programmed just %u/%u actual devices", programmedNumDevices, actualNumDevices);
        }
      }
    }

    { // -- verify-config
        std::string config_file;
        if (isSpecified(argc, argv, "--verify-config", config_file)) {
            for (unsigned int idevice = 0; idevice < actualNumDevices; ++idevice) {
                std::filebuf fb;
                if (fb.open(config_file.c_str(), std::ios::in)) {
                    std::istream is(&fb);
                    HID_SMBUS_STATUS status = verifyConfig(idevice, vid, pid, is);
                    fb.close();
                    if (HID_SMBUS_SUCCESS != status) {
                        throw CUsageErr("\nERROR: verify-config failed.");
                    }
                    else{
                      verifiedNumDevices++;
                    }
                }
            }
          if(actualNumDevices == verifiedNumDevices) {
            printf("Verified %u devices: OK\n", actualNumDevices);
          }
        }
    }

    { // --set-and-verify-config
        std::string config_file;
        if (isSpecified(argc, argv, "--set-and-verify-config", config_file)) {
            for (unsigned int idevice = 0; idevice < actualNumDevices; ++idevice) {
                std::filebuf set_fb;
                if (set_fb.open(config_file.c_str(), std::ios::in)) {
                    std::istream set_is(&set_fb);
                    HID_SMBUS_STATUS setStatus = setConfig(idevice, vid, pid, set_is);
                    set_fb.close();
                    if (HID_SMBUS_SUCCESS != setStatus) {
                        throw CUsageErr("\nERROR: set-config failed.");
                    }
                    else {
                      programmedNumDevices++;
                    }

                    if (g_EchoParserReads) { printf("\n"); } // Start verify block on new line

                    std::filebuf verify_fb;
                    if (verify_fb.open(config_file.c_str(), std::ios::in)) {
                        std::istream verify_is(&verify_fb);
                        HID_SMBUS_STATUS verifyStatus = verifyConfig(idevice, vid, pid, verify_is);
                        verify_fb.close();
                        if (HID_SMBUS_SUCCESS != verifyStatus) {
                            throw CUsageErr("\nERROR: verify-config failed.");
                        }
                        else {
                          verifiedNumDevices++;
                        }
                    }
                }
            }
            if(actualNumDevices == programmedNumDevices) {
              printf("Programmed %u devices: OK\n", actualNumDevices);
            }
            if(actualNumDevices == verifiedNumDevices) {
              printf("Verified %u devices: OK\n", actualNumDevices);
              printf("\n");
            }
        }
    }

    { // --set-user-profile
      std::string user_profile_file;
      if (isSpecified(argc, argv, "--set-user-profile", user_profile_file)) {
        for (unsigned int idevice = 0; idevice < actualNumDevices; ++idevice) {
          std::filebuf fb;
          if (fb.open(user_profile_file.c_str(),std::ios::in))
          {
            std::istream is(&fb);
            HID_SMBUS_STATUS status = setUserProfile(idevice, vid, pid,is);
            fb.close();
            if (status != HID_SMBUS_SUCCESS) {
              throw CUsageErr("\nERROR: set-user-profile failed.");
            }
          }
        }

      }
    }

    { // --verify-user-profile
      std::string user_profile_file;
      if (isSpecified(argc, argv, "--verify-user-profile", user_profile_file)) {
        for (unsigned int idevice = 0; idevice < actualNumDevices; ++idevice) {
          std::filebuf fb;
          if (fb.open(user_profile_file.c_str(),std::ios::in))
          {
            std::istream is(&fb);
            HID_SMBUS_STATUS status = verifyUserProfile(idevice, vid, pid,is);
            fb.close();

            if (status != HID_SMBUS_SUCCESS) {
              throw CUsageErr("\nERROR: 'verify-user-profile' operation failed.");
            }
          }
        }
      }
    }

    // Unimplemented options

    { // --reset
        std::string user_profile_file;
        if (isSpecified(argc, argv, "--reset", user_profile_file)) {
            throw CUsageErr("\nERROR: '--reset' option is not supported for the CP2615 device.");
        }
    }

    { // --lock
        if (isSpecified(argc, argv, "--lock")) {
            throw CUsageErr("\nERROR: '--lock' option is not supported for the CP2615 device.");
        }
    }

    { // --verify-locked-config
        if (isSpecified(argc, argv, "--verify-locked-config")) {
            throw CUsageErr("\nERROR: '--verify-locked-config' option is not supported for the CP2615 device.");
        }
    }

    { // --serial-nums
        if (isSpecified(argc, argv, "--serial-nums")) {
            throw CUsageErr("\nERROR: '--serial-nums' option is not supported for the CP2615 device.");
        }
    }


  return;
}

//#pragma mark -

/*

 What does 'verify' do?

 I think the best way will be to read the entire config, then validate the checksum.
 Read the supplied config, calculate the checksum, compare to its checksum, then compare the
 four checksums.

 */
HID_SMBUS_STATUS verifyConfig(DWORD DevIndex, WORD vid, WORD pid, std::istream & is )
{
  HID_SMBUS_STATUS status = HID_SMBUS_UNKNOWN_ERROR;
  BYTE * configFromDevice = NULL;
  std::vector<BYTE> configFromFile;
  std::string word;

  size_t szConfig = 0;

  status = readConfig(DevIndex, vid, pid, &configFromDevice, &szConfig);

  if (HID_SMBUS_SUCCESS == status) {
    // we read the config ok. We are on the hook to free configFromDevice (at the end of this block).

    // read the checksum and then compute the checksum.

    // verify the checksum.
    WORD deviceConfigBodySize = 0;
    WORD fileConfigBodySize = 0;
    {
      BYTE bodySizeH = configFromDevice[42];
      BYTE bodySizeL = configFromDevice[43];
      deviceConfigBodySize = (bodySizeH << 8) | bodySizeL;
    }

    WORD checksum_computed_from_device = ComputeChecksum_cp2615(configFromDevice + (CONFIG_PREFIX_SIZE - 2), 2 + deviceConfigBodySize);

    WORD checksum_orig_from_device = htons( *( WORD *)(configFromDevice + CONFIG_PREFIX_SIZE - 4) );

    while (readWord(word, is)) {
      if (0 == strcmp("Config", word.c_str())) {
        readKeyword("{", is);
        break;
      }
    }

    readByteArrayParm( configFromFile, MAX_USHORT, is);

    BYTE * pbConfigFromFile = &configFromFile[0];
    {
      BYTE bodySizeH = pbConfigFromFile[42];
      BYTE bodySizeL = pbConfigFromFile[43];
      fileConfigBodySize = (bodySizeH << 8) | bodySizeL;
    }
    WORD checksum_computed_from_file = ComputeChecksum_cp2615(pbConfigFromFile + (CONFIG_PREFIX_SIZE - 2), 2 + fileConfigBodySize);

    WORD checksum_orig_from_file = htons( *( WORD *)(pbConfigFromFile + CONFIG_PREFIX_SIZE - 4) );

    if ( checksum_computed_from_file == checksum_orig_from_file &&
        checksum_computed_from_device == checksum_orig_from_device &&
        checksum_computed_from_file == checksum_computed_from_device &&
        fileConfigBodySize == deviceConfigBodySize) {
      status = HID_SMBUS_SUCCESS;   // Record as success for now

      // Verify that the device's config matches the specified config file
      for (WORD i = 0; i < fileConfigBodySize; i++)
      {
        if (configFromFile[i] != configFromDevice[i])
        {
          status = HID_SMBUS_CHECKSUM_ERROR;
          break;
        }
      }
    } else {
      status = HID_SMBUS_CHECKSUM_ERROR;
    }

    free(configFromDevice);
  }

  return status;
}

/*
 Reads the config. Caller passes a pointer to a BYTE pointer. On return if status is
 success, outConfig points to a valid BYTE * which the caller must free.
 The config size is written into outConfigSz.
 */
HID_SMBUS_STATUS readConfig(DWORD DevIndex, WORD vid, WORD pid, BYTE ** outConfig, size_t * outConfigSz)
{
  WORD configBodySize = 0;
  HID_SMBUS_S0 hidStatus;
  BYTE mybuffer[RD_SIZE];
  BYTE bytesRead;
  bool header_read = false;
  DWORD szRead = 0;
  DWORD bytesToRead = RD_SEGMENT_SZ;
  HID_SMBUS_STATUS status = HID_SMBUS_UNKNOWN_ERROR;
  size_t DefaultBufferSize = 1024;
  unsigned char buf[16];
  memset(buf, 0, sizeof(buf));

  HID_SMBUS_DEVICE device = g_DevHandles.get( DevIndex, vid, pid);

  SetConfigMode(device);

  BYTE * configBuffer = (BYTE*)malloc( DefaultBufferSize + RD_SIZE );
  WORD readFromAddress = 0;

  while (szRead < bytesToRead) {
    BYTE readFromHi = (readFromAddress >> 8);
    BYTE readFromLo = (0x00FF & readFromAddress);

    buf[0] = readFromHi;
    buf[1] = readFromLo;

    status = HidSmbus_AddressReadRequest(device, SLAVE_ADDRESS, RD_SEGMENT_SZ, 2, buf );

    if (HID_SMBUS_SUCCESS != status) { throw CDllErr(); }

    readFromAddress += RD_SEGMENT_SZ;

    WaitForTransferStatusComplete(device);

    HidSmbus_ForceReadResponse(device, RD_SEGMENT_SZ);

    DWORD blockBytesToRead = szRead + RD_SEGMENT_SZ;

    if (blockBytesToRead > bytesToRead) {
      blockBytesToRead = bytesToRead;
    }

    while (szRead < blockBytesToRead) {
      memset(mybuffer, 0, RD_SIZE);

      status = HidSmbus_GetReadResponse(device, &hidStatus, mybuffer, RD_SIZE, &bytesRead);

      if (HID_SMBUS_SUCCESS != status) { throw CDllErr(); }

      if (! header_read ) {

        BYTE bodySizeH = mybuffer[42];
        BYTE bodySizeL = mybuffer[43];
        configBodySize = (bodySizeH << 8) | bodySizeL;

        bytesToRead = CONFIG_PREFIX_SIZE+configBodySize;

        if (bytesToRead > DefaultBufferSize) {
          free(configBuffer);
          configBuffer = (BYTE *) malloc(bytesToRead + RD_SIZE);
        }
        header_read = true;
      }
      memcpy(configBuffer + szRead, mybuffer, bytesRead);
      szRead += bytesRead;
    }
  }

  SetNormalMode(device);

  if (HID_SMBUS_SUCCESS != status) {
    free(configBuffer);
  } else {
    assert(outConfig);
    *outConfig = configBuffer;
    assert(outConfigSz);
    *outConfigSz = bytesToRead;
  }
  return status;
}

/*
 compare the entire user profile directly.
 */
HID_SMBUS_STATUS verifyUserProfile(DWORD DevIndex, WORD vid, WORD pid, std::istream & is )
{
  WORD userProfileSize = 0;
  HID_SMBUS_STATUS status = HID_SMBUS_UNKNOWN_ERROR;
  DWORD bytesToRead = 2;
  size_t DefaultBufferSize = 1024;
  HID_SMBUS_S0 hidStatus;

  BYTE mybuffer[RD_SIZE];
  BYTE bytesRead;
  bool header_read = false;
  DWORD szRead = 0;

  unsigned char buf[16];
  memset(buf, 0, sizeof(buf));

    // read in the user-profile from the disk.
  std::vector<BYTE> up;
  readByteArrayParm( up, MAX_USHORT, is);
  BYTE * user_profile_from_file = &up[0];

    // get the size;
  WORD sz_user_profile_from_file = htons( *((WORD *) user_profile_from_file));

    // read it from the device.
  HID_SMBUS_DEVICE device = g_DevHandles.get( DevIndex, vid, pid);
  BYTE * userProfileBuffer = (BYTE*)malloc( DefaultBufferSize + RD_SIZE );
  WORD readFromAddress = 0;

  WORD numBytesToRead = 2;

  while (szRead < bytesToRead) {
    status = HidSmbus_AddressReadRequest(device, USER_PROFILE_SLAVE_ADDRESS, numBytesToRead, 2, buf );

    if (HID_SMBUS_SUCCESS != status) { throw CDllErr(); }

    readFromAddress += numBytesToRead;

    WaitForTransferStatusComplete(device);

    HidSmbus_ForceReadResponse(device, numBytesToRead);

    DWORD blockBytesToRead = szRead + numBytesToRead;

    if (blockBytesToRead > bytesToRead) {
      blockBytesToRead = bytesToRead;
    }

    while (szRead < blockBytesToRead) {
      memset(mybuffer, 0, RD_SIZE);

      status = HidSmbus_GetReadResponse(device, &hidStatus, mybuffer, SILABS_CONVERT_LARGER_TO_8BITS(numBytesToRead), &bytesRead);

      if (HID_SMBUS_SUCCESS != status) { throw CDllErr(); }

      if (! header_read ) {

        BYTE bodySizeH = mybuffer[0];
        BYTE bodySizeL = mybuffer[1];
        userProfileSize = (bodySizeH << 8) | bodySizeL;

        bytesToRead = userProfileSize;

        if (bytesToRead > DefaultBufferSize) {
          free(userProfileBuffer);
          userProfileBuffer = (BYTE *) malloc(bytesToRead + RD_SIZE);
        }

        header_read = true;
      }
      
      memcpy(userProfileBuffer + szRead, mybuffer, bytesRead);
      szRead += bytesRead;
    }
  }

  if (sz_user_profile_from_file != userProfileSize) {
      // size mismatch
    return HID_SMBUS_SIZE_MISMATCH_ERROR;
  }

  for (int i = 0; i < userProfileSize; ++i) {
    if (userProfileBuffer[i] != user_profile_from_file[i]) {
      return HID_SMBUS_USER_PROFILE_ERROR;
    }
  }

  return status;
}

HID_SMBUS_STATUS setUserProfile(DWORD DevIndex, WORD vid, WORD pid, std::istream & is)
{
  HID_SMBUS_STATUS status = HID_SMBUS_UNKNOWN_ERROR;
  std::vector<BYTE> user_profile;
  BYTE flashKeyHi = (kFlashKeys >> 8);
  BYTE flashKeyLo = (0x00FF & kFlashKeys);

  readByteArrayParm( user_profile, MAX_USHORT, is);

  HID_SMBUS_DEVICE device = g_DevHandles.get( DevIndex, vid, pid);

  SetConfigMode(device);

    // send the config
  WORD writeAddress = 0;

  for (unsigned int index = 0; index < user_profile.size(); index += WR_SEGMENT_SZ) {
      // write a chunk.
    BYTE buffer[sizeof(WORD) + sizeof(WORD) + WR_SEGMENT_SZ];
    memset(buffer, 0, sizeof(buffer));
    size_t amt2send = sizeof(buffer);

      // pack the write address
    BYTE writeAddressHi = (writeAddress >> 8);
    BYTE writeAddressLo = (0x00FF & writeAddress);

    buffer[0] = writeAddressHi;
    buffer[1]= writeAddressLo;

    buffer[2] = flashKeyHi;
    buffer[3] = flashKeyLo;

      // pack the segment
    for (int j = 0; j < WR_SEGMENT_SZ; ++j) {
      if ( (index + j) >= user_profile.size() ) {
        amt2send = 4 + j + 1;
        break;
      }
      buffer[ 4 + j] = user_profile[index+j];
    }

      // write the buffer
    status = HidSmbus_WriteRequest(device, USER_PROFILE_SLAVE_ADDRESS, buffer, SILABS_CONVERT_LARGER_TO_8BITS(amt2send));
    if (HID_SMBUS_SUCCESS != status) { throw CDllErr(); }

      // wait for write to complete.

    WaitForTransferStatusComplete(device);

      // increment the write address
    writeAddress += WR_SEGMENT_SZ;
  }

  SetNormalMode(device);

  if (HID_SMBUS_SUCCESS != status) {
    throw CUsageErr("\nERROR: Write error.");
  }

  return status;
}

HID_SMBUS_STATUS setConfig(DWORD DevIndex, WORD vid, WORD pid, std::istream & is)
{
  WORD configBodySize = 0;

  HID_SMBUS_STATUS status = HID_SMBUS_UNKNOWN_ERROR;
  std::vector<BYTE> config;
  BYTE flashKeyHi = (kFlashKeys >> 8);
  BYTE flashKeyLo = (0x00FF & kFlashKeys);
  BYTE eraseCmdHi = (kEraseCmd >> 8);
  BYTE eraseCmdLo = (0x00FF & kEraseCmd);

  std::string word;

  while (readWord(word, is)) {
    if (0 == strcmp("Config", word.c_str())) {
      readKeyword("{", is);
      break;
    }
  }

  readByteArrayParm( config, MAX_USHORT, is);
  readKeyword("}", is);

  HID_SMBUS_DEVICE device = g_DevHandles.get( DevIndex, vid, pid);

  // verify the checksum.
  const BYTE * pbconfig = &config[0];

  {
    BYTE bodySizeH = pbconfig[42];
    BYTE bodySizeL = pbconfig[43];
    configBodySize = (bodySizeH << 8) | bodySizeL;
  }

  WORD checksum_computed = ComputeChecksum_cp2615(pbconfig + (CONFIG_PREFIX_SIZE - 2), 2 + configBodySize);

  WORD checksum_orig = htons( *( WORD *)(pbconfig + CONFIG_PREFIX_SIZE - 4) );

  if (checksum_orig != checksum_computed) {
    throw CSyntErr("CHECKSUM_ERROR");
  }

  SetConfigMode(device);

  // Send the Erase command and wait for transfer to complete
  BYTE erase_cmd_buffer[4];
  erase_cmd_buffer[0] = eraseCmdHi;
  erase_cmd_buffer[1] = eraseCmdLo;
  erase_cmd_buffer[2] = flashKeyHi;
  erase_cmd_buffer[3] = flashKeyLo;
  status = HidSmbus_WriteRequest(device, SLAVE_ADDRESS, erase_cmd_buffer, 4);
  if (HID_SMBUS_SUCCESS != status) { throw CDllErr(); }
  WaitForTransferStatusComplete(device);

    // send the config
  WORD writeAddress = 0;

  for (unsigned int index = 0; index < config.size(); index += WR_SEGMENT_SZ) {
      // write a chunk.
    BYTE buffer[sizeof(WORD) + sizeof(WORD) + WR_SEGMENT_SZ];
    memset(buffer, 0, sizeof(buffer));
    size_t amt2send = sizeof(buffer);

      // pack the write address
    BYTE writeAddressHi = (writeAddress >> 8);
    BYTE writeAddressLo = (0x00FF & writeAddress);

    buffer[0] = writeAddressHi;
    buffer[1]= writeAddressLo;

    buffer[2] = flashKeyHi;
    buffer[3] = flashKeyLo;

      // pack the segment
    for (int j = 0; j < WR_SEGMENT_SZ; ++j) {
      if ( (index + j) >= config.size() ) {
        amt2send = 4 + j + 1;
        break;
      }
      buffer[ 4 + j] = config[index+j];
    }

      // write the buffer
    status = HidSmbus_WriteRequest(device, SLAVE_ADDRESS, buffer, SILABS_CONVERT_LARGER_TO_8BITS(amt2send));
    if (HID_SMBUS_SUCCESS != status) { throw CDllErr(); }

      // wait for write to complete.

    WaitForTransferStatusComplete(device);

      // increment the write address
    writeAddress += WR_SEGMENT_SZ;
  }

  SetNormalMode(device);

  if (HID_SMBUS_SUCCESS != status) {
    throw CUsageErr("\nERROR: Write error.");
  }

  return status;
}


//#pragma mark - Help

#if 0
void printProgDesc()
{
  printf(
         "Name\n"
         "    Standalone Manufacturing Tool. Version 1.0.\n"
         "Synopsis\n"
         "    smt option1 option1_argument option2 option2_argument ...\n"
         "Description\n"
         "    SMT is a standalone executable tool that provides command line\n"
         "    capability to flash Silicon Labs fixed function devices with\n"
         "    desired configuration values. The tool takes as input\n"
         "    a configuration file in text format that was created using the\n"
         "    Xpress Family configuration tools provided in Simplicity Studio.\n"
         "    All devices must be of the same device model (e.g. CP2615).\n"
         );
}

void printCmdLineHelp()
{
  printf(
         "Options\n"
         "--help\n"
         "    Output this page.\n"
         "--verbose\n"
         "    Shows the location of a syntax error in the configuration file.\n"
         "--device-count <decimal number>\n"
         "    Mandatory. Specifies how many devices are connected. Programming\n"
         "    process will not start if it finds a different number of devices\n"
         "    or fails to open them. Verification process will endlessly retry\n"
         "    until it can verify this number of devices.\n"
         "--set-config config_file_name\n"
         "    Programs each device using the configuration provided in the\n"
         "    configuration file."
         "--verify-config config_file_name\n"
         "    Verifies each device using the configuration provided in the\n"
         "    configuration file.\n"
         "--set-and-verify-config config_file_name\n"
         "    Programs and verifies each device using the configuration provided in\n"
         "    the configuration file.\n"
         "--set-user-profile user_profile_filename\n"
         "    Programs each device with the user-profile provided in the\n"
         "    configuration file.\n"
         "\nNormal usage example\n"
         "    The following command will program, verify and permanently lock the\n"
         "    customizable parameters of all 3 connected devices. (Serial numbers\n"
         "    will be automatically generated.)\n"
         "\n    smt --device-count 3 --set-and-verify-config my_config.txt --serial-nums GUID --lock\n"
         "\nExample for custom commands\n"
         "    If you need to insert your own custom steps between programming and\n"
         "    verification, use the following commands to separate steps. (Serial\n"
         "    numbers are provided by the user to write to the device and provided\n"
         "    again by the user for verification.)\n"
         "\n    smt --device-count 3 --set-config my_config.txt\n"
         "    smt --device-count 3 --verify-config my_config.txt\n"
         );
}

bool isSpecified( int argc, const char * argv[], const std::string &parmName)
{
  for( int i = 0; i < argc; i++) {
    if( std::string( argv[ i]) == parmName) {
      return true;
    }
  }
  return false;
}

bool isSpecified( int argc, const char * argv[], const std::string &parmName, std::string &paramValue)
{
  for( int i = 0; i < argc; i++)
  {
    if( std::string( argv[ i]) == parmName)
    {
      i++;
      if( i < argc)
      {
        paramValue = argv[ i];
        return true;
      }
      throw CUsageErr( std::string( "parameter is missing after ") + parmName + " command line option");
    }
  }
  return false;
}
#endif

WORD axtoi(const char * hexnum)
{
  WORD result = 0;
  
  
  for (unsigned int i = 0; i < strlen(hexnum); ++i) {
    result *= 16;
    
    char c = hexnum[i];
    
    if (c >= '0' && c <= '9') {
      result += c - '0';
      
    } else if ( c >= 'a' && c <= 'f') {
      
      result += (10 + (c - 'a'));
      
    } else if ( c >= 'A' && c <= 'F') {
      
      result += (10 + (c - 'A'));
      
    }
  }
  
  return result;
}

DWORD LibSpecificNumDevices( const CVidPid &, const CVidPid &)
{
#if 0
  DWORD DevCnt;
  AbortOnErr( CP210x_GetNumDevices( &DevCnt ), "CP210x_GetNumDevices");
  return DevCnt;
#endif
  return 1;
}

