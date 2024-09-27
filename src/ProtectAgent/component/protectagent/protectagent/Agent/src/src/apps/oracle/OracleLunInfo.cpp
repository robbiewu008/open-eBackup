/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file OracleLunInfo.cpp
 * @brief  Contains function declarations oracle lun info
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "apps/oracle/OracleLunInfo.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "common/MpString.h"
#include "array/disk.h"
#include "array/array.h"
#include "securecom/SecureUtils.h"
using namespace std;
OracleLunInfo::OracleLunInfo()
{}

OracleLunInfo::~OracleLunInfo()
{}

/* ------------------------------------------------------------
Description  : get the storage information of the oracle database
Input        : stDBInfo -- the oracle database information
Output       : vecLUNInfos -- the result to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleLunInfo::GetDBLUNInfo(oracle_db_info_t& stDBInfo, vector<oracle_lun_info_t>& vecLUNInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, "Begin excute oracle lun info script.");

    if (stDBInfo.iGetArchiveLUN != ORACLE_QUERY_ARCHIVE_LOGS) {
        // 获取数据文件、控制文件、日志文件
        iRet = GetLunInfoByStorageType(stDBInfo, vecLUNInfos, mp_string(DBADAPTIVE_PRAMA_MUST));
        TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_QUERY_APP_LUN_FAILED);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR,
                "GetLunInfoByStorageType failed, type(must), instanceName(%s), iRet %d.",
                stDBInfo.strInstName.c_str(),
                iRet);
            return iRet;
        }

        // 获取临时文件、spfile文件
        iRet = GetLunInfoByStorageType(stDBInfo, vecLUNInfos, mp_string(DBADAPTIVE_PRAMA_OPTION));
        TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_QUERY_APP_LUN_FAILED);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR,
                "GetLunInfoByStorageType failed, type(operation), instanceName(%s), iRet %d.",
                stDBInfo.strInstName.c_str(),
                iRet);
            return iRet;
        }
    } else {
        iRet = GetLunInfoByStorageType(stDBInfo, vecLUNInfos, mp_string(DBADAPTIVE_PRAMA_ARCHIVE));
        TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_QUERY_APP_LUN_FAILED);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR,
                "GetLunInfoByStorageType failed, type(archive), instanceName(%s), iRet %d.",
                stDBInfo.strInstName.c_str(),
                iRet);
            return iRet;
        }
    }

    COMMLOG(OS_LOG_INFO, "Excute oracle lun info script succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : get the storage information of the oracle database by storage type {must|option|archive}
Input        : stDBInfo -- the oracle database information
Output       : vecLUNInfos -- the result to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleLunInfo::GetLunInfoByStorageType(
    oracle_db_info_t stDBInfo, vector<oracle_lun_info_t>& vecLUNInfos, const mp_string& strStorageType)
{
    mp_int32 iRet;
    mp_string strParam;
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_INFO, "Begin excute GetLunInfoByStorageType.");

    stDBInfo.strTableSpaceName = strStorageType;
    BuildLunInfoScriptParam(stDBInfo, strParam);

#ifdef WIN32
    // windows下调用脚本
    iRet = SecureCom::SysExecScript(mp_string(WIN_ORACLE_LUN_INFO), strParam, &vecResult);
    ClearString(strParam);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, "Exec script failed, initial return code is %d, tranformed return code is %d",
            iRet, iNewRet);
        return iNewRet;
    }
#else
    // Oracle下获取数据库LUN信息需要切换到Oracle用户下，必须在root或者实例用户下执行
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYORACLELUNINFO, strParam, &vecResult);
    ClearString(strParam);
#endif
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Excute oracle lun info script failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, "vecResult:%d;", vecResult.size());

    // 分析脚本返回结果
    vector<oracle_storage_script_info> vecDBStorageScriptInfo;
    (mp_void) AnalyseLunInfoScriptRST(vecResult, vecDBStorageScriptInfo);
    COMMLOG(OS_LOG_DEBUG, "vecDBStorageScriptInfo:%d;", vecDBStorageScriptInfo.size());

    // 根据脚本返回结果获取LUN信息
    iRet = AnalyseLunInfoByScriptRST(vecDBStorageScriptInfo, vecLUNInfos, strStorageType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Analyse oracle lun info from script result failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, "vecLUNInfos:%d;", vecLUNInfos.size());

    COMMLOG(OS_LOG_INFO, "Excute GetLunInfoByStorageType succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : build the parameter for the script to be excuted
Input        : stDBInfo -- the oracle database information
Output       : strParam -- the paramter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_void OracleLunInfo::BuildLunInfoScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBNAME) + stDBInfo.strDBName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stDBInfo.strDBPassword + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_TABLESPACENAME) + stDBInfo.strTableSpaceName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMINSTANCE) + stDBInfo.strASMInstance + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMUSERNAME) + stDBInfo.strASMUserName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMPASSWOD) + stDBInfo.strASMPassword + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stDBInfo.strOracleHome;
}

// 分析脚本返回结果
/* ------------------------------------------------------------
Description  : analyse database storage info from the result of script
Input        : vecResult -- the string results returned by the script
Output       : vecDBStorageScriptInfo -- the structure result to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleLunInfo::AnalyseLunInfoScriptRST(
    vector<mp_string>& vecResult, vector<oracle_storage_script_info>& vecDBStorageScriptInfo)
{
    COMMLOG(OS_LOG_INFO, "Begin get oracle storage info.");
    // storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
    for (vector<mp_string>::iterator iter = vecResult.begin(); iter != vecResult.end(); ++iter) {
        oracle_storage_script_info oracle_stor_info;

#ifdef WIN32
        if (AnalyseLunInfoScriptRSTWin(iter, oracle_stor_info) != MP_SUCCESS) {
            continue;
        }
#else
        if (AnalyseLunInfoScriptRSTNoWin(iter, oracle_stor_info) != MP_SUCCESS) {
            continue;
        }
#endif

        vecDBStorageScriptInfo.push_back(oracle_stor_info);
        COMMLOG(OS_LOG_DEBUG,
            "Get oracle storage info structure:storMainType(%s), storSubType(%s), systemDevice(%s), deviceName(%s), "
            "devicePath(%s), vgName(%s), ASMDGName(%s), UDEVResult(%s), UDEVDevice(%s).",
            oracle_stor_info.strStorMainType.c_str(),
            oracle_stor_info.strStorSubType.c_str(),
            oracle_stor_info.strSystemDevice.c_str(),
            oracle_stor_info.strDeviceName.c_str(),
            oracle_stor_info.strDevicePath.c_str(),
            oracle_stor_info.strVgName.c_str(),
            oracle_stor_info.strASMDiskGroup.c_str(),
            oracle_stor_info.strUDEVRes.c_str(),
            oracle_stor_info.strUDEVName.c_str());
    }

    COMMLOG(OS_LOG_INFO, "Get oracle storage info succ.");
    return MP_SUCCESS;
}

// 通过脚本返回LUN返回信息
/* ------------------------------------------------------------
Description  : analyse lun info with result of script
Input        : vecDBStorageScriptInfo -- the script result
                strStorageType -- the storage type {must|option|archive}
Output       : vecLUNInfos -- the lun information list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleLunInfo::AnalyseLunInfoByScriptRST(vector<oracle_storage_script_info>& vecDBStorageScriptInfo,
    vector<oracle_lun_info_t>& vecLUNInfos, const mp_string& strStorageType)
{
    COMMLOG(OS_LOG_INFO, "Begin get oracle lun info by script storage info.");

#ifdef WIN32
    return AnalyseLunInfoByScriptRSTWIN(vecDBStorageScriptInfo, vecLUNInfos, strStorageType);
#else
    return AnalyseLunInfoByScriptRSTNoWIN(vecDBStorageScriptInfo, vecLUNInfos, strStorageType);
#endif
}

#ifdef WIN32
/* ------------------------------------------------------------
Description  : analyse lun info with result of script in windows
Input        : vecDBStorageScriptInfo -- the script result
                strStorageType -- the storage type {must|option|archive}
Output       : vecLUNInfos -- the lun information list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleLunInfo::AnalyseLunInfoByScriptRSTWIN(vector<oracle_storage_script_info>& vecDBStorageScriptInfo,
    vector<oracle_lun_info_t>& vecLUNInfos, mp_string strStorageType)
{
    mp_int32 iRet;

    LOGGUARD("");
    // 获取的磁盘列表
    vector<mp_string> vecDiskPath;
    vector<oracle_lun_info_t> vecDBLUNInfos;
    // CodeDex误报,KLOCWORK.ITER.END.DEREF.MIGHT
    for (vector<oracle_storage_script_info>::iterator iter = vecDBStorageScriptInfo.begin();
         iter != vecDBStorageScriptInfo.end();
         ++iter) {
        if (iter->strStorMainType == STORAGE_TYPE_FS) {
            vecDiskPath.push_back(iter->strDeviceName);
        }
    }

    iRet = GetDBLUNInfoWin(vecDiskPath, vecDBStorageScriptInfo, vecDBLUNInfos);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get windows lun info failed, iRet %d.", iRet);
        return iRet;
    }

    // 判断arraySN和重复LUN信息
    for (vector<oracle_lun_info_t>::iterator lunIter = vecDBLUNInfos.begin(); lunIter != vecDBLUNInfos.end();
         ++lunIter) {
        // 如果是获取数据文件、控制文件、日志文件存储，必须所有的文件都在存储上
        if (lunIter->strArraySn.empty()) {
            if (strStorageType == DBADAPTIVE_PRAMA_MUST) {
                COMMLOG(OS_LOG_ERROR, "Part of DB file is not stored on huawei/huasai array.");
                return ERROR_COMMON_NOT_HUAWEI_LUN;
            } else {
                COMMLOG(OS_LOG_WARN, "ArraySN is empty, but not must info, continue.");
                continue;
            }
        }

        // check luninfo exists.
        if (MP_FALSE == CheckLUNInfoExists(vecLUNInfos, *lunIter)) {
            vecLUNInfos.push_back(*lunIter);
        }
    }

    return MP_SUCCESS;
}

mp_int32 OracleLunInfo::AnalyseLunInfoScriptRSTWin(vector<mp_string>::iterator& iter,
    oracle_storage_script_info& oracle_stor_info)
{
    COMMLOG(OS_LOG_DEBUG, "%s", iter->c_str());

    // find 1st separator(;)
    size_t idxSep = iter->find(NODE_SEMICOLON);
    if (mp_string::npos == idxSep) {
        COMMLOG(OS_LOG_ERROR,
            "Get db storage info failed when find 1nd separator, storage info=%s.",
            (*iter).c_str());
        return MP_FAILED;
    }
    mp_string strDeviceName = iter->substr(0, idxSep);

    // find 2nd separator(;)
    size_t idxSepSec = iter->find(NODE_SEMICOLON, idxSep + 1);
    if (mp_string::npos == idxSepSec) {
        COMMLOG(OS_LOG_ERROR,
            "Get db storage info failed when find 2nd separator, storage info=%s.",
            (*iter).c_str());
        return MP_FAILED;
    }
    mp_string strDevicePath = iter->substr(idxSep + 1, (idxSepSec - idxSep) - 1);

    // find 3rd separator(;)
    size_t idxSepTrd = iter->find(NODE_SEMICOLON, idxSepSec + 1);
    if (mp_string::npos == idxSepTrd) {
        COMMLOG(OS_LOG_ERROR,
            "Get db storage info failed when find 3rd separator, storage info=%s.",
            (*iter).c_str());
        return MP_FAILED;
    }
    mp_string strStorMainType = iter->substr(idxSepSec + 1, (idxSepTrd - idxSepSec) - 1);
    mp_string strASMDiskGroup = iter->substr(idxSepTrd + 1);

    oracle_stor_info.strStorMainType = strStorMainType;
    oracle_stor_info.strDeviceName = strDeviceName;
    oracle_stor_info.strDevicePath = strDevicePath;
    oracle_stor_info.strASMDiskGroup = strASMDiskGroup;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : get lun information by sub area list,  disk list and script result in windows platform
Input        : stSubareaInfoWin -- sub area list
                vecDiskInfoWinRes -- disk list
Output       : stDBLUNInfo -- the script result to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleLunInfo::GetLUNInfoWin(mp_string& strPath, sub_area_Info_t& stSubareaInfoWin,
    vector<disk_info>& vecDiskInfoWinRes, oracle_lun_info_t& stDBLUNInfo)
{
    mp_char acOffset[MAX_PATH_LEN] = {0};
    vector<disk_info>::iterator itDiskInfoWin;
    COMMLOG(OS_LOG_INFO, "Begin get oracle lun info with disk and subarea in windows.");

    for (itDiskInfoWin = vecDiskInfoWinRes.begin(); itDiskInfoWin != vecDiskInfoWinRes.end(); itDiskInfoWin++) {
        if (stSubareaInfoWin.iDiskNum == itDiskInfoWin->iDiskNum) {
            stDBLUNInfo.strArraySn = itDiskInfoWin->strArraySN;
            stDBLUNInfo.strLUNId = itDiskInfoWin->strLUNID;
            stDBLUNInfo.strWWN = itDiskInfoWin->strLUNWWN;
            stDBLUNInfo.strDeviceName = stSubareaInfoWin.acVolName;
            stDBLUNInfo.strDevicePath = strPath;
            CHECK_FAIL(snprintf_s(acOffset, sizeof(acOffset), sizeof(acOffset) - 1, "%lld", stSubareaInfoWin.llOffset));
            mp_string strAcOffset = acOffset;
            stDBLUNInfo.strLBA = strAcOffset;
            break;
        }
    }

    if (itDiskInfoWin == vecDiskInfoWinRes.end()) {
        COMMLOG(OS_LOG_WARN, "Get lun info(%s) with disk and subarea failed.", strPath.c_str());
    } else {
        COMMLOG(OS_LOG_INFO, "Get oracle lun info with disk and subarea in windows.");
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : get lun information by path in windows
Input        : strPath -- the disk label
                vecSubareaInfoWin -- sub area list
                vecDiskInfoWinRes -- disk list
Output       : vecLUNInfos -- the lun information list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleLunInfo::GetLUNInfoByPathWin(mp_string& strPath, vector<sub_area_Info_t>& vecSubareaInfoWin,
    vector<disk_info>& vecDiskInfoWinRes, vector<oracle_lun_info_t>& vecLUNInfos)
{
    vector<sub_area_Info_t>::iterator itSubareaInfoWin;
    COMMLOG(OS_LOG_INFO, "Begin get lun info by path in windows.");

    mp_string iRetUpper = CMpString::ToUpper(strPath);
    for (itSubareaInfoWin = vecSubareaInfoWin.begin(); itSubareaInfoWin != vecSubareaInfoWin.end();
         itSubareaInfoWin++) {
        mp_string tmpstrPath = CMpString::ToUpper(itSubareaInfoWin->acDriveLetter);
        if (iRetUpper == tmpstrPath) {
            oracle_lun_info_t stDBLUNInfo;
            stDBLUNInfo.strArraySn = "";
            stDBLUNInfo.iStorMainType = atoi(STORAGE_TYPE_FS.c_str());
            stDBLUNInfo.iStorSubType = VOLTYPE_NOVOL;
            if (MP_SUCCESS != GetLUNInfoWin(strPath, *itSubareaInfoWin, vecDiskInfoWinRes, stDBLUNInfo)) {
                COMMLOG(OS_LOG_ERROR, "Get lun info failed.");
                return MP_FAILED;
            }

            vecLUNInfos.push_back(stDBLUNInfo);

            break;
        }
    }

    if (itSubareaInfoWin == vecSubareaInfoWin.end()) {
        COMMLOG(OS_LOG_ERROR, "The disk label(%s) is not exist.", strPath.c_str());

        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "Get lun info by path in windows succ.");

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : get filesystem lun information in windows
Input        : vecDiskPath -- the disk label list
                vecSubareaInfoWin -- sub area list
                vecDiskInfoWinRes -- disk list
Output       : vecLUNInfos -- the lun information list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleLunInfo::GetDBLUNFSInfoWin(vector<mp_string>& vecDiskPath, vector<disk_info>& vecDiskInfoWinRes,
    vector<sub_area_Info_t>& vecSubareaInfoWin, vector<oracle_lun_info_t>& vecLUNInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    LOGGUARD("");
    for (vector<mp_string>::iterator itPath = vecDiskPath.begin(); itPath != vecDiskPath.end(); itPath++) {
        iRet = GetLUNInfoByPathWin(*itPath, vecSubareaInfoWin, vecDiskInfoWinRes, vecLUNInfos);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR,
                "Get lun file system info(%s) by path in windows failed.",
                itPath->c_str());
            return iRet;
        }
    }

    return iRet;
}

/* ------------------------------------------------------------
Description  : get asm lun information in windows
Input        : vecAdaptiveLUNInfo -- script result list
                vecSubareaInfoWin -- sub area list
                vecDiskInfoWinRes -- disk list
Output       : vecLUNInfos -- the lun information list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleLunInfo::GetDBLUNASMInfoWin(vector<oracle_storage_script_info>& vecAdaptiveLUNInfo,
    vector<disk_info>& vecDiskInfoWinRes, vector<sub_area_Info_t>& vecSubareaInfoWin,
    vector<oracle_lun_info_t>& vecLUNInfos)
{
    Disk diskManager;
    vector<sub_area_Info_t>::iterator itSubareaInfoWin;
    LOGGUARD("");
    if (MP_TRUE != diskManager.InitSymboLinkRes()) {
        COMMLOG(OS_LOG_ERROR, "init Symbolink failed.");
        return MP_FAILED;
    }

    // 获取ASM标记分区的设备信息
    for (vector<oracle_storage_script_info>::iterator iter = vecAdaptiveLUNInfo.begin();
        iter != vecAdaptiveLUNInfo.end(); ++iter) {
        if (MP_TRUE != diskManager.QuerySymboLinkInfo(iter->strDeviceName, iter->strSystemDevice) ||
            iter->strStorMainType != STORAGE_TYPE_ASMWIN) {
            continue;
        }

        COMMLOG(OS_LOG_DEBUG, "querySymboLinkInfo: [%s]=>[%s]", iter->strDeviceName.c_str(),
            iter->strSystemDevice.c_str());

        itSubareaInfoWin = vecSubareaInfoWin.begin();
        for (; itSubareaInfoWin != vecSubareaInfoWin.end(); ++itSubareaInfoWin) {
            if (mp_string(itSubareaInfoWin->acDeviceName).compare(iter->strSystemDevice) == 0) {
                continue;
            }
            oracle_lun_info_t stDBLUNInfo;
            stDBLUNInfo.strArraySn = "";
            stDBLUNInfo.strASMDiskGroup = iter->strASMDiskGroup;
            stDBLUNInfo.iStorMainType = atoi(STORAGE_TYPE_ASMWIN.c_str());
            stDBLUNInfo.iStorSubType = VOLTYPE_NOVOL;
            mp_int32 iRet = GetLUNInfoWin(iter->strDevicePath, *itSubareaInfoWin, vecDiskInfoWinRes, stDBLUNInfo);
            if (MP_SUCCESS != iRet) {
                COMMLOG(OS_LOG_ERROR, "Get lun info(%s) failed.", iter->strDevicePath.c_str());
                diskManager.FreeSymboLinkRes();
                return MP_FAILED;
            }

            // check luninfo exists.
            iRet = CheckLUNInfoExists(vecLUNInfos, stDBLUNInfo);
            if (MP_FALSE == iRet) {
                vecLUNInfos.push_back(stDBLUNInfo);
            }
            break;
        }

        if (itSubareaInfoWin == vecSubareaInfoWin.end()) {
            COMMLOG(OS_LOG_WARN, "ASM disk([%s]=>[%s]) match volumn deviceName failed.",
                iter->strDeviceName.c_str(), iter->strSystemDevice.c_str());
        }
    }
    diskManager.FreeSymboLinkRes();

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : get lun information in windows
Input        : vecDiskPath -- the disk label list
                vecSubareaInfoWin -- sub area list
                vecDiskInfoWinRes -- disk list
Output       : vecLUNInfos -- the lun information list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleLunInfo::GetDBLUNInfoWin(vector<mp_string>& vecDiskPath,
    vector<oracle_storage_script_info>& vecAdaptiveLUNInfo, vector<oracle_lun_info_t>& vecLUNInfos)
{
    mp_int32 iRet;
    Disk diskManager;

    vector<disk_info> vecDiskInfoWinRes;
    vector<sub_area_Info_t> vecSubareaInfoWin;

    COMMLOG(OS_LOG_INFO, "Begin get oracle lun in windows.");

    // 获取磁盘信息，包括阵列SN,LUN ID,LUN WWN,磁盘编号
    iRet = Disk::GetDiskInfoList(vecDiskInfoWinRes);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "%s", "Get disk infomation on windows failed.");

        return ERROR_DISK_GET_DISK_INFO_FAILED;
    }

    // 获取磁盘序号和盘符的对应信息
    iRet = Disk::GetSubareaInfoList(vecSubareaInfoWin);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "%s", "Get disk subares on windows failed.");

        return ERROR_DISK_GET_PARTITION_INFO_FAILED;
    }

    // 获取数据库文件所在的LUN信息
    iRet = GetDBLUNFSInfoWin(vecDiskPath, vecDiskInfoWinRes, vecSubareaInfoWin, vecLUNInfos);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get lun filesystem info in windows failed (%d).", iRet);
        return iRet;
    }

    iRet = GetDBLUNASMInfoWin(vecAdaptiveLUNInfo, vecDiskInfoWinRes, vecSubareaInfoWin, vecLUNInfos);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get lun ASM info in windows failed (%d).", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Get oracle lun in windows succ.");
    return MP_SUCCESS;
}

#else
/* ------------------------------------------------------------
Description  : analyse lun info with result of script in no windows
Input        : vecDBStorageScriptInfo -- the script result
                strStorageType -- the storage type {must|option|archive}
Output       : vecLUNInfos -- the lun information list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleLunInfo::AnalyseLunInfoByScriptRSTNoWIN(vector<oracle_storage_script_info>& vecDBStorageScriptInfo,
    vector<oracle_lun_info_t>& vecLUNInfos, const mp_string& strStorageType)
{
    mp_int32 iRet;
    mp_string strUDEVConfDir("");
    mp_string strUDEVRoot("");
    mp_string strDev;
    map<mp_string, luninfo_t> mapLuninfo;

    LOGGUARD("");
    // 获取UDEV的配置
#ifdef LINUX
    iRet = GetUDEVConfig(strUDEVConfDir, strUDEVRoot);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get udev config failed, iRet %d.", iRet);
        return iRet;
    }
#endif
    vector<mp_string> vecResult;
    iRet = GetDevList(vecDBStorageScriptInfo, strDev, vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get device list failed, iRet %d.", iRet);
        return iRet;
    }

    iRet = AnalyzeLunList(vecResult, mapLuninfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Analyze device list failed, iRet %d.", iRet);
        return iRet;
    }

    // 分析存储信息
    for (vector<oracle_storage_script_info>::iterator iter = vecDBStorageScriptInfo.begin();
            iter != vecDBStorageScriptInfo.end(); ++iter) {
        iRet = AnalyzeStorageGerInfo(
            iter, strUDEVConfDir, strUDEVRoot, mapLuninfo, vecLUNInfos, strStorageType, strDev);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Analyze storage info failed, iRet %d.", iRet);
            return iRet;
        }
    }

    return MP_SUCCESS;
}

mp_int32 OracleLunInfo::AnalyzeStorageGerInfo(vector<oracle_storage_script_info>::iterator& iter,
    const mp_string& strUDEVConfDir, const mp_string& strUDEVRoot, map<mp_string, luninfo_t>& mapLuninfo,
    vector<oracle_lun_info_t>& vecLUNInfos, const mp_string& strStorageType, mp_string& strDev)
{
    oracle_lun_info_t oracle_lun_info;
    CopyValue(oracle_lun_info, iter);

    mp_int32 iRet = GetLunUDEVInfo(strUDEVConfDir, strUDEVRoot, iter->strUDEVName, iter->strUDEVRes, oracle_lun_info);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get lun udev info failed, iRet %d.", iRet);
        return iRet;
    }

    iRet = GetHPRawDiskName(strDev, iter->strSystemDevice);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get HP Raw disk info failed, iRet %d.", iRet);
        return iRet;
    }

    map<mp_string, luninfo_t>::iterator mapIter = mapLuninfo.find(strDev);
    if (mapIter == mapLuninfo.end()) {
        COMMLOG(OS_LOG_ERROR, "Cannot find lun info of device (%s).", strDev.c_str());
        return ERROR_COMMON_QUERY_APP_LUN_FAILED;
    }

    return AnalyzeStorageOtherInfo(iter, vecLUNInfos, strStorageType, strDev, mapIter, oracle_lun_info);
}

mp_int32 OracleLunInfo::AnalyzeStorageOtherInfo(vector<oracle_storage_script_info>::iterator& iter,
    vector<oracle_lun_info_t>& vecLUNInfos, const mp_string& strStorageType, const mp_string& strDev,
    map<mp_string, luninfo_t>::iterator& mapIter, oracle_lun_info_t& oracle_lun_info)
{
    (mp_void)iter;
    // 获取厂商
    luninfo_t luninfo = mapIter->second;
    mp_string strVendor = luninfo.strVendor;

    if (MP_FALSE == Array::CheckHuaweiLUN(strVendor)) {
        if (strStorageType != DBADAPTIVE_PRAMA_MUST) {
            COMMLOG(OS_LOG_WARN, "Device name(%s-%s) is not in huawei/huasai array.",
                strStorageType.c_str(), strDev.c_str());
            return MP_SUCCESS;
        } else {
            COMMLOG(OS_LOG_ERROR, "Device name(%s-%s) is not in huawei/huasai array.",
                strStorageType.c_str(), strDev.c_str());
            return ERROR_COMMON_NOT_HUAWEI_LUN;
        }
    }

    // 获取LUN对应的ArranSN信息
    mp_string strArraySN = luninfo.strArraySN;
    if (strArraySN.empty()) {
        // 查询must表空间，arraylun为空时返回错误码
        if (strStorageType == DBADAPTIVE_PRAMA_MUST) {
            COMMLOG(OS_LOG_ERROR, "Get and check arraySN info of disk(%s-%s) failed.",
                strStorageType.c_str(), strDev.c_str());
            return ERROR_COMMON_NOT_HUAWEI_LUN;
        }

        // 查询option和archive，不返回arraysn为空的LUN
        return MP_SUCCESS;
    }

    // 获取LUN对应的信息
    mp_string strWWN = luninfo.strLUNWWN;
    mp_string strLUNID = luninfo.strLUNID;
    mp_bool bLuninfo = strWWN.empty() || strLUNID.empty();
    if (MP_TRUE == bLuninfo) {
        // 如果是获取数据文件、控制文件、日志文件存储，必须所有的文件都在存储上
        if (strStorageType == DBADAPTIVE_PRAMA_MUST) {
            COMMLOG(OS_LOG_ERROR, "Get lun(%s) info failed.", strDev.c_str());
            return ERROR_COMMON_QUERY_APP_LUN_FAILED;
        } else { // 如果是可选文件，则继续执行下一个查询
            COMMLOG(OS_LOG_WARN, "Get info of option lun(%s) failed, continue get next lun.",
                strDev.c_str());
            return MP_SUCCESS;
        }
    }

    oracle_lun_info.strLUNId = std::move(strLUNID);
    oracle_lun_info.strArraySn = std::move(strArraySN);
    oracle_lun_info.strWWN = std::move(strWWN);

    // check luninfo exists.
    if (MP_FALSE == CheckLUNInfoExists(vecLUNInfos, oracle_lun_info)) {
        vecLUNInfos.push_back(oracle_lun_info);
    }

    return MP_SUCCESS;
}

mp_int32 OracleLunInfo::GetDevList(
    vector<oracle_storage_script_info>& vecDBStorageScriptInfo, mp_string& strDev, vector<mp_string>& vecResult)
{
    mp_string strDevList;
    mp_int32 iRet;
    // 获取设备列表字符串，通过分号隔开
    for (vector<oracle_storage_script_info>::iterator iter = vecDBStorageScriptInfo.begin();
        iter != vecDBStorageScriptInfo.end(); ++iter) {
        // get device name
#ifdef HP_UX_IA
        iRet = Disk::GetHPRawDiskName(iter->strSystemDevice, strDev);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR,
                "Get raw device info of disk(%s) failed, iRet %d.",
                iter->strSystemDevice.c_str(),
                iRet);
            return ERROR_COMMON_QUERY_APP_LUN_FAILED;
        }
#else
        strDev = iter->strSystemDevice;
#endif

        // check not exists
        mp_string strTmp = strDev + ";";
        if (strDevList.find(strTmp) == mp_string::npos) {
            strDevList = strDevList + strDev + ";";
        }
    }

    // 去掉最后一个分号
    strDevList = strDevList.substr(0, strDevList.length() - 1);

    // 批量获取LUN信息,提升性能
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_BATCH_GETLUN_INFO, strDevList, &vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get lun info (%s) failed.", strDevList.c_str());
        return iRet;
    }
    // codedex误报CHECK_CONTAINER_EMPTY，此处容器已判空
    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "The result of lun info (%s) is null.", strDevList.c_str());
        return MP_FAILED;
    }
    
    return MP_SUCCESS;
}

mp_int32 OracleLunInfo::AnalyzeLunList(vector<mp_string>& vecResult, map<mp_string, luninfo_t>& mapLuninfo)
{
    vector<mp_string> vecAnalyse;
    // 分析结果文件
    for (vector<mp_string>::iterator iter = vecResult.begin(); iter != vecResult.end(); ++iter) {
        luninfo_t luninfo;
        vecAnalyse.clear();
        CMpString::StrSplit(vecAnalyse, *iter, ';');
        if (vecAnalyse.empty()) {
            COMMLOG(OS_LOG_ERROR,
                "Analyse result of ROOT_COMMAND_BATCH_GETLUN_INFO failed, lun info list is empty(%s).", iter->c_str());
            return MP_FAILED;
        }

        if (vecAnalyse.size() != ORALCE_NUM_6) {
            COMMLOG(OS_LOG_ERROR,
                "Analyse result of ROOT_COMMAND_BATCH_GETLUN_INFO failed, lun info list size is wrong(%s).",
                iter->c_str());
            return MP_FAILED;
        }

        // devicename;vendor;product;arraysn;lunid;wwn
        vector<mp_string>::iterator iterAnalyse = vecAnalyse.begin();
        luninfo.strDeviceName = *(iterAnalyse++);
        luninfo.strVendor = *(iterAnalyse++);
        luninfo.strProduct = *(iterAnalyse++);
        luninfo.strArraySN = *(iterAnalyse++);
        luninfo.strLUNID = *(iterAnalyse++);
        luninfo.strLUNWWN = *iterAnalyse;

        mapLuninfo.insert(pair<mp_string, luninfo_t>(luninfo.strDeviceName, luninfo));
    }

    return MP_SUCCESS;
}

mp_int32 OracleLunInfo::GetLunUDEVInfo(const mp_string& strUDEVConfDir, const mp_string& strUDEVRoot,
    mp_string& strUDEVName, mp_string& strUDEVRes, oracle_lun_info_t& oracle_lun_info)
{
#ifdef LINUX
    mp_string strUdevDeviceRecord;
    if (!strUDEVRes.empty()) {
        mp_int32 iRet = GetUDEVInfo(strUDEVConfDir, strUDEVName, strUDEVRes, strUdevDeviceRecord);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get udev info failed, iRet %d.", iRet);
            return iRet;
        }

        if (!strUdevDeviceRecord.empty()) {
            oracle_lun_info.strDeviceName = strUDEVRoot + strUDEVName;
            oracle_lun_info.strUDEVRules = std::move(strUdevDeviceRecord);
        } else {
            COMMLOG(OS_LOG_WARN, "Get (%s) udev info empty, iRet %d.",
                strUDEVName.c_str(), iRet);
        }
    }
#endif
    return MP_SUCCESS;
}

mp_void OracleLunInfo::CopyValue(oracle_lun_info_t& oracle_lun_info,
    vector<oracle_storage_script_info>::iterator& iter)
{
    oracle_lun_info.iStorMainType = atoi(iter->strStorMainType.c_str());
    oracle_lun_info.iStorSubType = atoi(iter->strStorSubType.c_str());
    oracle_lun_info.strDeviceName = iter->strDeviceName;
    oracle_lun_info.strPvName = iter->strSystemDevice;
    oracle_lun_info.strDevicePath = iter->strDevicePath;
    oracle_lun_info.strUDEVRules = "";
    oracle_lun_info.strVgName = iter->strVgName;
    oracle_lun_info.strASMDiskGroup = iter->strASMDiskGroup;
}

mp_int32 OracleLunInfo::GetHPRawDiskName(mp_string& strDev, mp_string& strSystemDevice)
{
#ifdef HP_UX_IA
    mp_int32 iRet = Disk::GetHPRawDiskName(strSystemDevice, strDev);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get raw device info of disk(%s) failed, iRet %d.",
            strSystemDevice.c_str(), iRet);
        return ERROR_COMMON_QUERY_APP_LUN_FAILED;
    }
#else
    strDev = strSystemDevice;
#endif

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : check array sn of device used by oracle database
Input        : strDev -- device name
                strStorageType -- the storage type {must|option|archive}
Output       : strArraySN -- array to be get
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleLunInfo::GetAndCheckArraySN(const mp_string& strDev, mp_string& strArraySN,
    const mp_string& strStorageType)
{
    mp_int32 iRet;
    LOGGUARD("");
    // 获取LUN对应的信息
    iRet = Array::GetArraySN(strDev, strArraySN);
    if (iRet != MP_SUCCESS) {
        // 如果是获取数据文件、控制文件、日志文件存储，必须所有的文件都在存储上
        if (strStorageType == DBADAPTIVE_PRAMA_MUST) {
            COMMLOG(OS_LOG_ERROR, "Get array SN of lun(%s) failed, iRet %d.", strDev.c_str(), iRet);
            return ERROR_COMMON_QUERY_APP_LUN_FAILED;
        } else { // 如果是可选文件，则继续执行下一个查询
            COMMLOG(OS_LOG_WARN, "Get array SN of option lun(%s) failed, continue get next lun.",
                strDev.c_str(), iRet);
            return MP_SUCCESS;
        }
    }

    // 判断阵列SN是否为空
    mp_bool bCheckArraySN = (strArraySN.empty() && (strStorageType == DBADAPTIVE_PRAMA_MUST));
    if (MP_TRUE == bCheckArraySN) {
        COMMLOG(OS_LOG_ERROR, "Part of DB file is not stored on huawei/huasai array");
        return ERROR_COMMON_NOT_HUAWEI_LUN;
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : get vendor and product of device used by oracle database
Input        : strDev -- device name
                strStorageType -- the storage type {must|option|archive}
Output       : strVendor -- array sn to be get
                strProduct -- product to be get
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleLunInfo::GetVendorAndProduct(
    const mp_string& strDev, mp_string& strVendor, mp_string& strProduct, mp_string strStorageType)
{
    LOGGUARD("");
    (mp_void)strStorageType;
    // 阵列的厂商和型号
    mp_int32 iRet = Array::GetArrayVendorAndProduct(strDev, strVendor, strProduct);
    if (iRet == MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "Get vendor and product info failed.");
        return iRet;
    }

    mp_string pStrTmp = CMpString::Trim(strVendor);
    if (pStrTmp.empty()) {
        COMMLOG(OS_LOG_ERROR, "Get vendor info of Device name(%s) failed.", strDev.c_str());
        return ERROR_COMMON_NOT_HUAWEI_LUN;
    }
    strVendor = pStrTmp;

    // product内容查询出来后并没有使用，如果为空只做提示，不做退出
    pStrTmp = CMpString::Trim(strProduct);
    if (pStrTmp.empty()) {
        COMMLOG(OS_LOG_WARN, "Get product info of Device name(%s) failed.", strDev.c_str());
        strProduct = "";
    } else {
        strProduct = std::move(pStrTmp);
    }

    // 排除掉非华为的产品
    mp_bool bFlag = strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI.c_str()) != 0 &&
                    strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI.c_str()) != 0 &&
                    strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY.c_str()) != 0 &&
                    strcmp(strVendor.c_str(), ARRAY_VENDER_FUSION_STORAGE.c_str()) != 0;
    if (bFlag) {
        COMMLOG(OS_LOG_ERROR, "Device name(%s) is not in huawei/huasai array.", strDev.c_str());
        return ERROR_COMMON_NOT_HUAWEI_LUN;
    }
    return MP_SUCCESS;
}

mp_int32 OracleLunInfo::FillLunInfo(mp_string lunArray[], std::size_t iArrayLen, mp_string source, const mp_char cSep)
{
    vector<string> result;
    CMpString::StrSplit(result, source, cSep);
    if (result.size() < iArrayLen) {
        return MP_FAILED;
    }
    for (std::size_t i = 0; i < iArrayLen; i++) {
        lunArray[i] = result[i];
    }
    return MP_SUCCESS;
}

mp_int32 OracleLunInfo::AnalyseLunInfoScriptRSTNoWin(vector<mp_string>::iterator& iter,
    oracle_storage_script_info& oracle_stor_info)
{
    COMMLOG(OS_LOG_DEBUG, "%s", iter->c_str());
    string lunInfoArray[] = {
        oracle_stor_info.strStorMainType, oracle_stor_info.strStorSubType,
        oracle_stor_info.strSystemDevice, oracle_stor_info.strDeviceName,
        oracle_stor_info.strDevicePath, oracle_stor_info.strVgName,
        oracle_stor_info.strASMDiskGroup, oracle_stor_info.strUDEVRes,
        oracle_stor_info.strUDEVName
    };

    std::size_t len = sizeof(lunInfoArray) / sizeof(lunInfoArray[0]);
    mp_string splitOperator = NODE_SEMICOLON;
    mp_int32 iRet = FillLunInfo(lunInfoArray, len, *iter, *splitOperator.c_str());
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Get db storage info failed when find separator, storage info=%s.", (*iter).c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

#endif

#ifdef LINUX
/* ------------------------------------------------------------
Description  : get udev config
Output       :  strUDEVConfDir -- the udev config to be returned
                strUDEVRoot -- the udev root directory to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleLunInfo::GetUDEVConfig(mp_string& strUDEVConfDir, mp_string& strUDEVRoot)
{
    mp_int32 ret = MP_FAILED;
    ERRLOG("Function is forbidden");
    return ret;
}

/* ------------------------------------------------------------
Description  : get udev information
Input        : strUdevRulesFileDir -- the directory of udev directory
                strUdevName -- the name of udev device
                strUdevResult -- the udev result(WWN) of udev device
Output       : strUdevDeviceRecord -- the udev config string to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleLunInfo::GetUDEVInfo(
    mp_string strUdevRulesFileDir, mp_string strUdevName, mp_string strUdevResult, mp_string& strUdevDeviceRecord)
{
    mp_int32 ret = MP_FAILED;
    ERRLOG("Function is forbidden");
    return ret;
}

#endif

/* ------------------------------------------------------------
Description  : check whether to repeat the lun information
Input        : vecLUNInfos -- the string results returned by the script
Output       :  oracle_lun_info -- the structure result to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_bool OracleLunInfo::CheckLUNInfoExists(vector<oracle_lun_info_t>& vecLUNInfos, oracle_lun_info_t& oracle_lun_info)
{
    vector<oracle_lun_info_t>::iterator lunIter;
    for (lunIter = vecLUNInfos.begin(); lunIter != vecLUNInfos.end(); ++lunIter) {
        if (lunIter->strWWN == oracle_lun_info.strWWN && lunIter->strDeviceName == oracle_lun_info.strDeviceName) {
            break;
        }
    }

    return (lunIter == vecLUNInfos.end()) ? MP_FALSE : MP_TRUE;
}
