#include <string.h>
#include "rootexec/SystemCallOM.h"
#include "rootexec/SystemCall.h"
#include "common/RootCaller.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "common/Defines.h"
#include "common/CSystemExec.h"
#include "common/ErrorCode.h"
#include "securecom/UniqueId.h"
#include "common/Sign.h"
#include "array/array.h"
#include "array/disk.h"
#include "securec.h"
#include <algorithm>
#include "common/ConfigXmlParse.h"
#include "common/Sign.h"
#include "common/JsonUtils.h"
#include "common/Uuid.h"
#include "common/CryptAlg.h"
#include <sstream>

#ifdef LINUX
#include <sys/ioctl.h>
#include <sstream>
#include <arpa/inet.h>
#include "driver/linux/ctl_cmd.h"
#endif

#ifdef LIN_FRE_SUPP
#include <linux/fs.h>
#endif

using namespace std;

const mp_int32 BITMAP_ALREADY_SET = 17;
const mp_string REDHAT_SHUTDOWN_LOCK_FILE = "/var/lock/subsys/shutdown2iomirror";

// vmware data process used
const mp_int32 VMWARE_DATAPROCESS_PARAM_NUM = 2;
mp_int32 SystemCallOM::GetDiskSize(mp_string& strUniqueID)
{
    mp_int32 iRet;
#ifndef WIN32
    mp_uint64 iDiskSize = 0;
    mp_string strDevice = "";

    iRet = GetParamFromTmpFile(strUniqueID, strDevice);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    iRet = Disk::GetDiskSize(strDevice, iDiskSize);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get disk size failed, ret %d.", iRet);
        return iRet;
    }
    vector<mp_string> vecResult;
    mp_string str;
    stringstream ss_stream;
    ss_stream << iDiskSize;
    ss_stream >> str;
    vecResult.push_back(str.c_str());
    iRet = CIPCFile::WriteResult(strUniqueID, vecResult);
#endif
    return iRet;
}

