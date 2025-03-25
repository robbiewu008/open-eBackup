/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file disk.cpp
 * @brief  Contains function declarations get disk info
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "array/disk.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/Defines.h"
#include "common/MpString.h"
#include "securecom/UniqueId.h"
#include "common/Path.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "array/array.h"
using namespace std;

namespace {
    using Defer = std::shared_ptr<void>;
};

/* ------------------------------------------------------------
Description  :获取主机上所有磁盘名称
Output       : vecDiskName --磁盘名称
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */

#ifdef WIN32
mp_int32 Disk::GetAllDiskName(vector<mp_string>& vecDiskName)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_INFO, "Begin get all disk names.");
    return GetAllDiskWin(vecDiskName);  // 降低圈复杂度，对函数进行拆分

    COMMLOG(OS_LOG_INFO, "End get all disk names.");
    return MP_SUCCESS;
}
#endif
#ifdef LINUX
mp_int32 Disk::GetAllDiskName(vector<mp_string>& vecDiskName)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_INFO, "Begin get all disk names.");
    mp_string strCmd = "cat /proc/partitions  | grep -e 'sd[a-z]' -e 'hd[a-z]' -e 'xvd[a-z]' -e 'vd[a-z]' |"
                       " awk '{print $4}' | grep -v -e '[1-9]' ";

    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecDiskName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Exec system cmd failed, cmd is %s, iRet is %d.", strCmd.c_str(), iRet);
        return iRet;
    }

    // 查找CCISS卡设备
    // 移除命令设备和移除测试失败的设备
    for (vector<mp_string>::iterator it = vecDiskName.begin(); it != vecDiskName.end();) {
        if (IsCmdDevice(*it)) {
            COMMLOG(OS_LOG_DEBUG, "\"%s\" is a cmd device, skip it.", it->c_str());
            it = vecDiskName.erase(it);
            continue;
        }

        if (!IsDeviceExist(*it)) {
            COMMLOG(OS_LOG_DEBUG, "Device \"%s\" dosen't exit, skip it.", it->c_str());
            // 2019/06/19 z00455045, some virtio disk (such as: /dev/vda) not support lookup capacity info by scsi,
            // so IsDeviceExist() will return MP_FALSE, now temporarily shield the below two sentences.
            // the code may be useful : // it = vecDiskName.erase(it); continue;
        }

        ++it;
    }

    COMMLOG(OS_LOG_INFO, "End get all disk names.");
    return MP_SUCCESS;
}
#endif
#ifdef AIX
mp_int32 Disk::GetAllDiskName(vector<mp_string>& vecDiskName)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_INFO, "Begin get all disk names.");
    mp_string strCmd = "lsdev -Cc disk  | awk '{print $1}'";

    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecDiskName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Exec system cmd failed, cmd is %s, iRet is %d.", strCmd.c_str(), iRet);
        return iRet;
    }

    for (vector<mp_string>::iterator it = vecDiskName.begin(); it != vecDiskName.end();) {
        mp_string strStatus;
        iRet = GetDiskStatus(*it, strStatus);
        if ((iRet == MP_SUCCESS) && (strStatus != "Available")) {  // 去除没有激活的
            it = vecDiskName.erase(it);
        } else {
            it++;
        }
    }

    COMMLOG(OS_LOG_INFO, "End get all disk names.");
    return MP_SUCCESS;
}
#endif
#ifdef HP_UX_IA
mp_int32 Disk::GetAllDiskName(vector<mp_string>& vecDiskName)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_INFO, "Begin get all disk names.");
    mp_string strUniqueID = CUniqueID::GetInstance().GetString();
    mp_string strTempFile = "App_" + strUniqueID + ".temp";
    mp_string strExectCmd = "/usr/sbin/ioscan -funC disk 2>/dev/null | grep /dev/dsk | awk '{print $1}'>" + strTempFile;
    mp_string strDiskName;
    mp_string strFullDiskName;
    vector<mp_string> vecSrcDiskName;

    mp_int32 iRet = ExecuteDiskCmdEx(strExectCmd, vecDiskName, strTempFile);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Execute disk cmd failed, cmd %s.", strExectCmd.c_str());
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "End get all disk names.");
    return MP_SUCCESS;
}
#endif
#ifdef SOLARIS
mp_int32 Disk::GetAllDiskName(vector<mp_string>& vecDiskName)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_INFO, "Begin get all disk names.");
    mp_string strCmd = "iostat -En | nawk '{if(NR%5==1) {print $1}}'";  // solaris获取磁盘信息

    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecDiskName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Exec system cmd failed, cmd is %s, iRet is %d.", strCmd.c_str(), iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "End get all disk names.");
    return MP_SUCCESS;
}
#endif

// 如下sdd1是扩展分区、sdl是阵列命令设备
// major minor  #blocks  name
// 8    48   10485760 sdd
// 8    49          1 sdd1
// 8    53      98272 sdd5
// 8    54      98288 sdd6
// 8    55      98288 sdd7
// 8    56      98288 sdd8
// 8    57     977904 sdd9
// 8   176          1 sdl
/* ------------------------------------------------------------
Description  : 判断是否是cmd设备
Input        : strDiskName---磁盘名称
Return       :  MP_TRUE---是cmd设备
               MP_FALSE---非cmd设备
------------------------------------------------------------- */
mp_bool Disk::IsCmdDevice(mp_string& strDiskName)
{
    LOGGUARD("");
    vector<mp_string> vecRlt;
    mp_int32 iBlocks;
    CHECK_FAIL_EX(CheckCmdDelimiter(strDiskName));
    mp_string strCmd = "cat /proc/partitions | grep -e '" + strDiskName + "'  | awk '{print $3}'";
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Exec system cmd failed, cmd is %s, iRet is %d.", strCmd.c_str(), iRet);
        return MP_FALSE;
    }

    if (vecRlt.size() <= 1) {
        COMMLOG(OS_LOG_ERROR, "vecRlt.size() is smaller than 1, the value is %d.", vecRlt.size());
        return MP_FALSE;
    }

    iBlocks = atoi(vecRlt[0].c_str());  // 第一行显示的是设备，其它是分区
    if (iBlocks == CMD_DEV_BLOCKS) {
        COMMLOG(OS_LOG_INFO, "%s is a cmd device.", strDiskName.c_str());
        return MP_TRUE;
    }

    return MP_FALSE;
}

