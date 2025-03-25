/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file disk.h
 * @brief  The implemention about disk
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef __AGENT_DISK_H__
#define __AGENT_DISK_H__

#include <vector>
#include <map>

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
#ifdef SOLARIS
#include <sys/ioctl.h>
#include <stropts.h>
#include <iconv.h>
#include <syslog.h>
#include <sys/scsi/scsi.h>
#include <sys/scsi/impl/uscsi.h>
#endif
#ifdef AIX
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/scsi.h>
#include <sys/scdisk.h>
#endif
#include "array/diskOM.h"

class Disk : public DiskOM {
public:
    Disk();
    ~Disk();

    static mp_int32 GetDevNameByWWN(mp_string& strDevName, const mp_string& strWWN);
    static mp_int32 OpenDev(mp_string& strDevice, mp_int32& iDevFd);
    static mp_int32 GetAllDiskName(std::vector<mp_string>& vecDiskName);
    static mp_int32 GetDiskCapacity(mp_string& strDevice, mp_string& strBuf);
    static mp_bool IsSdisk(mp_string& strDevice);
    static mp_bool IsHdisk(mp_string& strDevice);
    static mp_bool IsDeviceExist(mp_string& strDiskName);
    static mp_bool IsDskdisk(mp_string& strName);
    static mp_int32 GetHPRawDiskName(mp_string strDiskName, mp_string& strRawDiskName);
    static mp_int32 GetSolarisRawDiskName(mp_string strDiskName, mp_string& strRawDiskName);
    static mp_int32 GetPersistentDSFByLegacyDSF(mp_string& rstrLegacyDisk, mp_string& rstrPersistentDisk);
    static mp_int32 ClearInvalidDisk();
    static mp_int32 GetSecPvName(const mp_string& strPriPvName, mp_string& strLegacyDisk, mp_string& strSecPvName);
#ifdef WIN32
    static mp_int32 GetVolPaths(std::map<mp_string, mp_string>& mapPath);
    static mp_bool IsDriveExist(mp_string& strDriveLetter);
    static mp_int32 GetVolumeDiskInfo(mp_char pcVolumeName[], mp_int32 bufferLen, sub_area_Info_t& pstrPartInfo);
    static mp_int32 GetNextVolumeInformation(FILE_HANDLE FindHandle, mp_char pcVolumeName[], mp_int32 iSize);
    static mp_int32 GetDiskNum(mp_string& strDeviceName, mp_int32& DiskNum);
    static mp_int32 GetSubareaInfoList(std::vector<sub_area_Info_t>& rvecSubareaInfo);
    static mp_int32 AssignVolumeInfo(const FILE_HANDLE &FindHandle, mp_char acVolumeName[], mp_int32 buffLen,
        std::map<mp_string, mp_string> &mapVol, sub_area_Info_t &strPartInfo);
    static mp_int32 GetDiskInfoList(std::vector<disk_info>& vecDiskInfoWin);
    mp_bool InitSymboLinkRes();
    mp_void FreeSymboLinkRes();
    mp_bool QuerySymboLinkInfo(const mp_string& strSymboLink, mp_string& strTargetDevice,
    mp_bool isVolumn = MP_FALSE);
#endif

private:
#ifdef WIN32
    HMODULE m_hNtdll;
    RTLINITUNICODESTRING RtlInitUnicodeString;
    ZWOPENDIRECTORYOBJECT ZwOpenDirectoryObject;
    ZWOPENSYMBOLICKLINKOBJECT ZwOpenSymbolicLinkObject;
    ZWQUERYSYMBOLICKLINKOBJECT ZwQuerySymbolicLinkObject;
    ZWCLOSE ZwClose;
#endif

private:
    static mp_int32 GetDiskStatus(mp_string& strDiskName, mp_string& strStatus);
    static mp_bool IsCmdDevice(mp_string& strDiskName);
    static mp_int32 ScsiNormalizeSense(mp_string& strBuf, ISSP_SCSI_SENSE_HDR_S& stSSHdr);
    static mp_int32 ScsiNormalizeSenseAfter(mp_string& strBuf, ISSP_SCSI_SENSE_HDR_S& stSSHdr,
    mp_int32 iSBLen);
    static mp_int32 ClearInvalidLUNPath(std::vector<mp_string>& vecLUNPaths);
    static mp_int32 ClearInvalidLegacyDSFs(std::vector<mp_string>& vecDiskNumbers);
    static mp_int32 ClearInvalidPersistentDSFs(std::vector<mp_string>& vecDiskNumbers);
    static mp_int32 DeletePersistentDSF(mp_string& strDeviceName);
    static mp_int32 GetPersistentDSFInfo(mp_string& rstrDiskName, mp_string& strPDHWPath,
    mp_string& strPDHWType);
    static mp_int32 ExecuteDiskCmdEx(mp_string strCmd, std::vector<mp_string>& vecDiskName,
    mp_string strFileName);
#ifdef WIN32
    static mp_int32 GetDiskPath(HDEVINFO& hIntDevInfo, mp_int64 iIndex, mp_char pszPath[], mp_int32 pszPathLen);
    static mp_int32 GetDiskPath_Ex(HDEVINFO& hIntDevInfo, SP_DEVICE_INTERFACE_DATA& stInterFaceData,
        mp_char pszPath[], mp_int32 pszPathLen);
    static mp_int32 GetAllDiskWin(std::vector<mp_string>& vecDiskName);
    static mp_void GetDiskInfo(const std::vector<mp_string> &vecDiskName, disk_info &stDiskInfo, mp_string &strVendor,
        mp_string &strProduct);
    static mp_void GetVolPathByVolName(std::map<mp_string, mp_string>& mapPath);
    mp_bool LoadNtdllModule(void);
    mp_void FreeNtdllModule(void);
    mp_ulong QuerySymbolicLink(IN PUNICODE_STRING SymbolicLinkName, OUT PUNICODE_STRING LinkTarget,
    mp_bool isVolumn);
#endif
#ifdef LINUX
    static mp_void SetSGHDR(sg_io_hdr& stScsiCmd, mp_bool bIsBigDisk);
    static mp_int32 HandleCapacityResult(sg_io_hdr& stScsiCmd, mp_int32& iRetryNum, mp_bool& bIsBigDisk,
        mp_bool& bIsBreakOut, mp_int32 iRet, const mp_string& strDevice, mp_bool bRet);
#endif
    static mp_bool CheckHuaweiDisk(mp_string& newDiskName);

    static const mp_uchar DISK_INDEX_2 = 2;
    static const mp_uchar DISK_INDEX_3 = 3;
    static const mp_uchar DISK_INDEX_4 = 4;
    static const mp_uchar DISK_INDEX_7 = 7;
    static const mp_uchar DISK_INDEX_8 = 8;
    static const mp_uchar DISK_INDEX_12 = 12;
    static const mp_uchar DISK_INDEX_13 = 13;

    static const mp_uchar DISK_NUM_12 = 12;
    static const mp_uchar DISK_NUM_13 = 13;
    static const mp_uchar DISK_NUM_100 = 100;
    static const mp_int32 ARRAY_BYTES_256 = 256;
};

#endif

