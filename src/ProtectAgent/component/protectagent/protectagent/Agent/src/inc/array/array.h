#ifndef __AGENT_ARRAY_H__
#define __AGENT_ARRAY_H__

#ifdef LINUX
#include <sys/vfs.h>
#include <sys/sysinfo.h>
#include <sys/syslog.h>
#include <iconv.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <scsi/sg.h>
#include <scsi/scsi_ioctl.h>
#include <scsi/scsi.h>
#include <ctype.h>
#include <linux/raw.h>
#include <linux/major.h>
#include <fcntl.h>
#include <linux/fs.h>
#endif
#ifdef SOLARIS
#include <sys/ioctl.h>
#include <stropts.h>
#include <iconv.h>
#include <syslog.h>
#include <sys/scsi/scsi.h>
#include <sys/scsi/impl/uscsi.h>
#endif
#ifdef HP_UX_IA
#include <iconv.h>
#include <sys/pstat.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <stropts.h>
#include <sys/scsi.h>
#endif
#ifdef AIX
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/scsi.h>
#include <sys/scdisk.h>
#endif
#include "array/storage_defines.h"

class Array {
public:
    Array();
    ~Array();

    static mp_int32 GetLunInfo(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID);
    static mp_int32 GetArraySN(const mp_string& strDev, mp_string& strSN);
    static mp_int32 GetArrayVendorAndProduct(const mp_string& strDev, mp_string& strvendor, mp_string& strproduct);
    static mp_int32 GetDisk80Page(const mp_string& strDevice, mp_string& strSN);
    static mp_int32 GetDisk83Page(const mp_string& strDevice, mp_string& strLunWWN, mp_string& strLunID);
    static mp_int32 GetDisk00Page(const mp_string& strDevice, std::vector<mp_string>& vecResult);
    static mp_int32 GetDiskC8Page(const mp_string& strDevice, mp_string& strLunID);
    static mp_int32 GetDiskArrayInfo(const mp_string& strDevice, mp_string& strVendor, mp_string& strProduct);
    static mp_int32 GetHostLunIDList(const mp_string& strDevice, std::vector<mp_int32>& vecHostLunID);
    static mp_bool CheckHuaweiLUN(const mp_string& strVendor);
    static mp_int32 GetHostLunIDInfo(const mp_string& strDevice, std::vector<mp_int32>& vecHostLunID);
    static mp_int32 GetDiskWWNAndLUNID(mp_uchar aucBuffer[], mp_int32 buffLen, mp_string &strLunWWN,
        mp_string &strLunID);
    static mp_int32 GetFusionStorageWWNAndLUNID(mp_uchar aucBuffer[], mp_int32 buffLen, mp_string &strLunWWN,
        mp_string &strLunID);
    static mp_int32 GetDiskSN(const mp_uchar aucBuffer[], mp_int32 buffLen, mp_string& strSN);
    static mp_int32 GetDiskFusionStorageSN(const mp_uchar aucBuffer[], mp_int32 bufferLen, mp_string& strSN);

private:
#ifdef WIN32
    static mp_int32 OpenDev(const mp_string& strDev, HANDLE& handle);
    static mp_int32 GetDevHandle(const mp_string& pszDeviceName, FILE_HANDLE& pHandle);
    static mp_int32 GetDevSCSIAddress(FILE_HANDLE fHandle, win_dev_info_t& pstDevInfo);
    static mp_void SetScsiBufValues(
        scsi_pass_through_with_buff_t& stSCSIPass, mp_ulong& ulLength, mp_uchar ucCdb, mp_uchar ucCmd);
    static mp_int32 InitDisk83PageRes(const mp_string& strDevice, win_dev_info_t& pstDevInfo, FILE_HANDLE& fHandle);
    static mp_int32 GetDisk83PageWin(const mp_string& strDevice, mp_string& strLunWWN, mp_string& strLunID);
    static mp_int32 GetLunInfoWin(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID);
    static mp_int32 GetDiskC8PageWin(const mp_string& strDevice, mp_string& strLunID);
    static mp_int32 GetDiskC8PageWin_Ex(const FILE_HANDLE& fHandle,
        const win_dev_info_t& pstDevInfo, mp_string& strLunID);
    static mp_int32 GetDisk00PageWin(const mp_string& strDevice, std::vector<mp_string>& vecResult);
    static mp_int32 GetDisk00PageWin_Ex(FILE_HANDLE &fHandle,
        win_dev_info_t& pstDevInfo, std::vector<mp_string>& vecResult);
    static mp_int32 GetDisk80PageWin(const mp_string& strDevice, mp_string& strSN);
#else
    static mp_int32 OpenDev(const mp_string& strDev, mp_int32& iDevFd);
    static mp_int32 GetDiskPage(mp_int32 iFd, mp_uchar ucCmd, mp_uchar aucBuffer[], mp_int32 bufferLen);
    
#if defined LINUX
    static mp_int32 GetHostLunID(const mp_string &strDevice, mp_int32 &iFd, struct sg_io_hdr &io_hdr,
        const mp_uchar gaucResponseBuff[], mp_uint32 buffLen, mp_uint32 &mx_resp_len,
        std::vector<mp_int32> &vecHostLunID);
    static mp_int32 GetDiskPageLinux(mp_int32 iFd, mp_uchar ucCmd, mp_uchar aucBuffer[], mp_int32 bufferLen);
    static mp_int32 GetVendorAndProductLinux(mp_int32 iFd, mp_string& strVendor, mp_string& strProduct);
#elif defined AIX
    static mp_int32 GetDiskPageAIX(mp_int32 iFd, mp_uchar ucCmd, mp_uchar* aucBuffer, mp_int32 bufferLen);
    static mp_int32 GetVendorAndProductAIX(mp_int32 iFd, mp_string& strVendor, mp_string& strProduct);
#elif defined HP_UX_IA
    static mp_int32 GetDiskPageHP(mp_int32 iFd, mp_uchar ucCmd, mp_uchar aucBuffer[], mp_int32 bufferLen);
    static mp_int32 GetVendorAndProductHP(mp_int32 iFd, mp_string& strVendor, mp_string& strProduct);
#elif defined SOLARIS
    static mp_int32 GetDiskPageSolaris(mp_int32 iFd, mp_uchar ucCmd, mp_uchar aucBuffer[], mp_int32 bufferLen);
    static mp_int32 GetVendorAndProductSolaris(mp_int32 iFd, mp_string& strVendor, mp_string& strProduct);
#endif