/* ------------------------------------------------------------
Description  : 判断设备是否是HP下disk
Input        : strName -- disk名称
Return       : MP_TRUE
                   MP_FALSE
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_bool Disk::IsDskdisk(mp_string& strName)
{
    mp_bool bIsDskdisk = MP_TRUE;
#ifdef HP_UX_IA
    mp_string::size_type iIndex;

    iIndex = 0;
    mp_string::size_type iIndexDsk = strName.find("/dev/dsk");
    mp_string::size_type iIndexDisk = strName.find("/dev/disk");

    iIndex = (iIndexDsk == mp_string::npos) ? iIndexDisk : iIndexDsk;
    if (iIndex != mp_string::npos) {
        bIsDskdisk = MP_TRUE;
    } else {
        bIsDskdisk = MP_FALSE;
    }
#endif
    (mp_void) strName;
    return bIsDskdisk;
}

/* ------------------------------------------------------------
Function Name: GetHPRawDiskName
Description  : 由磁盘设备名称得到对应的裸盘设备名-hp
               Data Accessed: None.
               Data Updated : None.
Input        : pszDiskName-磁盘设备名
Output       : pszRawDiskName-对应的裸盘设备名
Return       : 成功或失败.
Created By   : lufei 00253739
------------------------------------------------------------- */
mp_int32 Disk::GetHPRawDiskName(mp_string strDiskName, mp_string& strRawDiskName)
{
#ifdef HP_UX_IA
    mp_int32 iPos;
    mp_int32 iPosDsk = strDiskName.find("dsk", 0);
    mp_int32 iPosDisk = strDiskName.find("disk", 0);
    iPos = ((iPosDsk == mp_string::npos) ? iPosDisk : iPosDsk);
    if (mp_string::npos == iPos) {
        COMMLOG(OS_LOG_ERROR, "Disk name is invalid, disk name %s.", strDiskName.c_str());

        return MP_FAILED;
    }

    mp_string strHead = strDiskName.substr(0, iPos);
    mp_string strTail = strDiskName.substr(iPos, strDiskName.length() - iPos);
    strRawDiskName = strHead + "r" + strTail;
#endif
    (mp_void) strDiskName;
    (mp_void) strRawDiskName;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: GetSolarisRawDiskName
Description  : 由磁盘设备名称得到对应的裸盘设备名-solaris
               Data Accessed: None.
               Data Updated : None.
Input        : pszDiskName-磁盘设备名
Output       : pszRawDiskName-对应的裸盘设备名
Return       : 成功或失败.
Created By   : lufei 00253739
------------------------------------------------------------- */
mp_int32 Disk::GetSolarisRawDiskName(mp_string strDiskName, mp_string& strRawDiskName)
{
#ifdef SOLARIS
    mp_int32 iPos = strDiskName.find("dsk", 0);
    if (mp_string::npos == iPos) {
        COMMLOG(OS_LOG_ERROR, "Disk name is invalid, disk name %s.", strDiskName.c_str());
        return MP_FAILED;
    }

    mp_string strHead = strDiskName.substr(0, iPos);
    mp_string strTail = strDiskName.substr(iPos, strDiskName.length() - iPos);
    strRawDiskName = strHead + "r" + strTail;
#endif
    (mp_void) strDiskName;
    (mp_void) strRawDiskName;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: GetSecPvName
Description  : 根据生产端PV名称获取灾备端相应PV名称
                     如生产端为/dev/disk/diskxx,则灾备端为/dev/disk/diskxx
                     生产端为/dev/dsk/xx,则灾备端为/dev/dsk/xx-hp
               Data Accessed: None.
               Data Updated : None.
Input        : strPriPvName-生产端PV名称
                  strLegacyDisk-灾备端LegacyDisk名称
Output       : strSecPvName-灾备端PV名称
Return       : 成功或失败.
Created By   : lufei 00253739
------------------------------------------------------------- */
mp_int32 Disk::GetSecPvName(const mp_string& strPriPvName, mp_string& strLegacyDisk, mp_string& strSecPvName)
{
#ifdef HP_UX_IA
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strPersistentDisk;
    mp_string::size_type iIndexDsk;
    mp_string::size_type iIndexDisk;

    iIndexDsk = strPriPvName.find("/dev/dsk");
    if (iIndexDsk != mp_string::npos) {
        strSecPvName = strLegacyDisk;
    } else {
        iIndexDsk = strPriPvName.find("/dev/disk");
        if (iIndexDsk != mp_string::npos) {
            iRet = Disk::GetPersistentDSFByLegacyDSF(strLegacyDisk, strPersistentDisk);
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "Get persistent DSF failed, device name %s", strLegacyDisk.c_str());
                return iRet;
            }
            strSecPvName = strPersistentDisk;
        } else {
            COMMLOG(OS_LOG_ERROR, "The pripv is not dsk or disk.");
            return MP_FAILED;
        }
    }
#endif
    (mp_void) strPriPvName;
    (mp_void) strLegacyDisk;
    (mp_void) strSecPvName;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 清理无用设备-HP-UX
Return       : MP_SUCCESS -- 成功
               MP_FAILED -- 失败
Create By    : lufei 00253739
Modification : 当前只适用于HP-UX 11i V3
               HP-UX 11i V3版本存在legacy DSFs and persistent DSFs两种设备
               之前LUN映射后WWN和LUNPATH建立了通道，一个LUN 通道由设备SN+host LUN ID决定
               如果在一个LUNPATH上映射不同WWN的LUN，此时是无法扫描到LUN的
               在进行设置存储前需要进行清理无效的设备
1.ioscan -kfNC lunpath 查询出无用的lunpath:解除WWN映射并去掉
2.ioscan -kfNC disk 查询无用的disk
3.根据2步骤中的disk，通过ioscan -m dsf查询无用的dsk删除
4.删除步骤2中的disk
------------------------------------------------------------- */
mp_int32 Disk::ClearInvalidDisk()
{
    mp_int32 ret = MP_FAILED;
    ERRLOG("Function is forbidden");
    return ret;
}

// Description  : 清理无用LUNPATH-HP-UX
mp_int32 Disk::ClearInvalidLUNPath(vector<mp_string>& vecLUNPaths)
{
    mp_int32 ret = MP_FAILED;
    ERRLOG("Function is forbidden");
    return ret;
}

/* ------------------------------------------------------------
Description  : 清理无用legacy DSFs-HP-UX
Input        : vecDiskNumbers -- 对应无用的disk number
Return       : MP_SUCCESS -- 成功
                  MP_FAILED -- 失败
Create By    : lufei 00253739
Modification : 删除下面类似的legacy DSFs
               ioscan -m dsf /dev/rdisk/disk414
               Persistent DSF           Legacy DSF(s)
               ========================================
               /dev/rdisk/disk414       /dev/rdsk/c85t0d4
               /dev/rdsk/c85t1d4
------------------------------------------------------------- */
mp_int32 Disk::ClearInvalidLegacyDSFs(vector<mp_string>& vecDiskNumbers)
{
    mp_int32 ret = MP_FAILED;
    ERRLOG("Function is forbidden");
    return ret;
}

/* ------------------------------------------------------------
Description  : 清理无用persistent DSFs-HP-UX
Input        : vecDiskNumbers -- 对应无用的disk number
Return       : MP_SUCCESS -- 成功     MP_FAILED -- 失败
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Disk::ClearInvalidPersistentDSFs(vector<mp_string>& vecDiskNumbers)
{
#ifdef HP_UX_IA
    mp_int32 iRet = MP_SUCCESS;
    mp_string strDeviceName;

    COMMLOG(OS_LOG_DEBUG, "Begin to clear invalid persistent DSFs.");

    for (vector<mp_string>::iterator iter = vecDiskNumbers.begin(); iter != vecDiskNumbers.end(); ++iter) {
        strDeviceName = "/dev/disk/disk" + *iter;  // 存在不是当前使用的磁盘，如果失败不用直接返回
        (mp_void) DeletePersistentDSF(strDeviceName);
    }

    COMMLOG(OS_LOG_DEBUG, "Clear invalid persistent DSFs succ.");
#endif
    (mp_void) vecDiskNumbers;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 获取disk设备的HW Path和HW Type
Input        : rstrDiskNumber -- persistent DSF instance number
Output       : strPDHWPath/strPDHWType对应设备HWPATH和HWType
Return       : MP_SUCCESS -- 操作成功      非ISSP_RTN_OK -- 操作失败
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Disk::GetPersistentDSFInfo(mp_string& rstrDiskName, mp_string& strPDHWPath, mp_string& strPDHWType)
{
    mp_int32 ret = MP_FAILED;
    ERRLOG("Function is forbidden");
    return ret;
}

/* ------------------------------------------------------------
Description  : 获取disk设备的HW Path和HW Type
Input        : rstrLegancyDisk -- Legacy DSF name
Output       : rstrPersistentDisk对应legacy DSF的persistent DSF
Return       : MP_SUCCESS -- 操作成功      非ISSP_RTN_OK -- 操作失败
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Disk::GetPersistentDSFByLegacyDSF(mp_string& rstrLegacyDisk, mp_string& rstrPersistentDisk)
{
    mp_int32 ret = MP_FAILED;
    ERRLOG("Function is forbidden");
    return ret;
}

/* ------------------------------------------------------------
Description  : 删除Persistent DSF
Input        : strDeviceName -- 需要Persistent DSF的设备名称
Return       : ISSP_RTN_OK -- 操作成功
               非ISSP_RTN_OK -- 操作失败
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Disk::DeletePersistentDSF(mp_string& strDeviceName)
{
    mp_int32 ret = MP_FAILED;
    ERRLOG("Function is forbidden");
    return ret;
}

/* ------------------------------------------------------------
Function Name: ExecuteDiskCmdEx
Description  : 执行命令-hp
Input        : pszCmd-命令;strFileName-临时文件名
Output       : listValue-值链表
Return       : 成功或失败.
Created By   : lufei 00253739
------------------------------------------------------------- */
mp_int32 Disk::ExecuteDiskCmdEx(mp_string strCmd, vector<mp_string>& vecDiskName, mp_string strFileName)
{
#ifdef HP_UX_IA
    FILE* pFile = NULL;
    mp_char acBuf[DISK_CMD_DATA_LEN] = {0};
    mp_string strBaseFileName = BaseFileName(strFileName);

    COMMLOG(OS_LOG_DEBUG, "Exec cmd, cmd %s.", strCmd.c_str());
    if (system(strCmd.c_str()) != 0) {
        COMMLOG(OS_LOG_ERROR, "Exec cmd failed, cmd %s.", strCmd.c_str());
        (void)unlink(strFileName.c_str());
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Open file %s.", strBaseFileName.c_str());
    pFile = fopen(strFileName.c_str(), "r");
    if (pFile == NULL) {
        COMMLOG(OS_LOG_ERROR, "Open file failed, file %s.", strBaseFileName.c_str());
        (void)unlink(strFileName.c_str());
        return MP_FAILED;
    }

    for (mp_uint32 uiIndex = 0; uiIndex < FILE_ROW_COUNT; ++uiIndex) {
        CHECK_NOT_OK(memset_s(acBuf, DISK_CMD_DATA_LEN, 0, DISK_CMD_DATA_LEN));
        if (fgets(acBuf, sizeof(acBuf), pFile) == NULL) {
            break;
        }

        mp_string strAcBuf = acBuf;
        (void)CMpString::TotallyTrimRight(strAcBuf);  // 去掉首尾无用的字符
        vecDiskName.push_back(strAcBuf);
    }

    (void)fclose(pFile);
    (void)unlink(strFileName.c_str());
#endif
    (mp_void) strCmd;
    (mp_void) vecDiskName;
    (mp_void) strFileName;
    return MP_SUCCESS;
}

#ifdef WIN32

// Description  : 获取磁盘路径
mp_int32 Disk::GetDiskPath(HDEVINFO& hIntDevInfo, mp_int64 iIndex, mp_char pszPath[], mp_int32 pszPathLen)
{
    if ((iIndex < 0) || (pszPath == NULL)) {
        COMMLOG(OS_LOG_ERROR, "Index less than zero or pszPath is null pointer.");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "Begin get disk path.");
    SP_DEVICE_INTERFACE_DATA stInterFaceData;
    mp_int32 iRet = memset_s(&stInterFaceData, sizeof(SP_DEVICE_INTERFACE_DATA), 0, sizeof(SP_DEVICE_INTERFACE_DATA));
    if (iRet != EOK) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iRet);
        return MP_FAILED;
    }
    stInterFaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
    if (!SetupDiEnumDeviceInterfaces(hIntDevInfo, 0, &DiskClassGuid, (mp_ulong)iIndex, &stInterFaceData)) {
        mp_ulong ulErrorCode = GetLastError();
        if (ulErrorCode == ERROR_NO_MORE_ITEMS) {
            COMMLOG(OS_LOG_WARN, "SetupDiEnumDeviceInterfaces error, errorcode %d", DISK_NORECORD_ERROR);
            return DISK_NORECORD_ERROR;
        }

        COMMLOG(OS_LOG_ERROR, "SetupDiEnumDeviceInterfaces error, errorcode %d", ulErrorCode);
        return MP_FAILED;
    }

    return GetDiskPath_Ex(hIntDevInfo, stInterFaceData, pszPath, pszPathLen);
}