/*------------------------------------------------------------
Description  : 开始Mobility保护
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::OMStartProtect(mp_string& strUniqueID)
{
    mp_string strParam;
    om_drv_protect_strategy_t protectStrategy;

    COMMLOG(OS_LOG_DEBUG, "Begin start protect.");
    mp_int32 iRet = GetParamFromTmpFile(strUniqueID, strParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, "Get start protect params %s.", strParam.c_str());

    iRet = ParseProtectStrategy(strParam, protectStrategy);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Parse protect strategy params failed, iRet %d.", iRet);
        return iRet;
    }

    iRet = SendIOControl(mp_string(MOBILITY_DEVICE_NAME), OM_IOCTL_START, &protectStrategy, sizeof(protectStrategy));
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Send ioctl start protect failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Start protect succ.");
    return iRet;
}

/*------------------------------------------------------------
Description  : 修改driver配置
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::OMModifyConf(mp_string& strUniqueID)
{
    mp_string strParam;
    om_drv_protect_strategy_t protectStrategy;

    COMMLOG(OS_LOG_DEBUG, "Begin modify driver configure.");
    mp_int32 iRet = GetParamFromTmpFile(strUniqueID, strParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Get modify configure params %s.", strParam.c_str());

    iRet = ParseProtectStrategy(strParam, protectStrategy);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Parse protect strategy params failed, iRet %d.", iRet);
        return iRet;
    }
    iRet = SendIOControl(mp_string(MOBILITY_DEVICE_NAME), OM_IOCTL_MODIFY, &protectStrategy, sizeof(protectStrategy));
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Send ioctl modify protect failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Modify driver configure succ.");
    return iRet;
}

mp_int32 SystemCallOM::DecryptToken(mp_string& strToken, om_drv_protect_strategy_t& protectStrategy)
{
    if (strToken == "") {
        CHECK_NOT_OK(memset_s(protectStrategy.tokenId, OM_DRIVER_TOKEN_ID_LEN, 0, OM_DRIVER_TOKEN_ID_LEN));
        COMMLOG(OS_LOG_INFO, "token is empty.");
        return MP_SUCCESS;
    }
    mp_string outStr;
    DecryptStr(strToken, outStr);
    if (outStr.empty()) {
        COMMLOG(OS_LOG_ERROR, "DecryptStr private token failed.");
        return MP_FAILED;
    }

    mp_uuid uuid;
#ifdef LINUX
    mp_int32 iRet = uuid_parse(outStr.c_str(), uuid);
    if (iRet != 0) {
        COMMLOG(OS_LOG_ERROR, "Convert string to uuid failed, returned iRet = %d", iRet);
        return MP_FAILED;
    }
#else
    (mp_void) uuid;
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif
    CHECK_NOT_OK(memcpy_s(protectStrategy.tokenId, sizeof(protectStrategy.tokenId), &uuid, sizeof(uuid)));
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 解析保护策略信息参数
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::ParseProtectStrategy(mp_string& strParams, om_drv_protect_strategy_t& protectStrategy)
{
    Json::Value jv;
    Json::Reader r;

    COMMLOG(OS_LOG_DEBUG, "Begin parse protect strategy info, strParams %s.", strParams.c_str());
    mp_bool bRet = r.parse(strParams, jv);
    if (bRet != MP_TRUE) {
        COMMLOG(OS_LOG_ERROR, "Parse params failed.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_string strHostId;
    mp_string strOmaId;
    mp_uuid uuid;

    // convert hostid string format to host id uuid format
    GET_JSON_STRING(jv, OM_DRV_PARAM_HOST_ID, strHostId);
    mp_int32 iRet = CUuidNum::CovertStandrdStrToUuid(strHostId, uuid);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Convert string to hostid uuid failed, iRet %d.", iRet);
        return iRet;
    }
    CHECK_NOT_OK(memcpy_s(protectStrategy.hostId, sizeof(protectStrategy.hostId), &uuid, sizeof(uuid)));

    // convert hostid string format to host id uuid format
    GET_JSON_STRING(jv, OM_DRV_PARAM_OMA_ID, strOmaId);
    iRet = CUuidNum::CovertStandrdStrToUuid(strOmaId, uuid);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Convert string to omaid uuid failed, iRet %d.", iRet);
        return iRet;
    }
    CHECK_NOT_OK(memcpy_s(protectStrategy.omaId, sizeof(protectStrategy.omaId), &uuid, sizeof(uuid)));

    mp_string strEncryptedToken;
    GET_JSON_STRING_OPTION(jv, OM_DRV_PARAM_TOKEN_ID, strEncryptedToken);
    iRet = DecryptToken(strEncryptedToken, protectStrategy);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Decrypt token failed, iRet %d.", iRet);
        return iRet;
    }

    GET_JSON_UINT32(jv, OM_DRV_PARAM_IP0, protectStrategy.ip[0]);
    GET_JSON_UINT32(jv, OM_DRV_PARAM_IP1, protectStrategy.ip[IP_SEG_1]);
    GET_JSON_UINT32(jv, OM_DRV_PARAM_IP2, protectStrategy.ip[IP_SEG_2]);
    GET_JSON_UINT32(jv, OM_DRV_PARAM_IP3, protectStrategy.ip[IP_SEG_3]);
    GET_JSON_UINT32(jv, OM_DRV_PARAM_PORT, protectStrategy.uiPort);
    GET_JSON_UINT32(jv, OM_DRV_PARAM_RPO, protectStrategy.uiRPO);
    GET_JSON_UINT32(jv, OM_DRV_PARAM_MEM_THRESHOLD, protectStrategy.uiMemThreshold);
    GET_JSON_UINT32(jv, OM_DRV_PARAM_PROTECT_SIZE, protectStrategy.uiProtectSize);
    GET_JSON_UINT32(jv, OM_DRV_PARAM_SEND_NOW, protectStrategy.sendNow);
    COMMLOG(OS_LOG_DEBUG, "Parse protect strategy info succ.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 解析受保护卷信息参数
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::ParseProtectVolume(mp_string& strParams, om_drv_protect_vol_t& protectVol)
{
    Json::Value jv;
    Json::Reader r;

    mp_bool bRet = r.parse(strParams, jv);
    if (bRet != MP_TRUE) {
        COMMLOG(OS_LOG_ERROR, "Parse params failed.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_string strVolId, strOldVolId;
    mp_uuid uuid;
    // vol Id
    GET_JSON_STRING(jv, OM_DRV_PARAM_VOL_ID, strVolId);
    mp_int32 iRet = CUuidNum::CovertStandrdStrToUuid(strVolId, uuid);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Convert vol id to uuid failed, iRet %d.", iRet);
        return iRet;
    }
    CHECK_NOT_OK(memcpy_s(protectVol.volId, sizeof(protectVol.volId), &uuid, sizeof(uuid)));

    // old vol Id
    if (jv.isMember(OM_DRV_PARAM_OLD_VOL_ID)) {
        GET_JSON_STRING(jv, OM_DRV_PARAM_OLD_VOL_ID, strOldVolId);
        iRet = CUuidNum::CovertStandrdStrToUuid(strOldVolId, uuid);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Convert old vol Id to uuid failed, iRet %d.", iRet);
            return iRet;
        }
        CHECK_NOT_OK(memcpy_s(protectVol.old_volId, sizeof(protectVol.old_volId), &uuid, sizeof(uuid)));
    }

    mp_string strDiskPath;
    GET_JSON_STRING(jv, OM_DRV_PARAM_DISK_PATH, strDiskPath);
    CHECK_NOT_OK(strncpy_s(
        protectVol.disk_path, sizeof(protectVol.disk_path), strDiskPath.c_str(), strlen(strDiskPath.c_str())));
    GET_JSON_UINT32(jv, OM_DRV_PARAM_DISK_NUM, protectVol.uiDiskNum);

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 停止Mobility保护
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::OMStopProtect(mp_string& strUniqueID)
{
    (mp_void) strUniqueID;
    COMMLOG(OS_LOG_DEBUG, "Begin to stop protect in systemcall.");
    mp_int32 iRet = SendIOControl(mp_string(MOBILITY_DEVICE_NAME), OM_IOCTL_STOP, NULL, 0);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Send ioctl stop protect failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Stop protect succ.");
    return iRet;
}

// pause_state value range is 0 or 1
mp_int32 SystemCallOM::OMPersistPauseState(const mp_uchar pause_state)
{
    const mp_uchar inPauseState = 1;
    const mp_uchar outPauseState = 0;

    if (pause_state != outPauseState && pause_state != inPauseState) {
        COMMLOG(OS_LOG_ERROR, "pause state value %u is error.", pause_state);
        return MP_FAILED;
    }

    if (CMpFile::FileExist(LINUX_DRIVER_CONF) == MP_FALSE) {
        COMMLOG(OS_LOG_ERROR, "driver configure file is not exist.");
        return MP_FAILED;
    }

    mp_int32 iErr;
    mp_char szErr[ERR_INFO_SIZE] = {0};

    mp_int32 fd = open(LINUX_DRIVER_CONF.c_str(), O_RDWR | O_SYNC);
    if (fd < 0) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "open driver configure file failed, error[%d]:%s.",
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    struct im_config_pg driverConfig;
    if (read(fd, &driverConfig, sizeof(struct im_config_pg)) != sizeof(struct im_config_pg)) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "read driver config file failed, error[%d]:%s.",
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        goto error_out;
    }

    driverConfig.reboot_pause_state = pause_state;

    if (lseek(fd, 0, SEEK_SET) < 0) {
        COMMLOG(OS_LOG_WARN, "lseek driver configure file failed.");
        goto error_out;
    }
    if (write(fd, &driverConfig, sizeof(driverConfig)) != sizeof(driverConfig)) {
        iErr = GetOSError();
        COMMLOG(
            OS_LOG_WARN, "Persist pause state failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        goto error_out;
    }

    COMMLOG(OS_LOG_INFO, "Persist pause state succ.");
    return MP_SUCCESS;

error_out:
    close(fd);
    return MP_FAILED;
}

/*------------------------------------------------------------
Description  : 暂停Mobility保护
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::OMPauseProtect(mp_string& strUniqueID)
{
    COMMLOG(OS_LOG_DEBUG, "Begin to pause protect in systemcall.");

    mp_string strParam;
    mp_int32 iRet = GetParamFromTmpFile(strUniqueID, strParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    WaitFlushQueue waitFlush;
    waitFlush.waitFlushQueueFlag = (strParam == "0" ? MP_FALSE : MP_TRUE);
    iRet = SendIOControl(mp_string(MOBILITY_DEVICE_NAME), OM_IOCTL_PAUSE, &waitFlush, sizeof(waitFlush));
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Send ioctl pause protect failed, iRet %d.", iRet);
        return iRet;
    }

    const mp_uchar inPauseState = 1;
    if (OMPersistPauseState(inPauseState) != MP_SUCCESS) {
        // may be send an alarm
        COMMLOG(OS_LOG_ERROR, "Persisit pause state failed.");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Pause protect succ.");
    return iRet;
}

/*------------------------------------------------------------
Description  : 恢复Mobility保护
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::OMResumeProtect(mp_string& strUniqueID)
{
    (mp_void) strUniqueID;
    COMMLOG(OS_LOG_DEBUG, "Begin to resume protect in systemcall.");
    mp_int32 iRet = SendIOControl(mp_string(MOBILITY_DEVICE_NAME), OM_IOCTL_RESUME, NULL, 0);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Send ioctl resume protect failed, iRet %d.", iRet);
        return iRet;
    }

    const mp_uchar outPauseState = 0;
    if (OMPersistPauseState(outPauseState) != MP_SUCCESS) {
        // may be send an alarm
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Resume protect succ.");
    return iRet;
}

/*------------------------------------------------------------
Description  : Mobility增加保护卷
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::OMAddVolume(mp_string& strUniqueID)
{
    mp_string strParam;
    om_drv_protect_vol_t protectVol;

    COMMLOG(OS_LOG_DEBUG, "Begin add volume.");
    mp_int32 iRet = GetParamFromTmpFile(strUniqueID, strParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    iRet = ParseProtectVolume(strParam, protectVol);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Parse protect volume params failed, ret %d.", iRet);
        return iRet;
    }
    iRet = SendIOControl(mp_string(MOBILITY_DEVICE_NAME), OM_IOCTL_VOL_ADD, &protectVol, sizeof(protectVol));
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Send ioctl add volume failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Add volume succ.");
    return iRet;
}

/*------------------------------------------------------------
Description  : Mobility删除保护卷
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::OMDelVolume(mp_string& strUniqueID)
{
    mp_string strParam;
    om_drv_protect_vol_t protectVol;

    COMMLOG(OS_LOG_DEBUG, "Begin delete volume.");
    mp_int32 iRet = GetParamFromTmpFile(strUniqueID, strParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    iRet = ParseProtectVolume(strParam, protectVol);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Parse protect volume params failed, ret %d.", iRet);
        return iRet;
    }
    iRet = SendIOControl(mp_string(MOBILITY_DEVICE_NAME), OM_IOCTL_VOL_DELETE, &protectVol, sizeof(protectVol));
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Send ioctl delete volume failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Delete volume succ.");
    return iRet;
}

/*------------------------------------------------------------
Description  : Mobility修改保护卷
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::OMModVolume(mp_string& strUniqueID)
{
    mp_string strParam;
    om_drv_protect_vol_t protectVol;

    COMMLOG(OS_LOG_DEBUG, "Begin modify volume.");
    mp_int32 iRet = GetParamFromTmpFile(strUniqueID, strParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    iRet = ParseProtectVolume(strParam, protectVol);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Parse protect volume params failed, ret %d.", iRet);
        return iRet;
    }
    iRet = SendIOControl(mp_string(MOBILITY_DEVICE_NAME), OM_IM_CTL_MOD_VOLUME, &protectVol, sizeof(protectVol));
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Send ioctl modify volume failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Modify volume succ.");
    return iRet;
}

/*------------------------------------------------------------
Description  : 开始Mobility resync
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::OMStartResync(mp_string& strUniqueID)
{
    mp_string strParam;
    om_drv_protect_strategy_t protectStrategy;

    COMMLOG(OS_LOG_DEBUG, "Begin start resync.");
    mp_int32 iRet = GetParamFromTmpFile(strUniqueID, strParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, "Get start resync params %s.", strParam.c_str());

    iRet = ParseProtectStrategy(strParam, protectStrategy);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Parse protect strategy params failed, iRet %d.", iRet);
        return iRet;
    }
    iRet = SendIOControl(mp_string(MOBILITY_DEVICE_NAME), OM_IOCTL_VERIFY, &protectStrategy, sizeof(protectStrategy));
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Send ioctl start resync failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Start resync succ.");
    return iRet;
}

mp_uchar SystemCallOM::ComputeBitmapGranularity(mp_uint32 protectSize, mp_uint32 memThreshold)
{
    const mp_uint32 bitsPerByte = 8;
    mp_uchar bitmapGranularity;
    mp_int32 leftShiftGB = 30;
    mp_int32 leftShiftMB = 20;
    mp_uint64 protectSizeBytes = (1ULL << leftShiftGB) * protectSize;
    mp_uint64 protectSizeSector = memThreshold * (1ULL << leftShiftMB) * IM_SECTOR_SIZE * bitsPerByte;

    if (memThreshold == 0 || protectSize == 0) {
        bitmapGranularity = IM_BITMAP_GRANULARITY;
        COMMLOG(OS_LOG_INFO, "bitmap granularity is set default vaule %d.", IM_BITMAP_GRANULARITY);
        return bitmapGranularity;
    }

    if (protectSizeBytes % protectSizeSector == 0) {
        bitmapGranularity = (uint8_t)MakeLogUpperBound(protectSizeBytes / protectSizeSector) - 1;
    } else {
        bitmapGranularity = (uint8_t)MakeLogUpperBound(protectSizeBytes / protectSizeSector);
    }

    if (bitmapGranularity < IM_BITMAP_GRANULARITY) {
        COMMLOG(OS_LOG_INFO, "Given memThreshold is enough big, use the minimal granularity.");
        bitmapGranularity = IM_BITMAP_GRANULARITY;
    } else if (bitmapGranularity > IM_MAX_BITMAP_GRANULARITY) {
        COMMLOG(OS_LOG_INFO,
            "Given memThreshold is too small, "
            "use the max granularity, driver may use more memory than memThreshold.");
        bitmapGranularity = IM_MAX_BITMAP_GRANULARITY;
    }

    COMMLOG(OS_LOG_INFO, "Final bitmap granularity is %d.", bitmapGranularity);
    return bitmapGranularity;
}

/*------------------------------------------------------------
Description  : 配置/etc/huawei/im.conf的内容，如果该文件已经存在，说明是configure接口触发，
               不修改bitmap_granularity和reboot_pause_state内容，不存在的话需要修改
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : z00455045
Modification : 2019/10/22
-------------------------------------------------------------*/
mp_int32 SystemCallOM::SetConfigureFile(
    struct im_config_pg config[], om_db_protect_info_t& dbProtectInfo, mp_bool fileExist, mp_int32 fd)
{
    mp_char szErr[ERR_INFO_SIZE] = {0};

