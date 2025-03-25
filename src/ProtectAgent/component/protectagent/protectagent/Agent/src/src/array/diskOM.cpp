#include "array/diskOM.h"
#ifndef WIN32
#include <sys/ioctl.h>
#endif
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
#include "securecom/SecureUtils.h"
using namespace std;

/*------------------------------------------------------------
Description  : 判断是不是SSD硬盘
Input        : strDeviceName  设备名字,/dev/sdx、/dev/vxdx
Output       : mediaType "Unspecified" "HDD" "SSD" "SCM";
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : z00455045
Modification : 无
-------------------------------------------------------------*/
#ifdef WIN32
mp_int32 DiskOM::GetDiskHWType(vector<mp_string>& vecResult, media_type_t& mediaType, mp_string& strDeviceName,
    mp_string& strDeviceNum)
{
    mp_string tempMediaType;
    mp_string deviceIdStr;
    map<mp_string, media_type_t> mapMediaType;
    mapMediaType.insert(std::pair<mp_string, media_type_t>("Unspecified", MEDIA_TYPE_UNSPECIFIED));
    mapMediaType.insert(std::pair<mp_string, media_type_t>("HDD", MEDIA_TYPE_HDD));
    mapMediaType.insert(std::pair<mp_string, media_type_t>("SSD", MEDIA_TYPE_SSD));
    mapMediaType.insert(std::pair<mp_string, media_type_t>("SCM", MEDIA_TYPE_SCM));
    
    vector<mp_string>::iterator iter;
    for (iter = vecResult.begin(); iter != vecResult.end(); iter++) {
        std::size_t found = iter->find("MediaType");
        if (found != std::string::npos) {
            found = iter->find(":");
            if (found == std::string::npos) {
                COMMLOG(OS_LOG_ERROR, "The WMI output info [\"MediaType\"] format is not supported.");
                return MP_FAILED;
            }
            tempMediaType = iter->substr(found + 2); // +2 for trim left space
        }

        found = iter->find("DeviceId");
        if (found != std::string::npos) {
            found = iter->find(":");
            if (found == std::string::npos) {
                COMMLOG(OS_LOG_ERROR, "The WMI output info [\"DeviceId\"] format is not supported.");
                return MP_FAILED;
            }

            deviceIdStr = iter->substr(found + 2); // +2 for trim left space
            if (deviceIdStr.compare(strDeviceNum) == 0) {
                map<mp_string, media_type_t>::iterator iterMediaType = mapMediaType.find(tempMediaType);
                mediaType = ((iterMediaType == mapMediaType.end()) ? MEDIA_TYPE_UNSPECIFIED :
                    iterMediaType->second);
                COMMLOG(OS_LOG_INFO, "The disk [%s] MediaType is %u.", strDeviceName.c_str(), mediaType);
                return MP_SUCCESS;
            }
        }
    }
    mediaType = MEDIA_TYPE_UNSPECIFIED;
    return MP_SUCCESS;
}
mp_int32 DiskOM::IsSSD(media_type_t& mediaType, mp_string& strDeviceName, mp_string& strDeviceNum)
{
    // get disk mediatype by WMI
    LOGGUARD("");
    mp_int32 iRettmp = MP_SUCCESS;
    mp_string strScriptName = GET_DEVICE_MEDIA_TYPE_BAT;
    mp_string strScriptParam = "";
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_DEBUG, "Begin query disk media type info.");

    mp_int32 iRet = SecureCom::SysExecScript(strScriptName, strScriptParam, &vecResult);
    if (iRet != MP_SUCCESS) {
        iRettmp = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, "Exec script failed, initial return code is %d, tranformed return code is %d",
            iRet, iRettmp);
        iRet = iRettmp;
        return iRet;
    }

    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "The result info is empty, script device_media_type.bat.");
        return iRet;
    }

    if (GetDiskHWType(vecResult, mediaType, strDeviceName, strDeviceNum) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get disk hardware type failed.");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Check disk type succ.");
    return MP_SUCCESS;
}
#elif defined LINUX
mp_int32 DiskOM::IsSSD(media_type_t& mediaType, mp_string& strDeviceName, mp_string& strDeviceNum)
{
    (mp_void)strDeviceNum;
    vector<mp_string> vecRlt;
    mp_string strFile;
    mp_string strDevice = strDeviceName;

    COMMLOG(OS_LOG_DEBUG, "Begin check disk type, device name %s.", strDeviceName.c_str());
    mp_string::size_type iLastIndex = strDeviceName.rfind("/");
    if (iLastIndex != mp_string::npos) {
        strDevice = strDevice.substr(iLastIndex + 1);
    }

    strFile = "/sys/block/" + strDevice + "/queue/rotational";
    COMMLOG(OS_LOG_DEBUG, "read rotational file %s.", strFile.c_str());
    mp_int32 iRet = CMpFile::ReadFile(strFile, vecRlt);
    if (iRet != MP_SUCCESS || vecRlt.empty()) {
        COMMLOG(OS_LOG_ERROR, "read rotational file failed, file %s.", strFile.c_str());
        return MP_FAILED;
    }

    mp_string strRotational = vecRlt.front();
    (void)CMpString::Trim(strRotational.c_str());
    mp_int32 iRotational = atoi(strRotational.c_str());
    mediaType = ((iRotational == 0) ? MEDIA_TYPE_SSD : MEDIA_TYPE_HDD);

    COMMLOG(OS_LOG_DEBUG, "Check disk type succ.");
    return MP_SUCCESS;
}
#else
mp_int32 DiskOM::IsSSD(media_type_t& mediaType, mp_string& strDeviceName, mp_string& strDeviceNum)
{
    COMMLOG(OS_LOG_ERROR, "Unimplement func.");
    (mp_void)mediaType;
    (mp_void)strDeviceName;
    (mp_void)strDeviceNum;
    return MP_FAILED;
}
#endif