mp_int32 Disk::GetDiskPath_Ex(HDEVINFO& hIntDevInfo, SP_DEVICE_INTERFACE_DATA &stInterFaceData,
    mp_char pszPath[], mp_int32 pszPathLen)
{
    mp_ulong ulReqSize = 0;
    mp_int32 iStatus = SetupDiGetDeviceInterfaceDetail(hIntDevInfo, &stInterFaceData, NULL, 0, &ulReqSize, NULL);
    if (!iStatus || ulReqSize <= 0) {
        mp_ulong ulErrorCode = GetLastError();
        if (ulErrorCode != ERROR_INSUFFICIENT_BUFFER) {
            COMMLOG(OS_LOG_ERROR, "SetupDiGetDeviceInterfaceDetail error(%d)", ulErrorCode);
            return MP_FAILED;
        }
    }

    mp_ulong ulInterfaceDetailDataSize = ulReqSize;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pstInterfaceDetailData =
        (PSP_DEVICE_INTERFACE_DETAIL_DATA)calloc(ulInterfaceDetailDataSize, sizeof(PSP_DEVICE_INTERFACE_DETAIL_DATA));
    if (pstInterfaceDetailData == NULL) {
        COMMLOG(OS_LOG_ERROR, "Call calloc failed.");
        return MP_FAILED;
    }

    pstInterfaceDetailData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

    iStatus = SetupDiGetDeviceInterfaceDetail(hIntDevInfo, &stInterFaceData, pstInterfaceDetailData,
        ulInterfaceDetailDataSize, &ulReqSize, NULL);
    if (!iStatus) {
        COMMLOG(OS_LOG_ERROR, "SetupDiGetDeviceInterfaceDetail error");
        free(pstInterfaceDetailData);
        return MP_FAILED;
    }
    mp_int32 iRet = strncpy_s(pszPath, pszPathLen, pstInterfaceDetailData->DevicePath, DISK_PATH_MAX + 1);
    if (iRet != EOK) {
        free(pstInterfaceDetailData);
        COMMLOG(OS_LOG_ERROR, "Call strncpy_s failed, ret %d.", iRet);
        return MP_FAILED;
    }
    free(pstInterfaceDetailData);

    COMMLOG(OS_LOG_INFO, "End get disk path.");
    return MP_SUCCESS;
}

// 获取所有驱动器号和卷路径保存在map中
mp_int32 Disk::GetVolPaths(map<mp_string, mp_string>& mapPath)
{
    LOGGUARD("");
    mp_ulong ulDrive;
    mp_int32 iErr = 0;
    mp_char cDriveNum = 'A';  // 盘符从A开始
    mp_char acVolName[DISK_PATH_MAX] = {0};
    mp_char szErr[ARRAY_BYTES_256] = {0};
    mp_int32 iRet = 0;
    mp_string strDriverLetter;

    COMMLOG(OS_LOG_INFO, "Begin get volume paths.");

    ulDrive = GetLogicalDrives();
    if (ulDrive == 0) {
        iErr = GetOSError();
        COMMLOG(
            OS_LOG_ERROR, "Get logical drives failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    for (cDriveNum = 'A'; cDriveNum <= 'Z'; ++cDriveNum) {
        if (ulDrive & 0x01) {
            strDriverLetter = cDriveNum;
            strDriverLetter += ":\\";

            COMMLOG(OS_LOG_DEBUG, "Get volume path for mount point %s.", strDriverLetter.c_str());
            iRet = memset_s(acVolName, sizeof(acVolName), 0, sizeof(acVolName));
            if (iRet != EOK) {
                COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iRet);
                return iRet;
            }

            iRet = GetVolumeNameForVolumeMountPoint(strDriverLetter.c_str(), acVolName, sizeof(acVolName));
            if (iRet == 0) {
                iErr = GetOSError();
                COMMLOG(OS_LOG_ERROR,
                    "Get volume path for mount point failed, mount point %s, errno[%d] { :%s.",
                    strDriverLetter.c_str(),
                    iErr,
                    GetOSStrErr(iErr, szErr, sizeof(szErr)));
                return MP_FAILED;
            }

            COMMLOG(OS_LOG_DEBUG,
                "Add volume path and mount point info, volume path %s, mount point %s.",
                acVolName,
                strDriverLetter.c_str());
            (mp_void) mapPath.insert(make_pair(acVolName, strDriverLetter));
        }

        ulDrive = ulDrive >> 1;
    }

    COMMLOG(OS_LOG_INFO, "Get volume paths succ.");
    return MP_SUCCESS;
}