    if (fileExist) {
        if (read(fd, config, sizeof(struct im_config_pg)) != sizeof(struct im_config_pg)) {
            mp_int32 iErr = GetOSError();
            COMMLOG(OS_LOG_ERROR,
                "read driver config file failed, error:%d",
                iErr,
                GetOSStrErr(iErr, szErr, sizeof(szErr)));
            return MP_FAILED;
        }

        if (lseek(fd, 0, SEEK_SET) < 0) {
            COMMLOG(OS_LOG_WARN, "lseek driver configure file failed.");
            return MP_FAILED;
        }
    }

    if (!fileExist) {
        config->bitmap_granularity = ComputeBitmapGranularity(
            dbProtectInfo.protectStrategy.uiProtectSize, dbProtectInfo.protectStrategy.uiMemThreshold);
        // reboot_pause_state = 1, driver will enter pasue state
        config->reboot_pause_state = (dbProtectInfo.protectStrategy.sendNow == 1 ? 0 : 1);
    }

    config->type = 0;

    mp_int32 iRet = CUuidNum::CovertStandrdStrToUuid(
        dbProtectInfo.protectStrategy.strVMId, *(reinterpret_cast<mp_uuid*>(config->host_id)));
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Convert string to hostid uuid failed, iRet %d.", iRet);
        return iRet;
    }

    iRet = CUuidNum::CovertStandrdStrToUuid(
        dbProtectInfo.protectStrategy.strOMAId, *(reinterpret_cast<mp_uuid*>(config->oma_id)));
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Convert string to hostid uuid failed, iRet %d.", iRet);
        return iRet;
    }

    config->vrg_ip = inet_addr(dbProtectInfo.protectStrategy.strOMAIp.c_str());
    config->vrg_port = dbProtectInfo.protectStrategy.uiOMAPort;
    config->exp_rpo = dbProtectInfo.protectStrategy.uiRPO;

    return MP_SUCCESS;
}