#ifdef WIN32
mp_int32 DiskOM::GetDiskSize(mp_string& strDevieName, mp_uint64& iDiskSize)
{
    DISK_GEOMETRY_EX pdg = {0};
    DWORD junk = 0;                       // discard results
    mp_int32 iErr = 0;
    mp_char szErr[ERR_INFO_SIZE] = {0};

    HANDLE hDevice = CreateFile(strDevieName.c_str(),  // drive to open
        0,                      // no access to the drive
        FILE_SHARE_READ | FILE_SHARE_WRITE,      // share mode
        NULL,                   // default security attributes
        OPEN_EXISTING,          // disposition
        0,                      // file attributes
        NULL);                  // do not copy file attributes
    if (hDevice == INVALID_HANDLE_VALUE) {      // cannot open the drive
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "Invoke CreateFile failed, dev %s, errno[%d]: %s.", strDevieName.c_str(),
            iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    BOOL bResult = DeviceIoControl(hDevice,                          // device to be queried
        IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,   // operation to perform
        NULL, 0,                            // no input buffer
        &pdg, sizeof(pdg),                  // output buffer
        &junk,                              // # bytes returned
        (LPOVERLAPPED) NULL);               // synchronous I/O
    if (!bResult) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "Invoke DeviceIoControl failed, dev %s, errno[%d]: %s.", strDevieName.c_str(),
            iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        CloseHandle(hDevice);
        return MP_FAILED;
    }

    iDiskSize = pdg.DiskSize.QuadPart;
    CloseHandle(hDevice);
    return MP_SUCCESS;
}
#elif defined LINUX
mp_int32 DiskOM::GetDiskSize(mp_string& strDevieName, mp_uint64& iDiskSize)
{
    mp_int32 iErr;
    mp_char szErr[ERR_INFO_SIZE] = {0};

    mp_int32 iFd = open(strDevieName.c_str(), O_RDONLY);
    if (iFd < 0) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "Open disk failed, dev %s, errno[%d]: %s.", strDevieName.c_str(),
            iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    mp_int32 iRet = ioctl(iFd, BLKGETSIZE64, &iDiskSize);
    if (iRet != 0) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "Get disk size failed, dev %s, errno[%d]: %s.", strDevieName.c_str(),
            iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        close(iFd);
        return MP_FAILED;
    }
    close(iFd);
    return MP_SUCCESS;
}
#else
mp_int32 DiskOM::GetDiskSize(mp_string& strDevieName, mp_uint64& iDiskSize)
{
    COMMLOG(OS_LOG_ERROR, "Unimplement func.");
    (mp_void)strDevieName;
    (mp_void)iDiskSize;
    return MP_FAILED;
}
#endif