// Description  : 判断驱动是否存在
mp_bool Disk::IsDriveExist(mp_string& strDrive)
{
    mp_ulong ulDrive;
    mp_int32 iErr = 0;
    mp_char cDriveNum = 'A';  // 盘符从A开始
    mp_char szErr[ARRAY_BYTES_256] = {0};
    mp_string strDriverLetter;

    ulDrive = GetLogicalDrives();
    if (ulDrive == 0) {
        iErr = GetOSError();
        COMMLOG(
            OS_LOG_ERROR, "Get logical drives failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FALSE;
    }

    for (cDriveNum = 'A'; cDriveNum <= 'Z'; ++cDriveNum) {
        if (ulDrive & 0x01) {
            strDriverLetter = cDriveNum;
            strDriverLetter += ":\\";
            COMMLOG(OS_LOG_DEBUG,
                "Get drive letter %s, expected drive letter %s.",
                strDriverLetter.c_str(),
                strDrive.c_str());
            if (strDrive == strDriverLetter) {
                COMMLOG(OS_LOG_DEBUG, "Drive %s exist.", strDrive.c_str());
                return MP_TRUE;
            }
        }

        ulDrive = ulDrive >> 1;
    }

    return MP_FALSE;
}

/* ------------------------------------------------------------
Description  : 获取next卷信息
Input        : pcVolumeName---PC卷名称
Return       :  MP_SUCCESS---获取成功
               MP_FAILED---获取失败
------------------------------------------------------------- */
mp_int32 Disk::GetNextVolumeInformation(FILE_HANDLE FindHandle, mp_char pcVolumeName[], mp_int32 iSize)
{
    LOGGUARD("");
    mp_int32 iret = MP_SUCCESS;
    if (FindHandle == NULL) {
        return MP_FAILED;
    }

    mp_ulong ulError = 0;
    mp_int32 iSuccess = FindNextVolume(&FindHandle, pcVolumeName, iSize);
    if (!iSuccess) {
        ulError = GetLastError();
        if (ulError != ERROR_NO_MORE_FILES) {
            COMMLOG(OS_LOG_ERROR,
                "FindNextVolume failed with error code(%lu) when get next volume info on windows",
                ulError);
            return MP_FAILED;
        }
        return MP_FAILED;
    }

    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  : 获取磁盘号
Input        : strDeviceName---磁盘名称
Output       :  iDiskNum---磁盘号
Return       :  MP_SUCCESS---获取成功
               MP_FAILED---获取失败
------------------------------------------------------------- */
mp_int32 Disk::GetDiskNum(mp_string& strDeviceName, mp_int32& iDiskNum)
{
    mp_int32 iRet;
    FILE_HANDLE fHandle;
    mp_ulong ulReturn = 0;
    mp_int32 iErr = 0;
    mp_char szErr[ARRAY_BYTES_256] = {0};
    STORAGE_DEVICE_NUMBER strNum = {0};

    COMMLOG(OS_LOG_DEBUG, "Begin get disk num.");
    fHandle = CreateFile(strDeviceName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    if (fHandle == INVALID_HANDLE_VALUE) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "Open device failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    iRet = DeviceIoControl(fHandle,
        IOCTL_STORAGE_GET_DEVICE_NUMBER,
        &strNum,
        sizeof(STORAGE_DEVICE_NUMBER),
        &strNum,
        sizeof(STORAGE_DEVICE_NUMBER),
        &ulReturn,
        NULL);
    if (!iRet) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "Get device number failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        (mp_void) CloseHandle(fHandle);
        return MP_FAILED;
    }

    iDiskNum = (mp_int32)strNum.DeviceNumber;
    (mp_void) CloseHandle(fHandle);
    COMMLOG(OS_LOG_INFO, "Get disk num succ, disk num %d.", iDiskNum);
    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  : 获取分区信息列表
Output       :  rvecSubareaInfo---分区信息
Return       :  MP_SUCCESS---获取成功
               MP_FAILED---获取失败
------------------------------------------------------------- */
mp_int32 Disk::GetSubareaInfoList(vector<sub_area_Info_t>& rvecSubareaInfo)
{
    LOGGUARD("");
    mp_char acVolumeName[DISK_PATH_MAX] = {0};
    sub_area_Info_t strPartInfo;
    map<mp_string, mp_string> mapVol;
    DWORD dwFlg = 0;

    mp_int32 iRet = Disk::GetVolPaths(mapVol);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get volume paths failed, iRet %d.", iRet);
        return iRet;
    }

    FILE_HANDLE FindHandle = FindFirstVolume(acVolumeName, ARRAYSIZE(acVolumeName));
    if (FindHandle == INVALID_HANDLE_VALUE) {
        mp_ulong ulError = GetLastError();
        COMMLOG(OS_LOG_ERROR,
            "FindNextVolume failed with error code(%lu) when get subares info "
            "on windows",
            ulError);
        return MP_FAILED;
    }

    if (AssignVolumeInfo(FindHandle, acVolumeName, DISK_PATH_MAX, mapVol, strPartInfo) != MP_SUCCESS) {
        return MP_FAILED;
    }

    (mp_void) FindVolumeClose(FindHandle);

    for (vector<sub_area_Info_t>::iterator itSubareaInfoWin = rvecSubareaInfo.begin();
         itSubareaInfoWin != rvecSubareaInfo.end();
         ++itSubareaInfoWin) {
        COMMLOG(OS_LOG_DEBUG, "device=%s, fs=%s, path=%s, lable=%s, vol=%s, disk=%d, lba=%lld, cp=%lld",
            itSubareaInfoWin->acDeviceName, itSubareaInfoWin->acFileSystem, itSubareaInfoWin->acDriveLetter,
            itSubareaInfoWin->acVolLabel, itSubareaInfoWin->acVolName, itSubareaInfoWin->iDiskNum,
            itSubareaInfoWin->llOffset, itSubareaInfoWin->ullTotalCapacity);
    }

    return MP_SUCCESS;
}

mp_int32 Disk::AssignVolumeInfo(const FILE_HANDLE& FindHandle, mp_char acVolumeName[],
    mp_int32 buffLen, map<mp_string, mp_string>& mapVol, sub_area_Info_t& strPartInfo)
{
    do  {
        sub_area_Info_t strPartInfo;
        map<mp_string, mp_string> mapVol;
        errno_t erRes = EOK;
        erRes = memset_s(&strPartInfo, sizeof(sub_area_Info_t), 0, sizeof(sub_area_Info_t));
        if (erRes != EOK) {
            COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", erRes);
            (mp_void) FindVolumeClose(FindHandle);
            return MP_FAILED;
        }

        CHECK_FAIL(snprintf_s(strPartInfo.acVolName, sizeof(strPartInfo.acVolName), sizeof(strPartInfo.acVolName) - 1,
            "%s", acVolumeName));

        mp_uint32 uiIndex = strlen(acVolumeName) - 1;  // 去掉'\\?\'和最后的'\'

        acVolumeName[uiIndex] = '\0';  // QueryDosDeviceW doesn't allow a trailing backslash

        // 去掉前面的\\?\，QueryDosDevice函数需要去掉，故从第四个字符开始
        if (QueryDosDevice(&acVolumeName[DISK_INDEX_4], strPartInfo.acDeviceName,
            ARRAYSIZE(strPartInfo.acDeviceName)) == 0) {
            mp_ulong ulError = GetLastError();
            COMMLOG(OS_LOG_ERROR, "QueryDosDevice failed with error code(%lu).", ulError);
            continue;
        }
        acVolumeName[uiIndex] = '\\';

        if (DRIVE_FIXED == GetDriveType(acVolumeName)) {
            map<mp_string, mp_string>::iterator itMap = mapVol.find(acVolumeName);
            if (itMap != mapVol.end()) {
                CHECK_FAIL(snprintf_s(strPartInfo.acDriveLetter, sizeof(strPartInfo.acDriveLetter),
                    sizeof(strPartInfo.acDriveLetter) - 1, itMap->second.c_str()));
                strPartInfo.acDriveLetter[strlen(strPartInfo.acDriveLetter) - DISK_INDEX_2] = '\0';  // 去掉末尾的":\"
            }

            acVolumeName[uiIndex] = '\0';
            if (MP_SUCCESS != Disk::GetVolumeDiskInfo(acVolumeName, DISK_PATH_MAX, strPartInfo)) {
                COMMLOG(OS_LOG_ERROR, "Get volume disk info error,vol(%s) get subares info on windows", acVolumeName);
                continue;
            }

            acVolumeName[uiIndex] = '\\';

            // 获取文件系统类型
            vector<sub_area_Info_t> rvecSubareaInfo;
            DWORD dwFlg = 0;
            (mp_void) GetVolumeInformation(acVolumeName, strPartInfo.acVolLabel, sizeof(strPartInfo.acVolLabel),
                NULL, 0, &dwFlg, strPartInfo.acFileSystem, sizeof(strPartInfo.acFileSystem));
            rvecSubareaInfo.push_back(strPartInfo);  // 将分区信息加入到list中
        }
    } while (MP_SUCCESS == Disk::GetNextVolumeInformation(FindHandle, acVolumeName, strlen(acVolumeName)));
    return MP_SUCCESS;
}

// Description  : 获取磁盘信息列表
mp_int32 Disk::GetDiskInfoList(vector<disk_info>& vecDiskInfoWin)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecDiskName;
    vector<mp_string>::iterator itDiskNameLst;
    disk_info stDiskInfo;
    mp_string strVendor;
    mp_string strProduct;
    mp_bool bRet = MP_FALSE;

    COMMLOG(OS_LOG_INFO, "Begin get lun infos in windows.");

    // 获取主机上有硬盘名称
    (void)GetAllDiskWin(vecDiskName);
    if (vecDiskName.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "Disk name list is empty when get disk name on windows.");
        return MP_SUCCESS;
    }

    GetDiskInfo(vecDiskName, stDiskInfo, strVendor, strProduct);

    if (vecDiskInfoWin.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "Disk info list is empty when get disk info on windows.");
    }

    for (vector<disk_info>::iterator itDiskInfoWin = vecDiskInfoWin.begin(); itDiskInfoWin != vecDiskInfoWin.end();
         ++itDiskInfoWin) {
        COMMLOG(OS_LOG_DEBUG,
            "sn=%s, id=%s, wwn=%s, disk=%d",
            itDiskInfoWin->strArraySN.c_str(),
            itDiskInfoWin->strLUNID.c_str(),
            itDiskInfoWin->strLUNWWN.c_str(),
            itDiskInfoWin->iDiskNum);
    }

    COMMLOG(OS_LOG_INFO, "Get lun infos in windows succ.");

    return MP_SUCCESS;
}