// CreateDir only supports the creation of directory in the existing directory.
#define CHECK_AND_CREATE_FILE(strFilePath)                                                                             \
    {                                                                                                                  \
        if (!CMpFile::DirExist(strFilePath.c_str())) {                                                                 \
            if (CMpFile::CreateDir(strFilePath.c_str()) != MP_SUCCESS) {                                               \
                COMMLOG(OS_LOG_ERROR, "Failed to create directory %s.", strFilePath.c_str());                          \
                return MP_FAILED;                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
    }

mp_int32 SystemCallOM::WriteConfigFile(
    om_db_protect_info_t& dbProtectInfo, const ProtectVol pPartition[], mp_int32 iVolNum)
{
    mp_int32 iRet = MP_FAILED;
    mp_int32 iErr;
    mp_char szErr[ERR_INFO_SIZE] = {0};
    mp_bool driverConfigExisted = (CMpFile::FileExist(LINUX_DRIVER_CONF) == MP_FALSE ? MP_FALSE : MP_TRUE);
    mp_int32 size = sizeof(struct im_config_pg) + sizeof(ProtectVol) * iVolNum;
    // 使用同步IO强制写入文件，防止强制重启主机后，缓存中断的数据未写入磁盘，但是O_SYNC的性能会很差，此处对性能影响不大
    mp_int32 fd = open(LINUX_DRIVER_CONF.c_str(), O_RDWR | O_CREAT | O_SYNC);  // not use O_TRUNC
    if (fd < 0) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "Open driver config filed failed, errno[%d]:%s.",
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    struct im_config_pg* config = static_cast<struct im_config_pg*>(malloc(size));
    if (config == NULL) {
        COMMLOG(OS_LOG_ERROR, "malloc im_config_pg failed.");
        close(fd);
        return MP_FAILED;
    }

    memset_s(config, size, 0, size);
    // must execute before other below config set
    if (SetConfigureFile(config, dbProtectInfo, driverConfigExisted, fd) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Set driver configure file failed.");
        goto error_handle;
    }
    memcpy_s(&config[1], size - sizeof(struct im_config_pg), pPartition, sizeof(ProtectVol) * iVolNum);
    config->vol_num = static_cast<mp_uchar>(iVolNum);

    if (write(fd, config, size) != size) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "Write driver config filed failed, errno[%d]:%s.",
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        goto error_handle;
    }

    // set permission root 660, continue if error
    if (fchmod(fd, S_IRUSR | S_IWUSR) != 0) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "set driver config filed permission failed, errno[%d]:%s.",
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
    }

    iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_DEBUG, "Write configure file succ.");