    static mp_int32 GetVendorAndProduct(mp_int32 iFd, mp_string& strVendor, mp_string& strProduct);
    static mp_int32 GetDisk83PageNoWin(const mp_string& strDevice, mp_string& strLunWWN, mp_string& strLunID);
    static mp_int32 GetLunInfoNoWin(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID);
    static mp_int32 GetLunInfoNoWinAfter(mp_string& strDev, mp_string& strLunID);
    static mp_int32 GetDiskC8PageNoWin(const mp_string& strDevice, mp_string& strLunID);
    static mp_int32 GetDisk00PageNoWin(const mp_string& strDevice, std::vector<mp_string>& vecResult);
    static mp_int32 GetDisk80PageNoWin(const mp_string& strDevice, mp_string& strSN);
#endif

    static mp_int32 BinaryToAscii(
        mp_char pszHexBuffer[], mp_int32 iBufferLen, const mp_uchar pucBuffer[], mp_int32 iStartBuf, mp_int32 iLength);
    static mp_int32 ConvertLUNIDtoAscii(
        mp_char puszAsciiLunID[], mp_int32 iBufferLen, mp_uchar puszLunLunID[], mp_int32 iLen);
    static mp_int32 HextoDec(mp_char pDecStr[], mp_char pHexStr[], mp_int32 iMaxLen);
    static mp_int32 HexEncode(mp_char cHex, mp_int32 iStep, mp_int32& iDecNum);
    static mp_int32 CalStep(mp_int32 iStep);
    static mp_bool IsSupportXXPage(const mp_string& page, std::vector<mp_string>& vecResult);

    static const mp_uchar ARRAY_INDEX_2   = 2;
    static const mp_uchar ARRAY_INDEX_3   = 3;
    static const mp_uchar ARRAY_INDEX_4   = 4;
    static const mp_uchar ARRAY_INDEX_5   = 5;
    static const mp_uchar ARRAY_INDEX_6   = 6;
    static const mp_uchar ARRAY_INDEX_7   = 7;
    static const mp_uchar ARRAY_INDEX_8   = 8;
    static const mp_uchar ARRAY_INDEX_9   = 9;
    static const mp_uchar ARRAY_INDEX_192 = 192;
    static const mp_uchar ARRAY_INDEX_193 = 193;
    static const mp_uchar ARRAY_INDEX_194 = 194;
    static const mp_uchar ARRAY_INDEX_195 = 195;

    static const mp_uchar ARRAY_NUM_2     = 2;
    static const mp_uchar ARRAY_NUM_6     = 6;
    static const mp_uchar ARRAY_NUM_8     = 8;
    static const mp_uchar ARRAY_NUM_9     = 9;
    static const mp_uchar ARRAY_NUM_12    = 12;
    static const mp_uchar ARRAY_NUM_16    = 16;
    static const mp_uchar ARRAY_NUM_20    = 20;
    static const mp_uchar ARRAY_NUM_24    = 24;
    static const mp_uchar ARRAY_NUM_32    = 32;
    static const mp_uchar ARRAY_NUM_47    = 47;
    static const mp_uchar ARRAY_NUM_48    = 48;
    static const mp_uchar ARRAY_NUM_55    = 55;
    static const mp_uchar ARRAY_NUM_60    = 60;
    static const mp_int32 ARRAY_NUM_255   = 255;
    static const mp_int32 ARRAY_NUM_500   = 500;
    static const mp_int32 ARRAY_BYTES_256 = 256;
};

#endif