mp_void Disk::GetDiskInfo(const std::vector<mp_string> &vecDiskName,
    disk_info &stDiskInfo, mp_string &strVendor, mp_string &strProduct)
{
    vector<disk_info> vecDiskInfoWin;
    mp_bool bRet = MP_FALSE;
    mp_int32 iRet;
    vector<mp_string>::iterator itDiskNameLst;
    // 获取LUN的ID、WWN、阵列SN以及对应的disk number
    for (itDiskNameLst == vecDiskName.begin(); itDiskNameLst != vecDiskName.end(); ++itDiskNameLst) {
        stDiskInfo.iDiskNum = 0;
        stDiskInfo.strArraySN = "";
        stDiskInfo.strLUNID = "";
        stDiskInfo.strLUNWWN = "";

        // 阵列的厂商和型号
        iRet = Array::GetArrayVendorAndProduct(*itDiskNameLst, strVendor, strProduct);
        if (iRet == MP_FAILED) {
            continue;
        }

        strVendor = CMpString::Trim(strVendor);
        strProduct = CMpString::Trim(strProduct);
        // 排除掉非华为的产品
        bRet = (strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI.c_str()) != 0 &&
                strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI.c_str()) != 0 &&
                strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY.c_str()) != 0 &&
                strcmp(strVendor.c_str(), ARRAY_VENDER_FUSION_STORAGE.c_str()) != 0);
        if (bRet) {
            continue;
        }

        iRet = Array::GetArraySN(*itDiskNameLst, stDiskInfo.strArraySN);  // 获取SN
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Failed to get array SN on windows.");
            continue;
        }

        iRet = Array::GetLunInfo(*itDiskNameLst, stDiskInfo.strLUNWWN, stDiskInfo.strLUNID);  // 获取LUN info
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Failed to get LUN info on windows.");
            continue;
        }

        iRet = Disk::GetDiskNum(*itDiskNameLst, stDiskInfo.iDiskNum);  // 获取disknumber
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Failed to get disk number on windows.");
            continue;
        }

        vecDiskInfoWin.push_back(stDiskInfo);
    }
}

/* ------------------------------------------------------------
Description  : 获取所有磁盘信息列表
Output       :  vecDiskName---磁盘名称组
Return       :  MP_SUCCESS---获取成功
------------------------------------------------------------- */
mp_int32 Disk::GetAllDiskWin(vector<mp_string>& vecDiskName)
{
    mp_int64 iIndex = 0;
    mp_int32 iRet = MP_SUCCESS;
    mp_char acDosLinkName[NAME_PATH_LEN] = {0};
    HDEVINFO hIntDevInfo;

    vecDiskName.clear();
    COMMLOG(OS_LOG_INFO, "Begin get all lun infos in windows.");
    // 创建枚举设备句柄
    hIntDevInfo = SetupDiGetClassDevs(&DiskClassGuid, NULL, NULL, (DIGCF_PRESENT | DIGCF_INTERFACEDEVICE));
    for (;;) {
        iRet = memset_s(acDosLinkName, sizeof(acDosLinkName), 0, sizeof(acDosLinkName));
        if (iRet != EOK) {
            COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iRet);
            (mp_void) SetupDiDestroyDeviceInfoList(hIntDevInfo);
            return MP_SUCCESS;
        }

        // 查询磁盘路径
        iRet = GetDiskPath(hIntDevInfo, iIndex, acDosLinkName, NAME_PATH_LEN);
        if (iRet == DISK_NORECORD_ERROR) {
            COMMLOG(OS_LOG_DEBUG, "Disk is not recorded, then return succ.");
            (mp_void) SetupDiDestroyDeviceInfoList(hIntDevInfo);
            return MP_SUCCESS;
        } else if (iRet == MP_FAILED) {
            COMMLOG(OS_LOG_WARN, "GetDiskPath is failed, then continue.");
            iIndex++;
            continue;
        }
        mp_string strAcDosLinkName = acDosLinkName;
        vecDiskName.push_back(strAcDosLinkName);
        COMMLOG(OS_LOG_DEBUG, "Disk name %s.", acDosLinkName);
        iIndex++;
    }

    (mp_void) SetupDiDestroyDeviceInfoList(hIntDevInfo);

    COMMLOG(OS_LOG_INFO, "Get all lun infos in windows end.");
}
/*------------------------------------------------------------
Description  : 通过卷名称获取卷路径
Output       :  mapPath---映射路径

-------------------------------------------------------------*/
mp_void Disk::GetVolPathByVolName(map<mp_string, mp_string>& mapPath)
{
    mp_ulong ulDrive;
    mp_char cDriveNum = 'A';  // 盘符从A开始
    mp_char acVolName[MAX_PATH_LEN];
    mp_int32 iRet = 0;
    mp_string strTmp = "";

    ulDrive = GetLogicalDrives();

    for (cDriveNum = 'A'; cDriveNum <= 'Z'; ++cDriveNum) {
        if (ulDrive & 0x01) {
            strTmp = cDriveNum;
            strTmp += ":\\";

            iRet = memset_s(acVolName, sizeof(acVolName), 0, sizeof(acVolName));
            if (iRet != EOK) {
                COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iRet);
                return;
            }

            iRet = GetVolumeNameForVolumeMountPoint(strTmp.c_str(), acVolName, sizeof(acVolName));
            if ((iRet != 0) && (strlen(acVolName) != 0)) {
                (mp_void) mapPath.insert(make_pair(acVolName, strTmp));
            }
        }

        ulDrive = ulDrive >> 1;
    }
}
/* ------------------------------------------------------------
Description  : 获取磁盘卷信息
Input        : pcVolumeName---PC卷名称
Output       :pstrPartInfo---磁盘部分信息
Return       :  MP_SUCCESS---获取成功
                MP_FAILED---获取失败
------------------------------------------------------------- */
mp_int32 Disk::GetVolumeDiskInfo(mp_char pcVolumeName[], mp_int32 bufferLen, sub_area_Info_t& subAreaInfo)
{
    mp_ulong ulReCount;
    mp_ulong ulCount;
    mp_ulong ulBufferSize = sizeof(VOLUME_DISK_EXTENTS) + (MAX_DISK_EXTENS - 1) * sizeof(DISK_EXTENT);
    PVOLUME_DISK_EXTENTS pstrVolumeDiskExtents;

    HANDLE hDevice = CreateFile(pcVolumeName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        COMMLOG(OS_LOG_ERROR, "%s", "Get volume disk info hDevice is INVALID_HANDLE_VALUE.");
        return MP_FAILED;
    }

    mp_char* pcOutBuffer = new (std::nothrow) mp_char[ulBufferSize];
    if (pcOutBuffer == NULL) {
        COMMLOG(OS_LOG_ERROR, "Call calloc failed.");
        return MP_FAILED;
    }

    mp_int32 iResult = DeviceIoControl(hDevice, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
        NULL, 0, pcOutBuffer, ulBufferSize, &ulReCount,
        (LPOVERLAPPED)NULL);
    if (iResult == 0) {
        if (ERROR_INSUFFICIENT_BUFFER == GetLastError()) {
            COMMLOG(OS_LOG_ERROR, "DeviceIoControl error! errorcode[%d] volname=%s.",
                GetLastError(), pcVolumeName);
        }

        (mp_void) CloseHandle(hDevice);
        free(pcOutBuffer);
        return MP_FAILED;
    }

    pstrVolumeDiskExtents = (PVOLUME_DISK_EXTENTS)pcOutBuffer;
    if (pstrVolumeDiskExtents->NumberOfDiskExtents > 1) {
        (mp_void) CloseHandle(hDevice);
        free(pcOutBuffer);
        COMMLOG(OS_LOG_ERROR, "%s", "Get partion info error disk when get volume disk info on windows");
        return MP_FAILED;
    }

    // 卷分区可以跨越多块磁盘，目前不考虑这种情况，只对应最后一块磁盘序号
    for (ulCount = 0; ulCount < pstrVolumeDiskExtents->NumberOfDiskExtents; ++ulCount) {
        subAreaInfo.iDiskNum = (mp_int32)pstrVolumeDiskExtents->Extents[ulCount].DiskNumber;
        subAreaInfo.llOffset = pstrVolumeDiskExtents->Extents[ulCount].StartingOffset.QuadPart;
        subAreaInfo.ullTotalCapacity = pstrVolumeDiskExtents->Extents[ulCount].ExtentLength.QuadPart;
    }

    (mp_void) CloseHandle(hDevice);
    free(pcOutBuffer);
    return (iResult);
}

Disk::Disk()
{
    m_hNtdll = NULL;
}