error_handle:
    close(fd);
    free(config);
    return iRet;
}
/*------------------------------------------------------------
Description  : 创建linux driver的配置文件信息
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : z00455045
Modification : clean code 和pause持久化、bitmap粒度需要进行了拆分修改
-------------------------------------------------------------*/
mp_int32 SystemCallOM::CreateConfigFile(
    om_db_protect_info_t& dbProtectInfo, const ProtectVol pPartition[], mp_int32 iVolNum)
{
    if (!pPartition) {
        COMMLOG(OS_LOG_ERROR, "pPartition is NULL.");
        return MP_FAILED;
    }

    mp_string strFilePath;

    if (CMpFile::GetPathFromFilePath(LINUX_DRIVER_CONF, strFilePath) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get path from file full path.");
        return MP_FAILED;
    }

    CHECK_AND_CREATE_FILE(strFilePath);
    if (WriteConfigFile(dbProtectInfo, pPartition, iVolNum) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to write configure file.");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "Configure driver config filed succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 根据磁盘信息生成配置文件数据格式
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification :
-------------------------------------------------------------*/
ProtectVol* SystemCallOM::GenerateProtectVols(vector<om_db_disk_info_t>& disksInfo)
{
    mp_size volNum = disksInfo.size();
    ProtectVol* pVols = static_cast<ProtectVol*>(malloc(sizeof(ProtectVol) * volNum));
    ProtectVol* pTmp = pVols;

    for (mp_size i = 0; i < volNum; ++i) {
        om_db_disk_info_t diskInfo = disksInfo[i];
        mp_int32 iRet = CUuidNum::ConvertStrUUIToArray(diskInfo.strDiskId, pTmp->vol_id, sizeof(pTmp->vol_id));
        if (iRet != MP_SUCCESS) {
            free(pVols);
            COMMLOG(OS_LOG_ERROR,
                "Convert volId uuid string %s to char array failed, iRet %d.",
                diskInfo.strDiskId.c_str(),
                iRet);
            return NULL;
        }

        COMMLOG(OS_LOG_DEBUG, "device name %s.", diskInfo.strDeviceName.c_str());
        pTmp->disk_path[OM_DISK_PATH_LEN - 1] = 0;
        iRet = strncpy_s(pTmp->disk_path,
            sizeof(pTmp->disk_path),
            diskInfo.strDeviceName.c_str(),
            strlen(diskInfo.strDeviceName.c_str()));
        if (iRet != MP_SUCCESS) {
            free(pVols);
            COMMLOG(OS_LOG_ERROR, "strcpy device name failed, device %s.", iRet, diskInfo.strDeviceName.c_str());
            return NULL;
        }

        COMMLOG(OS_LOG_DEBUG, "Set disk path succ, disk path %s.", pTmp->disk_path);
        pTmp->disk_num = static_cast<unsigned int>(atoi(diskInfo.strDiskNum.c_str()));
        pTmp++;
    }

    COMMLOG(OS_LOG_DEBUG, "GenerateProtectVols succ.");
    return pVols;
}

/*------------------------------------------------------------
Description  : 解析保护策略以及磁盘信息
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::ParseProtectConfig(mp_string& strParams, om_db_protect_info_t& dbProtectConf)
{
    Json::Value jv;
    Json::Reader r;
    vector<Json::Value> vecDiskInfo;

    COMMLOG(OS_LOG_DEBUG, "Begin parse protect config info, strParams %s.", strParams.c_str());
    mp_bool bRet = r.parse(strParams, jv);
    if (bRet != MP_TRUE) {
        COMMLOG(OS_LOG_ERROR, "Parse protect config params failed.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    // parse protect strategy
    GET_JSON_STRING(jv, OM_DRV_PARAM_HOST_ID, dbProtectConf.protectStrategy.strVMId);
    GET_JSON_STRING(jv, OM_DRV_PARAM_OMA_ID, dbProtectConf.protectStrategy.strOMAId);
    GET_JSON_STRING(jv, OM_DRV_PARAM_IP, dbProtectConf.protectStrategy.strOMAIp);
    GET_JSON_UINT32(jv, OM_DRV_PARAM_PORT, dbProtectConf.protectStrategy.uiOMAPort);
    GET_JSON_UINT32(jv, OM_DRV_PARAM_RPO, dbProtectConf.protectStrategy.uiRPO);
    GET_JSON_UINT32(jv, OM_DRV_PARAM_MEM_THRESHOLD, dbProtectConf.protectStrategy.uiMemThreshold);
    GET_JSON_UINT32(jv, OM_DRV_PARAM_PROTECT_SIZE, dbProtectConf.protectStrategy.uiProtectSize);
    GET_JSON_UINT32(jv, OM_DRV_PARAM_SEND_NOW, dbProtectConf.protectStrategy.sendNow);

    // parse protect disk
    GET_JSON_ARRAY_JSON(jv, OM_DRV_PARAM_PRO_DISKLIST, vecDiskInfo);
    for (vector<Json::Value>::iterator it = vecDiskInfo.begin(); it != vecDiskInfo.end(); it++) {
        om_db_disk_info_t protectDisk;
        GET_JSON_STRING((*it), OM_DRV_PARAM_VOL_ID, protectDisk.strDiskId);
        GET_JSON_STRING((*it), OM_DRV_PARAM_DISK_PATH, protectDisk.strDeviceName);
        GET_JSON_STRING((*it), OM_DRV_PARAM_DISK_NUM, protectDisk.strDiskNum);

        dbProtectConf.hwInfo.disksInfo.push_back(protectDisk);
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : initial driver config file
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::OMInitDriverConfig(mp_string& strUniqueID)
{
    mp_string strParam;
    om_db_protect_info_t dbProtect;

    COMMLOG(OS_LOG_INFO, "Begin initial driver config.");
    mp_int32 iRet = GetParamFromTmpFile(strUniqueID, strParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Initial driver config failed, ret %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, "Get protect params %s.", strParam.c_str());

    // parse protect config from parameter
    iRet = ParseProtectConfig(strParam, dbProtect);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Parse protect strategy params failed, iRet %d.", iRet);
        return iRet;
    }

    // generate protect volumes
    ProtectVol* pVols = GenerateProtectVols(dbProtect.hwInfo.disksInfo);
    if (pVols == NULL) {
        COMMLOG(OS_LOG_ERROR, "GenerateProtectVols failed, iRet %d.", iRet);
        return iRet;
    }

    // create config file for drier, driver can protect disk when os reboot
    iRet = CreateConfigFile(dbProtect, pVols, dbProtect.hwInfo.disksInfo.size());
    if (iRet != MP_SUCCESS) {
        free(pVols);
        COMMLOG(OS_LOG_ERROR, "CreateConfigFile failed, iRet %d.", iRet);
        return iRet;
    }

    free(pVols);
    COMMLOG(OS_LOG_INFO, "Initial driver config Succ.");
    return MP_SUCCESS;
}

mp_int32 SystemCallOM::OMGetKernelAlarm(mp_string& strUniqueID)
{
    LOGGUARD("");
    GetAlarm info;
    mp_int32 iRet = SendIOControl(
        mp_string(MOBILITY_DEVICE_NAME), OM_IOCTL_GET_KERNEL_ALARM, static_cast<mp_void*>(&info), sizeof(info));
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "send ioctl to get kernel alarm failed, iRet = %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Begin save alarm info into temp file, total %d alarms.", info.return_alarm_num);
    vector<mp_string> vecResult;
    mp_string alarmInfoStr;
    Json::FastWriter w;
    Json::Value jvs;
    jvs[KERNEL_ALARM_NUM] = info.return_alarm_num;

    if (info.return_alarm_num <= 0) {  // kernel no alarm info to return
        alarmInfoStr = w.write(jvs);
        vecResult.push_back(alarmInfoStr);
        return CIPCFile::WriteResult(strUniqueID, vecResult);
    }

    for (int i = 0; i < info.return_alarm_num; i++) {
        ALARM_ITEM item = info.pData[i];
        Json::Value jv;
        jv[g_OMAlarmInfoErrCode] = (unsigned long long)item.error_code;
        jv[g_OMAlarmInfoDesc] = item.info;
        jvs[KERNEL_ALARM_ITEMS].append(jv);
    }
    alarmInfoStr = w.write(jvs);
    alarmInfoStr = alarmInfoStr.substr(0, alarmInfoStr.length() - 1);
    vecResult.push_back(alarmInfoStr);
    return CIPCFile::WriteResult(strUniqueID, vecResult);
}

mp_int32 SystemCallOM::OMGetStatistics(mp_string& strUniqueID)
{
    LOGGUARD("");
    StatisticsInfo info;
    mp_int32 iRet = SendIOControl(
        mp_string(MOBILITY_DEVICE_NAME), OM_IOCTL_GET_STATISTICS, static_cast<mp_void*>(&info), sizeof(StatisticsInfo));
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "send ioctl get statistics failed, iRet = %d.", iRet);
        return iRet;
    }

    vector<mp_string> vecResult;
    mp_string strParam;
    Json::FastWriter w;
    Json::Value jv;

    jv[REMAIN_SYNC_DATA] = (mp_uint64)info.remain_sync_data;
    jv[SYNCED_DATA] = (mp_uint64)info.synced_data;
    jv[SYNCED_DATA_RATE] = (mp_uint32)info.synced_data_rate;
    jv[EXPECTED_TIME] = (mp_uint32)info.expected_time;
    jv[WRITE_IOPS] = (mp_uint32)info.write_iops;
    jv[WRITE_THROUGHOUT] = (mp_uint64)info.write_throughout;
    jv[DATA_SEND_SPEED] = (mp_uint64)info.data_send_speed;
    jv[DRIVER_RPO_TIME] = (mp_uint32)info.driver_rpo_time;
    jv[DRIVER_DATA_STATE] = (mp_int32)info.driver_pair_state.data_state;
    jv[DRIVER_LINK_STATE] = (mp_int32)info.driver_pair_state.link_state;
    jv[DRIVER_WORK_MODE] = (mp_int32)info.driver_pair_state.work_mode;
    jv[DRIVER_WORK_STATE] = (mp_int32)info.driver_pair_state.work_state;

    strParam = w.write(jv);
    // remove the last "\n", or the rootcaller return failed
    strParam = strParam.substr(0, strParam.length() - 1);
    vecResult.push_back(strParam);
    return CIPCFile::WriteResult(strUniqueID, vecResult);
}

/*------------------------------------------------------------
Description  : 传递tokenID到driver
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : z00455045
Modification : 2020/3/6
-------------------------------------------------------------*/
mp_int32 SystemCallOM::OMSetKernelTokenId(mp_string& strUniqueID)
{
    SetTokenID tokenInfo;

    COMMLOG(OS_LOG_DEBUG, "Begin to set kernel token id.");

    tokenInfo.isValid = MP_TRUE;

    mp_string strToken;
    mp_int32 iRet = GetParamFromTmpFile(strUniqueID, strToken);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Initial driver config failed, ret %d.", iRet);
        return iRet;
    }

    mp_string outStr;
    DecryptStr(strToken, outStr);
    if (outStr.empty()) {
        COMMLOG(OS_LOG_ERROR, "DecryptStr token failed.");
        return MP_FAILED;
    }

    mp_uuid uuid;
#ifdef LINUX
    iRet = uuid_parse(outStr.c_str(), uuid);  // must not print the token id
    if (iRet != 0) {
        COMMLOG(OS_LOG_ERROR, "Convert string to uuid failed, returned iRet = %d.", iRet);
        return MP_FAILED;
    }
#else
    mp_void(uuid);
    COMMLOG(OS_LOG_ERROR, "do not support uuid_parse function now");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif
    CHECK_NOT_OK(memcpy_s(tokenInfo.token_id, sizeof(tokenInfo.token_id), &uuid, sizeof(uuid)));

    iRet = SendIOControl(mp_string(MOBILITY_DEVICE_NAME), OM_IOCTL_SET_TOKEN_ID, &tokenInfo, sizeof(tokenInfo));
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Send set token id ioctl failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Set kernel token id succ.");
    return iRet;
}

/*------------------------------------------------------------
Description  : 删除Driver的配置信息
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::OMDelDriverConfig(mp_string& strUniqueID)
{
    (mp_void) strUniqueID;
    return CMpFile::DelFile(LINUX_DRIVER_CONF);
}
mp_int32 SystemCallOM::LoadBitmapAndSetInvaild(mp_int32 fd, mp_char headerDescBuff[], mp_int32 headerSize)
{
    mp_char szErr[ERR_INFO_SIZE] = {0};
    mp_int32 iErr;

    if (read(fd, headerDescBuff, headerSize) != headerSize) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "read %s failed, err[%d]:%s",
            BOOT_SAVED_BITMAP.c_str(),
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    PPERS_DATA_HEADER pBitmapHeader = reinterpret_cast<PPERS_DATA_HEADER>(headerDescBuff);
    if (pBitmapHeader->start_safe_flag == 0) {
        COMMLOG(OS_LOG_ERROR, "Bitmap is already invalid.");
        return MP_SUCCESS;
    }

    if (strncmp(pBitmapHeader->magic, PER_DATA_MAGIC, sizeof(pBitmapHeader->magic)) != 0) {
        COMMLOG(OS_LOG_ERROR, "bitmap header is not right.");
        return MP_FAILED;
    }
    PPERS_VOL_BITMAP_DESC pBitmapDesc = reinterpret_cast<PPERS_VOL_BITMAP_DESC>(
        headerDescBuff + pBitmapHeader->desc_offset);
    if (LoadBitmap(fd, pBitmapHeader->bitmap_offset, pBitmapDesc) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "load bitmap failed.");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "begin to set safe flag.");
    pBitmapHeader->start_safe_flag = 0;  // 设置bitmap不可以用
    if (lseek(fd, 0, SEEK_SET) < 0) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "lseek saved bitmap failed, err[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    if (write(fd, headerDescBuff, headerSize) != headerSize) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_WARN,
            "modify bitmap safe flag failed, errno[%d]:%s.",
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
/*------------------------------------------------------------
Description  : Agent启动时重新加载证书，并传给driver使用
Input        :
Output       :
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::OMRetrieveBitMap(mp_string& strUniqueID)
{
    (mp_void) strUniqueID;
    mp_int32 headerSize = PER_HEADER_SECTION_SIZE + PER_DESCRIPTOR_SECTION_SIZE;
    mp_char szErr[ERR_INFO_SIZE] = {0};

    COMMLOG(OS_LOG_INFO, "begin to retrieve bitmap.");
    CreateLockFile();

    if (CMpFile::FileExist(LINUX_DRIVER_CONF) == MP_FALSE) {
        COMMLOG(OS_LOG_WARN, "im control file %s not exist, no need retrieve bitmap", LINUX_DRIVER_CONF.c_str());
        return MP_SUCCESS;
    }

    mp_int32 ret = LoadConfigFile();
    if (ret == BITMAP_ALREADY_SET) {
        COMMLOG(OS_LOG_INFO, "Bitmap is alread set.");
        return MP_SUCCESS;
    }

    if (ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Load config file failed.");
        return MP_FAILED;
    }

    if (CMpFile::FileExist(BOOT_SAVED_BITMAP) == MP_FALSE) {  // 首次安装时bitmap是不存在的
        COMMLOG(OS_LOG_WARN, "Bitmap is not exists.");
        return MP_SUCCESS;
    }

    mp_int32 fd = open(BOOT_SAVED_BITMAP.c_str(), O_RDWR);
    if (fd < 0) {
        mp_int32 iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "open bitmap file failed, err[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    ret = MP_FAILED;
    mp_char* headerDescBuff = static_cast<mp_char*>(malloc(headerSize));
    if (headerDescBuff == NULL) {
        COMMLOG(OS_LOG_ERROR, "malloc headerDescBuff failed.");
        goto end;
    }

    memset_s(headerDescBuff, headerSize, 0, headerSize);
    if (LoadBitmapAndSetInvaild(fd, headerDescBuff, headerSize) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Load bitmap to driver or set bitmap invalid failed");
        goto end;
    }

    ret = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, "Retrievebitmap succ.");
end:
    close(fd);
    if (headerDescBuff) {
        free(headerDescBuff);
    }
    return ret;
}
mp_int32 SystemCallOM::DoIoctl(mp_uint32 cmd, void* data)
{
#ifdef LINUX
    mp_int32 fd = open(MOBILITY_DEVICE_NAME.c_str(), O_RDWR);
    if (fd < 0) {
        COMMLOG(OS_LOG_ERROR, "open driver im_ctldev failed");
        return MP_FAILED;
    }
    if (ioctl(fd, cmd, data) < 0) {
        close(fd);
        mp_int32 iErr = GetOSError();
        if (iErr == BITMAP_ALREADY_SET) {
            COMMLOG(OS_LOG_INFO, "Bitmap is already set, error:%d.", iErr);
            return BITMAP_ALREADY_SET;
        } else {
            COMMLOG(OS_LOG_ERROR, "ioctl failed, error:%d.", iErr);
        }
        return MP_FAILED;
    }
    close(fd);
#endif
    return MP_SUCCESS;
}

mp_void SystemCallOM::CreateLockFile()
{
    // 如果redhat需要创建lock文件，否则shutdown服务无法执行
#ifdef REDHAT
    if (MP_FALSE == CMpFile::FileExist(REDHAT_SHUTDOWN_LOCK_FILE)) {
        mp_string strExec = "touch " + mp_string(REDHAT_SHUTDOWN_LOCK_FILE);
        mp_int32 ret = CSystemExec::ExecSystemWithoutEcho(strExec, MP_FALSE);
        if (ret != MP_SUCCESS) {
            COMMLOG(OS_LOG_WARN, "create shutdown lock file failed.");
        }
    }
#endif
}

/*------------------------------------------------------------
Description  : LoadConfigFile when reboot
Input        :
Output       :
Return       : MP_SUCCESS
               MP_FAILED
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::LoadConfigFile()
{
#ifdef LINUX
    mp_uint32 cmd = IM_CTL_SETCONFIG_CONTENT;
    COMMLOG(OS_LOG_INFO, "Begin to load config file.");
    struct im_config_variable boot_config;

    // 加载后文件不存在，需要返回成功
    if (CMpFile::FileExist(LINUX_DRIVER_BOOT_CONF) == MP_FALSE) {
        COMMLOG(OS_LOG_WARN, "boot config file is not exists.");
        return MP_SUCCESS;
    }

    // 加载配置文件
    mp_int32 fd = open(LINUX_DRIVER_BOOT_CONF.c_str(), O_RDWR);
    if (fd < 0) {
        mp_int32 iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "open boot config file failed, error:%d.", iErr);
        return MP_FAILED;
    }
    if (read(fd, &boot_config, sizeof(struct im_config_variable)) != sizeof(struct im_config_variable)) {
        mp_int32 iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "read boot config file failed, error:%d", iErr);
        close(fd);
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO,
        "flushtime %llu, flushtime_done %llu.",
        boot_config.cbt_flush_times,
        boot_config.cbt_flush_times_done);

    // 如果已经设置bitmap，那么前面就会返回，不会到这个地方失败
    mp_int32 iRet = DoIoctl(cmd, &boot_config);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "do_ioctl IM_CTL_SETCONFIG_CONTENT failed");
        close(fd);
        return iRet;
    }

    close(fd);

    // 等本次加载完成后需要删除配置文件，防止下次异常重启后会继续使用这个文件
    if (CMpFile::DelFile(LINUX_DRIVER_BOOT_CONF)) {
        COMMLOG(OS_LOG_ERROR, "delete confile file failed.");
    }

    COMMLOG(OS_LOG_INFO, "Load config file succ.");
#endif
    return MP_SUCCESS;
}
mp_int32 SystemCallOM::GetBitmapInfo(
    struct im_ctl_bitmap bitmapInfo[], const PPER_VOL_BITMAP_INFO pVolInfo, mp_int32 fd, uint64_t bitmapOffset)
{
    memset_s(bitmapInfo, sizeof(struct im_ctl_bitmap), 0, sizeof(struct im_ctl_bitmap));
    // get dev name
    strncpy_s(bitmapInfo->vol_path,
        sizeof(bitmapInfo->vol_path) - 1,
        pVolInfo->vol_info.disk_name,
        sizeof(pVolInfo->vol_info.disk_name));
    memcpy_s(
        bitmapInfo->vol_id, sizeof(bitmapInfo->vol_id), pVolInfo->vol_info.vol_id, sizeof(pVolInfo->vol_info.vol_id));
    mp_int32 pos = pVolInfo->logical_offset + bitmapOffset;
    if (lseek(fd, pos, SEEK_SET) < 0) {
        COMMLOG(OS_LOG_ERROR, "lseek saved bitmap failed.");
        return MP_FAILED;
    }

    // get bitmap length
    mp_uint32 size = pVolInfo->bitmap_size;
    if (size == 0) {
        COMMLOG(OS_LOG_ERROR, "bitmap header format failed.");
        return MP_FAILED;
    }

    bitmapInfo->bitmap_size = static_cast<uint64_t>(size);  // unit is byte !!!
    bitmapInfo->data = static_cast<uint64_t*>(malloc(size));
    bitmapInfo->bitmap_count = pVolInfo->bitmap_count;
    if (NULL == bitmapInfo->data) {
        COMMLOG(OS_LOG_ERROR, "malloc failed.");
        return MP_FAILED;
    }

    if (read(fd, bitmapInfo->data, size) < 0) {
        COMMLOG(OS_LOG_ERROR, "read saved bitmap failed.");
        free(bitmapInfo->data);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
/*------------------------------------------------------------
Description  : read saved_bitmap to driver when boot, one disk / volume invoke once DoIoctl。
Input        :
Output       :
Return       : MP_SUCCESS
               MP_FAILED
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 SystemCallOM::LoadBitmap(mp_int32 fd, uint64_t bitmapOffset, PPERS_VOL_BITMAP_DESC pBitmapDesc)
{
#ifdef LINUX
    mp_uint32 cmd = IM_CTL_SETBITMAP;
    COMMLOG(OS_LOG_INFO, "Begin to load bitmap.");

    struct im_ctl_bitmap bitmapInfo;
    COMMLOG(OS_LOG_INFO, "disk count = %u.", pBitmapDesc->disk_count);
    for (uint32_t i = 0; i < pBitmapDesc->disk_count; i++) {
        PPER_VOL_BITMAP_INFO pVolInfo = &(pBitmapDesc->bitmap_info[i]);

        if (GetBitmapInfo(&bitmapInfo, pVolInfo, fd, bitmapOffset) != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Assemble bitmap info failed.");
            return MP_FAILED;
        }

        if (bitmapInfo.data == NULL) {
            COMMLOG(OS_LOG_ERROR, "bitmap data area is null");
            return MP_FAILED;
        }

        mp_int32 iRet = DoIoctl(cmd, &bitmapInfo);
        if (iRet == BITMAP_ALREADY_SET) {
            COMMLOG(OS_LOG_INFO, "Bitmap is alread set.");
            free(bitmapInfo.data);
            return MP_SUCCESS;
        }

        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "do_ioctl IM_CTL_BACKUP_SETBITMAP failed.");
            free(bitmapInfo.data);
            return MP_FAILED;
        }
        free(bitmapInfo.data);
    }

    memset_s(&bitmapInfo, sizeof(bitmapInfo), 0, sizeof(bitmapInfo));
    // 如果已经设置bitmap，那么前面就会返回，不会到这个地方失败
    if (DoIoctl(cmd, &bitmapInfo) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "do_ioctl IM_CTL_BACKUP_SETBITMAP failed.");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "Load bitmap succ.");
#endif
    return MP_SUCCESS;
}

mp_int32 SystemCallOM::StartDataProcess(mp_string& strUniqueID)
{
    COMMLOG(OS_LOG_DEBUG, "Begin to start path process.");

    mp_string strParam;
    if (MP_SUCCESS != GetParamFromTmpFile(strUniqueID, strParam)) {
        COMMLOG(OS_LOG_ERROR, "Call GetParamFormTmpFile failed!");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "The parameter passed is: '%s'.", strParam.c_str());

    vector<mp_string> vecCmds;
    CMpString::StrSplit(vecCmds, strParam, ';');
    if (VMWARE_DATAPROCESS_PARAM_NUM != vecCmds.size()) {
        COMMLOG(
            OS_LOG_ERROR, "Both service type and vddk version must be provided, please check the parameter passed!");
        return MP_FAILED;
    }

    mp_string strDpName = CPath::GetInstance().GetBinPath() + PATH_SEPARATOR + OM_DPP_EXEC_NAME + " " + vecCmds[0];
    // if process is running, return failed, since the agent daemon need to check it first.
    mp_string strCmd = "ps -aef | grep '" + strDpName + "' | grep -v grep | grep -v gdb | grep -v vi | grep -v tail";
    CHECK_FAIL_EX(CheckCmdDelimiter(strDpName));
    if (MP_SUCCESS == CSystemExec::ExecSystemWithoutEcho(strCmd)) {
        COMMLOG(OS_LOG_ERROR, "Data process service '%s' is already running.", OM_DPP_EXEC_NAME.c_str());
        return MP_FALSE;
    }

    mp_string strVddkPath;
    strVddkPath = "LD_LIBRARY_PATH=" + CPath::GetInstance().GetLibPath() + PATH_SEPARATOR + "vddk" + PATH_SEPARATOR +
                  vecCmds[1] + PATH_SEPARATOR + "vmware-vix-disklib-distrib/lib64/" + ":$LD_LIBRARY_PATH";

    strDpName = strVddkPath + " nohup " + strDpName + " 1>" + CPath::GetInstance().GetSlogPath() + PATH_SEPARATOR +
                ROOT_EXEC_LOG_NAME + " 2>&1 &";
    FILE* pStream = popen(strDpName.c_str(), "r");
    if (NULL == pStream) {
        mp_int32 iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "Run service '%s' failed, ret: '%d'.", OM_DPP_EXEC_NAME.c_str(), iErr);
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: GetParamFormTmpFile
Description  : 根据id获取临时文件内容
Others       :-------------------------------------------------------- */
mp_int32 SystemCallOM::GetParamFromTmpFile(mp_string& strUniqueID, mp_string& strParam)
{
    // 构造临时输入文件路径
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(mp_string(INPUT_TMP_FILE) + strUniqueID);
    if (CMpFile::FileExist(strFilePath)) {
        mp_int32 iRet = CIPCFile::ReadInput(strUniqueID, strParam);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "ReadInput failed, ret %d.", iRet);
            return iRet;
        }
    }
    // 临时文件不存在按参数为空处理
    return MP_SUCCESS;
}