Disk::~Disk()
{
    FreeNtdllModule();
}
/* ------------------------------------------------------------
Description  : 关闭dll
------------------------------------------------------------- */
mp_void Disk::FreeNtdllModule()
{
    if (m_hNtdll) {
        FreeLibrary(m_hNtdll);
        m_hNtdll = NULL;
    }
}
/* ------------------------------------------------------------
Description  : 加载dll
Return       :  MP_TRUE---加载成功
            MP_FALSE---加载失败
------------------------------------------------------------- */
mp_bool Disk::LoadNtdllModule()
{
    LOGGUARD("");
    if (m_hNtdll != NULL) {
        COMMLOG(OS_LOG_ERROR, "%s", "ntdll handle is exists.");
        return MP_FALSE;
    }

    mp_char strSystemPath[MAX_PATH_LEN] = {0};
    int lenPath = GetSystemDirectory(strSystemPath, MAX_PATH_LEN);
    if (lenPath == 0) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[ARRAY_BYTES_256] = {0};
        COMMLOG(OS_LOG_ERROR, "Get system directory failed, errno [%d]: %s.",
            iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FALSE;
    }

    mp_string tmpSystemPath = strSystemPath;
    mp_string ntdllPath = tmpSystemPath + "\\ntdll.dll";
    m_hNtdll = LoadLibrary(TEXT(ntdllPath.c_str()));
    if (m_hNtdll == NULL) {
        COMMLOG(OS_LOG_ERROR, "LoadNtdllModule failed [%ld]", GetLastError());
        return MP_FALSE;
    }

    RtlInitUnicodeString = (RTLINITUNICODESTRING)GetProcAddress(m_hNtdll, "RtlInitUnicodeString");
    if (RtlInitUnicodeString == NULL) {
        COMMLOG(OS_LOG_ERROR, "RtlInitUnicodeString null, errorcode(%d).", GetLastError());
    }

    ZwOpenDirectoryObject = (ZWOPENDIRECTORYOBJECT)GetProcAddress(m_hNtdll, "ZwOpenDirectoryObject");
    if (ZwOpenDirectoryObject == NULL) {
        COMMLOG(OS_LOG_ERROR, "ZwOpenDirectoryObject null, errorcode(%d).", GetLastError());
    }

    ZwOpenSymbolicLinkObject = (ZWOPENSYMBOLICKLINKOBJECT)GetProcAddress(m_hNtdll, "ZwOpenSymbolicLinkObject");
    if (ZwOpenSymbolicLinkObject == NULL) {
        COMMLOG(OS_LOG_ERROR, "ZwOpenSymbolicLinkObject null, errorcode(%d).", GetLastError());
    }

    ZwQuerySymbolicLinkObject = (ZWQUERYSYMBOLICKLINKOBJECT)GetProcAddress(m_hNtdll, "ZwQuerySymbolicLinkObject");
    if (ZwQuerySymbolicLinkObject == NULL) {
        COMMLOG(OS_LOG_ERROR, "ZwQuerySymbolicLinkObject null, errorcode(%d).", GetLastError());
    }

    ZwClose = (ZWCLOSE)GetProcAddress(m_hNtdll, "ZwClose");
    if (ZwClose == NULL) {
        COMMLOG(OS_LOG_ERROR, "ZwClose null, errorcode(%d).", GetLastError());
    }

    if (((RtlInitUnicodeString == NULL) || (ZwOpenDirectoryObject == NULL) || (ZwOpenSymbolicLinkObject == NULL) ||
        (ZwQuerySymbolicLinkObject == NULL) || (ZwClose == NULL))) {
        return MP_FALSE;
    }
    return MP_TRUE;
}

/* ------------------------------------------------------------
Description  : 查询符号链接
Input        :  SymbolicLinkName---符号链接名字，isVolumn---卷判断
Output       : LinkTarget---链接目标
Return       :  MP_TRUE---加载成功
            MP_FALSE---加载失败
------------------------------------------------------------- */
mp_ulong Disk::QuerySymbolicLink(IN PUNICODE_STRING SymbolicLinkName, OUT PUNICODE_STRING LinkTarget, mp_bool isVolumn)
{
    OBJECT_ATTRIBUTES oa;
    HANDLE handle;

    UNICODE_STRING strDirName;
    OBJECT_ATTRIBUTES oba;
    HANDLE hDirectory = NULL;
    Defer _(nullptr, [&](...) {
        if (hDirectory != NULL) {
            ZwClose(hDirectory);
        }
    });

    LOGGUARD("");
    if (isVolumn == MP_TRUE) {
        RtlInitUnicodeString(&strDirName, L"\\global\?\?");
        InitObjectAttributes(&oba, &strDirName, OBJ_CASE_INSENSITIVE, NULL, NULL);
        mp_ulong ntStatus = ZwOpenDirectoryObject(&hDirectory, DIRECTORY_QUERY, &oba);
        if (ntStatus != STATUS_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Open directory object failed (0x%X), [%ld].", ntStatus, GetLastError());
            return ntStatus;
        }
    }

    InitObjectAttributes(&oa, SymbolicLinkName, OBJ_CASE_INSENSITIVE, hDirectory, 0);

    // ZwOpenSymbolicLinkObject returns STATUS_SUCCESS on success or the appropriate error status.
    mp_ulong status = ZwOpenSymbolicLinkObject(&handle, GENERIC_READ, &oa);
    if (status != STATUS_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "ZwOpenSymbolicLinkObject (%s) faild (0x%X), [%ld].",
            SymbolicLinkName->Buffer, status, GetLastError());
        return status;
    }

    LinkTarget->MaximumLength = MAX_PATH_LEN * sizeof(WCHAR);
    LinkTarget->Length = 0;
    LinkTarget->Buffer = (PWSTR)GlobalAlloc(GPTR, LinkTarget->MaximumLength);
    if (!LinkTarget->Buffer) {
        ZwClose(handle);
        COMMLOG(OS_LOG_ERROR, "GlobalAlloc faild (0x%X)", STATUS_INSUFFICIENT_RESOURCES);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(LinkTarget->Buffer, LinkTarget->MaximumLength);

    // ZwQuerySymbolicLinkObject returns either STATUS_SUCCESS to indicate the routine completed without error or
    // STATUS_BUFFER_TOO_SMALL if the Unicode string provided at LinkTarget is too small to hold the returned string.
    status = ZwQuerySymbolicLinkObject(handle, LinkTarget, NULL);
    ZwClose(handle);
    if (status != STATUS_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "ZwQuerySymbolicLinkObject faild (0x%X), [%ld].", status, GetLastError());
        GlobalFree(LinkTarget->Buffer);
        return status;
    }

    return status;
}

/* ------------------------------------------------------------
Description  : 初始化符号链接资源
Return       :  MP_TRUE---初始化成功
            MP_FALSE---初始化失败
------------------------------------------------------------- */
mp_bool Disk::InitSymboLinkRes()
{
    if (!LoadNtdllModule()) {
        COMMLOG(OS_LOG_ERROR, "LoadNtdllModule failed.");
        return MP_FALSE;
    }

    return MP_TRUE;
}

// Description  : 关闭符号链接资源
mp_void Disk::FreeSymboLinkRes()
{
    FreeNtdllModule();
}
/* ------------------------------------------------------------
Description  : 查询符号链接信息
Input        :   strSymboLink---符号链接名字，isVolumn---判断卷
Output       :  strTargetDevice---目标设备
Return       :   MP_TRUE---查询成功
             MP_FALSE---查询失败
------------------------------------------------------------- */
mp_bool Disk::QuerySymboLinkInfo(const mp_string& strSymboLink, mp_string& strTargetDevice, mp_bool isVolumn)
{
    mp_bool initFlag = (RtlInitUnicodeString == NULL) || (ZwOpenDirectoryObject == NULL) ||
                       (ZwOpenSymbolicLinkObject == NULL) || (ZwQuerySymbolicLinkObject == NULL) || (ZwClose == NULL);

    COMMLOG(OS_LOG_INFO, "Begin query symbolink info (%s).", strSymboLink.c_str());

    if (initFlag == MP_TRUE) {
        COMMLOG(OS_LOG_ERROR, "querySymboLinkInfo faild, not init resource.");
        return MP_FALSE;
    }

    mp_ulong status;
    UNICODE_STRING szSymbolicLink;
    UNICODE_STRING szDeviceName;
    WCHAR* lpszSymbolicLink = (PWSTR)GlobalAlloc(GPTR, MAX_PATH_LEN * sizeof(WCHAR));
    if (!lpszSymbolicLink) {
        COMMLOG(OS_LOG_ERROR, "GlobalAlloc faild.");
        return MP_FALSE;
    }

    mp_int32 iRet = swprintf_s(lpszSymbolicLink, MAX_PATH_LEN, L"%S", strSymboLink.c_str());
    if (iRet == MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "ISSP_SNPRINTF_S Faild.");
        GlobalFree(lpszSymbolicLink);
        return MP_FALSE;
    }

    RtlInitUnicodeString(&szSymbolicLink, lpszSymbolicLink);

    status = QuerySymbolicLink(&szSymbolicLink, &szDeviceName, isVolumn);
    if (status != STATUS_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "QuerySymbolicLink faild [0x%X].", status);
        if (!szDeviceName.Buffer) {
            GlobalFree(szDeviceName.Buffer);
        }
        if (!lpszSymbolicLink) {
            GlobalFree(lpszSymbolicLink);
        }
        return MP_FALSE;
    }

    wstring wStrTmp = szDeviceName.Buffer;
    strTargetDevice = CMpString::UnicodeToANSI(wStrTmp);
    GlobalFree(szDeviceName.Buffer);
    GlobalFree(lpszSymbolicLink);

    COMMLOG(OS_LOG_INFO, "Query symbolink info succ.");
    return MP_TRUE;
}

#else
/* ------------------------------------------------------------
Description  :判断设备是否存在
Input        :   strDiskName---磁盘名字
Return       :   MP_TRUE---存在
                 MP_FALSE---不存在
------------------------------------------------------------- */
mp_bool Disk::IsDeviceExist(mp_string& strDiskName)
{
    LOGGUARD("");
    // 组装设备全名
    mp_string strFullDiskName = "/dev/" + strDiskName;
    mp_string strBuf;

    // 打开设备需要root权限
    CRootCaller rootCaller;
    vector<mp_string> vecRlt;
    mp_int32 iRet = rootCaller.Exec(ROOT_COMMAND_CAPACITY, strFullDiskName, &vecRlt);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "CRootCaller::Exec failed, iRet is %d.", iRet);
        return MP_FALSE;
    }
    if (vecRlt.size() != 1) {
        COMMLOG(OS_LOG_ERROR, "vecRlt.size() is not equal to 1, the value is %d.", vecRlt.size());
        return MP_FALSE;
    } else {
        strBuf = vecRlt[0];
    }

    ISSP_SCSI_SENSE_HDR_S stSshdr;
    iRet = ScsiNormalizeSense(strBuf, stSshdr);
    if (iRet == MP_SUCCESS && stSshdr.ucSenseKey) {
        COMMLOG(OS_LOG_WARN, "ScsiNormalizeSense: iRet is %d, stSshdr.ucSenseKey is %d.", iRet, stSshdr.ucSenseKey);
        return MP_FALSE;
    }

    return MP_TRUE;
}
#endif  // WIN32
/* ------------------------------------------------------------
Description  :获取磁盘容量
Input        :   strDevice---磁盘名字
Output       :   strBuf---容量
Return       :   MP_TRUE---存在
             MP_FALSE---不存在
------------------------------------------------------------- */
mp_int32 Disk::GetDiskCapacity(mp_string& strDevice, mp_string& strBuf)
{
    LOGGUARD("");
#ifdef LINUX
    COMMLOG(OS_LOG_INFO, "Open scsi device = %s.", strDevice.c_str());
    mp_int32 iFd = open(strDevice.c_str(), O_RDWR | O_NONBLOCK);  // 打开scsi设备
    if (iFd < 0) {
        iFd = open(strDevice.c_str(), O_RDONLY | O_NONBLOCK);
    }
    if (iFd < 0) {
        mp_int32 iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "Open scsi device = %s failed,errno = [%d]", strDevice.c_str(), iErr);
        return MP_FAILED;
    }

    mp_int32 iRetryNum = 0;
    mp_bool bIsBigDisk = MP_FALSE;
    mp_bool bIsBreakOut = MP_TRUE;
    mp_uchar aucBuffer[DISK_BYTE_OF_SECTOR] = {0};
    mp_uchar ucSenseBuf[SCSI_MAX_SENSE_LEN] = {0};

    do {
        COMMLOG(OS_LOG_DEBUG, "Query capactiy info %s, time %d.", strDevice.c_str(), iRetryNum);
        sg_io_hdr stScsiCmd;
        CHECK_CLOSE_FD(memset_s(&stScsiCmd, sizeof(struct sg_io_hdr), 0, sizeof(struct sg_io_hdr)));
        SetSGHDR(stScsiCmd, bIsBigDisk);
        // CodeDex误报，FORTIFY.Path_Manipulation
        CHECK_CLOSE_FD(memset_s(aucBuffer, DISK_BYTE_OF_SECTOR, 0, DISK_BYTE_OF_SECTOR));
        stScsiCmd.dxferp = aucBuffer;
        // sense init
        CHECK_CLOSE_FD(memset_s(ucSenseBuf, SCSI_MAX_SENSE_LEN, 0, SCSI_MAX_SENSE_LEN));
        stScsiCmd.sbp = ucSenseBuf;

        // 把构造好的命令通过ioctl下发
        mp_int32 iRet = ioctl(iFd, SG_IO, &stScsiCmd);
        mp_bool bRet = (((mp_uchar)aucBuffer[0] == 0xff) && ((mp_uchar)aucBuffer[1] == 0xff) &&
                        ((mp_uchar)aucBuffer[DISK_INDEX_2] == 0xff) && ((mp_uchar)aucBuffer[DISK_INDEX_3]) == 0xff);
        iRet = HandleCapacityResult(stScsiCmd, iRetryNum, bIsBigDisk, bIsBreakOut, iRet, strDevice, bRet);
        if (iRet != MP_SUCCESS) {
            close(iFd);
            return iRet;
        }
    } while (!bIsBreakOut);

    strBuf = std::move(reinterpret_cast<mp_char*>(ucSenseBuf));
    close(iFd);
#endif
    return MP_SUCCESS;
}

#ifdef LINUX
mp_int32 Disk::HandleCapacityResult(sg_io_hdr& stScsiCmd, mp_int32& iRetryNum, mp_bool& bIsBigDisk,
    mp_bool& bIsBreakOut, mp_int32 iRet, const mp_string& strDevice, mp_bool bRet)
{
    // 执行成功，但是sg_io_hdr的status错误码为CHECK_CONDITION
    // (具体定义见/usr/include/scsi/scsi.h，获取具体值需要移位后再进行判断)，
    // sense code和sense key在sbp中，需要具体进行解析，解析方式参考内核代码
    // CHECK_CONDITION说明阵列侧发生了状态变化，需要主机端进行检查，
    // 当前简单处理在主机侧进行重试，如果重试三次还是存在问题，
    // 则还是和之前的处理策略相同，直接退出执行
    // 参考http://www.tldp.org/HOWTO/SCSI-Generic-HOWTO/sg_io_hdr_t.html
    // https://www.ibm.com/developerworks/cn/linux/l-scsi-api/
    // http://sg.danny.cz/sg/p/sg_v3_ho.html
    mp_bool bCheckRst = (iRetryNum < DISK_INDEX_3 && iRet == 0 && ((stScsiCmd.status >> 1) & 0x7F) == 1);
    if (bCheckRst) {
        COMMLOG(OS_LOG_WARN,
            "Ioctl failed %s, sg_io_hdr.status %d(CHECK_CONDITION), retry to do it %d.",
            strDevice.c_str(),
            stScsiCmd.status,
            iRetryNum);
        bIsBreakOut = MP_FALSE;
        ++iRetryNum;
        DoSleep(DISK_NUM_100);
        return MP_SUCCESS;
    }

    bCheckRst = (iRet < 0 || 0 != stScsiCmd.status);
    if (bCheckRst) {
        COMMLOG(OS_LOG_ERROR,
            "Ioctl failed %s, ret %d, cmdStatus %d, len %d.",
            strDevice.c_str(),
            iRet,
            stScsiCmd.status,
            stScsiCmd.mx_sb_len);
        return MP_FAILED;
    }

    if (!bIsBigDisk) {
        if (bRet) {
            bIsBreakOut = MP_FALSE;
            bIsBigDisk = MP_TRUE;
            return MP_SUCCESS;
        }
        bIsBreakOut = MP_TRUE;
    } else {
        bIsBreakOut = MP_TRUE;
    }

    return MP_SUCCESS;
}

mp_void Disk::SetSGHDR(sg_io_hdr& stScsiCmd, mp_bool bIsBigDisk)
{
    stScsiCmd.interface_id = 'S';
    stScsiCmd.dxfer_direction = SG_DXFER_FROM_DEV;
    mp_uchar auCdb[CDB16GENERIC_LENGTH] = {0};
    if (!bIsBigDisk) {
        auCdb[0] = SCSIOP_READ_CAPACITY;
        // 下发给设备的命令
        stScsiCmd.cmdp = auCdb;
        stScsiCmd.cmd_len = CDB10GENERIC_LENGTH;
    } else {
        auCdb[0] = 0x9e;
        auCdb[1] = 0x10;
        auCdb[DISK_NUM_13] = DISK_NUM_12;

        stScsiCmd.cmdp = auCdb;  // 下发给设备的命令
        stScsiCmd.cmd_len = CDB16GENERIC_LENGTH;
    }

    // 下发ioctl的超时时间
    stScsiCmd.timeout = SCSI_CMD_TIMEOUT_LINUX;

    stScsiCmd.dxfer_len = DISK_BYTE_OF_SECTOR;
    stScsiCmd.mx_sb_len = SCSI_MAX_SENSE_LEN;
}
#endif

// Description  :获取Scsi NormalizeSense 的功能
mp_int32 Disk::ScsiNormalizeSense(mp_string& strBuf, ISSP_SCSI_SENSE_HDR_S& stSSHdr)
{
    LOGGUARD("");
    CHECK_NOT_OK(memset_s(&stSSHdr, sizeof(ISSP_SCSI_SENSE_HDR_S), 0, sizeof(ISSP_SCSI_SENSE_HDR_S)));
    stSSHdr.ucResponseCode = ((mp_uchar)strBuf[0] & 0x7f);

    if ((stSSHdr.ucResponseCode & 0x70) == 0x70) {
        COMMLOG(OS_LOG_ERROR, "stSSHdr.ucResponseCode is %d.", stSSHdr.ucResponseCode);
        return MP_FAILED;
    }

    std::size_t iSBLen = strBuf.length();
    if (stSSHdr.ucResponseCode >= 0x72) {
        // descriptor format
        if (iSBLen > 1) {
            stSSHdr.ucSenseKey = ((mp_uchar)strBuf[1] & 0xf);
        }

        if (iSBLen > DISK_INDEX_2) {
            stSSHdr.ucAsc = (mp_uchar)strBuf[DISK_INDEX_2];
        }

        if (iSBLen > DISK_INDEX_3) {
            stSSHdr.ucAscq = (mp_uchar)strBuf[DISK_INDEX_3];
        }

        if (iSBLen > DISK_INDEX_7) {
            stSSHdr.ucAdditionalLength = (mp_uchar)strBuf[DISK_INDEX_7];
        }
        return MP_SUCCESS;
    }
    return ScsiNormalizeSenseAfter(strBuf, stSSHdr, iSBLen);
}

mp_int32 Disk::ScsiNormalizeSenseAfter(mp_string& strBuf, ISSP_SCSI_SENSE_HDR_S& stSSHdr, mp_int32 iSBLen)
{
    if (iSBLen > DISK_INDEX_2) {
        stSSHdr.ucSenseKey = ((mp_uchar)strBuf[DISK_INDEX_2] & 0xf);
    }
    if (iSBLen > DISK_INDEX_7) {
        iSBLen = (iSBLen < (strBuf[DISK_INDEX_7] + DISK_INDEX_8)) ? iSBLen : (strBuf[DISK_INDEX_7] + DISK_INDEX_8);

        if (iSBLen > DISK_INDEX_12) {
            stSSHdr.ucAsc = (mp_uchar)strBuf[DISK_INDEX_12];
        }

        if (iSBLen > DISK_INDEX_13) {
            stSSHdr.ucAscq = (mp_uchar)strBuf[DISK_INDEX_13];
        }
    }
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

// Description  :判断是否sd型磁盘(LINUX)
mp_bool Disk::IsSdisk(mp_string& strDevice)
{
    mp_string::size_type pos = strDevice.find("sd");
    return (pos == 0) ? MP_TRUE : MP_FALSE;
}
/* ------------------------------------------------------------
Description  :获取磁盘状态
Input        :    strDiskName---磁盘名
Output       :  strStatus---磁盘状态值
Return       :   MP_SUCCESS---获取成功
             MP_FAILED---获取失败，iRet---对应错误码
------------------------------------------------------------- */
mp_int32 Disk::GetDiskStatus(mp_string& strDiskName, mp_string& strStatus)
{
    LOGGUARD("");
    CHECK_FAIL_EX(CheckCmdDelimiter(strDiskName));
    mp_string strCmd = "lsdev -Cc disk | awk '$1==\"" + strDiskName + "\" {print $2}'";
    vector<mp_string> vecRlt;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Exec system cmd failed, cmd is %s, iRet is %d.", strCmd.c_str(), iRet);
        return iRet;
    }

    if (vecRlt.size() < 1) {
        COMMLOG(OS_LOG_ERROR, "vecRlt.size() is smaller than 1, the value is %d.", vecRlt.size());
        return MP_FAILED;
    } else {
        strStatus = vecRlt[0];
    }
    return MP_SUCCESS;
}

// Description  :判断是否hdisk型磁盘(AIX)
mp_bool Disk::IsHdisk(mp_string& strDevice)
{
    mp_string::size_type pos = strDevice.find("hdisk");
    return (pos == 0) ? MP_TRUE : MP_FALSE;
}
/* ---------------------------------------------------------------------
Function Name: GetDevNameByWWN
Description  : 根据lun wwn获取对应的设备名称，不带"/dev/"前缀
Input        : lunWWN LUN WWN；如果是多个LUN组成一个vg，server端有可能下发以逗号分隔的WWN串，
           这时返回的是其中最前面lun所在的dev名称；
           如果需要精确匹配，请拆分成准确的lun wwn，分别调用
Others       :-------------------------------------------------------- */
mp_int32 Disk::GetDevNameByWWN(mp_string& strDevName, const mp_string& strWWN)
{
#ifndef WIN32
    vector<mp_string> vecDiskName;
    mp_string lunWWN;
    mp_string newDiskName;

    mp_int32 iRet = Disk::GetAllDiskName(vecDiskName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get all disk name failed, iRet = %d.", iRet);
        return iRet;
    }

    mp_string lunID;
    for (vector<mp_string>::iterator it = vecDiskName.begin(); it != vecDiskName.end(); ++it) {
        // 过滤掉非华为厂商设备   , 前面添加"/dev"
#if defined(LINUX) || defined(AIX)
        newDiskName = mp_string("/dev/") + *it;  // 拼装全路径
#elif defined(HP_UX_IA)
        iRet = Disk::GetHPRawDiskName(*it, newDiskName);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get full disk name of disk(%s) failed, ret %d.", it->c_str(), iRet);
            return ERROR_DISK_GET_RAW_DEVICE_NAME_FAILED;
        }
#endif

        if (CheckHuaweiDisk(newDiskName) == MP_FALSE) {  // 如果不是华为磁盘，则继续下一个磁盘查询
            continue;
        }

        iRet = Array::GetLunInfo(newDiskName, lunWWN, lunID);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "GetLunInfo failed, iRet=%d.", iRet);
            return iRet;
        }
        COMMLOG(OS_LOG_DEBUG, "disk %s LUN WWN: %s, LUN ID: %s", newDiskName.c_str(), lunWWN.c_str(), lunID.c_str());
        COMMLOG(OS_LOG_DEBUG, "Input WWN is %s", strWWN.c_str());
        // 存在多个lun组成一个vg的场景，server下发的wwn以分号分隔
        mp_string::size_type pos = strWWN.find(lunWWN);
        if (mp_string::npos != pos) {
            strDevName = *it;  // 返回参数不加/dev前缀
            break;
        }
    }

    if (strDevName.empty()) {
        COMMLOG(OS_LOG_ERROR, "Can not find device by WWN \"%s\".", strWWN.c_str());
        return ERROR_COMMON_DEVICE_NOT_EXIST;
    }
#endif
    return MP_SUCCESS;
}

mp_bool Disk::CheckHuaweiDisk(mp_string& newDiskName)  // 判断是否是华为Disk
{
#ifndef WIN32
    mp_string strVendor;
    mp_string strProduct;

    mp_int32 iRet = Array::GetArrayVendorAndProduct(newDiskName, strVendor, strProduct);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "Get Array Vendor And Product failed.", newDiskName.c_str());
        return MP_FALSE;  // 本地磁盘会返回失败，直接跳过;
    }
    strVendor = CMpString::Trim(strVendor);
    strProduct = CMpString::Trim(strProduct);

    if ((strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI.c_str()) != 0) &&
        (strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI.c_str()) != 0) &&
        (strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY.c_str()) != 0) &&
        (strcmp(strVendor.c_str(), ARRAY_VENDER_FUSION_STORAGE.c_str()) != 0)) {
        COMMLOG(OS_LOG_DEBUG, "device vendor %s is not huawei", strVendor.c_str());
        return MP_FALSE;
    }
#endif

    return MP_TRUE;
}
