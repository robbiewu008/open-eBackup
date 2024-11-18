/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "apps/app/App.h"
#include "securecom/RootCaller.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/MpString.h"
#include "common/ConfigXmlParse.h"
using namespace std;
static mp_time g_lastFreezeTime = -1;

App::App()
{
    m_freezeState = DB_UNKNOWN;

    (mp_void)CMpThread::InitLock(&m_freezeStateLock);
}

App::~App()
{
    (mp_void)CMpThread::DestroyLock(&m_freezeStateLock);
}

EXTER_ATTACK mp_int32 App::CheckFreezeIntervalTime(
    const mp_time& tFreezeTime, vector<app_failed_info_t>& vecAppFailedList)
{
    COMMLOG(OS_LOG_INFO, "Check interval time of freeze.");

    mp_int32 freezeIntervalTime = 0;
    mp_int32 cfgIntervalTime = 0;
    app_failed_info_t errorinfo;

    if (-1 != g_lastFreezeTime) {
        freezeIntervalTime = ((tFreezeTime > g_lastFreezeTime) ? (tFreezeTime - g_lastFreezeTime)
                                                                : (g_lastFreezeTime - tFreezeTime));
    } else {
        freezeIntervalTime = tFreezeTime;
    }

    mp_int32 iRet =
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_FREEZEINTERVALTIME, cfgIntervalTime);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Read cfg-freeze_interval_time failed, iRet %d.", iRet);
        errorinfo.iErrorCode = ERROR_COMMON_READ_CONFIG_FAILED;
        errorinfo.strDbName = "";
        vecAppFailedList.push_back(errorinfo);
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    if (cfgIntervalTime < FREEZE_MIX_INTERVAL || cfgIntervalTime > FREEZE_MAX_INTERVAL) {
        cfgIntervalTime = FREEZE_DEFAULT_INTERVAL;
        COMMLOG(OS_LOG_DEBUG,
            "Freeze intervalue(%d) is invalid, set defaule %d.",
            cfgIntervalTime,
            FREEZE_DEFAULT_INTERVAL);
    }
    COMMLOG(OS_LOG_DEBUG,
        "Freeze app time.tFreezeTime=%d,g_lastFreezeTime=%d,cfgIntervalTime=%d",
        tFreezeTime,
        g_lastFreezeTime,
        cfgIntervalTime);
    if (freezeIntervalTime >= cfgIntervalTime) {
        g_lastFreezeTime = tFreezeTime;
    } else {
        COMMLOG(OS_LOG_ERROR, "Freeze too much times in a short time.");
        errorinfo.iErrorCode = ERROR_APP_FREEZE_TOO_FREQUENCE_FAILED;
        errorinfo.strDbName = "";
        vecAppFailedList.push_back(errorinfo);
        return ERROR_APP_FREEZE_TOO_FREQUENCE_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 App::Freeze(mp_time& tFreezeTime, vector<app_failed_info_t>& vecAppFailedList)
{
    COMMLOG(OS_LOG_DEBUG, "Begin freeze app.");
    CMpTime::Now(tFreezeTime);
    mp_int32 freezeOldState;
    {
        // 冻结和解冻, 冻结和冻结需要互斥
        CThreadAutoLock lock(&m_freezeStateLock);
        freezeOldState = m_freezeState;
        if (m_freezeState == DB_FREEZING || m_freezeState == DB_UNFREEZING) {
            COMMLOG(OS_LOG_ERROR, "There hava another freeze or unfreeze task is running.");
            app_failed_info_t errorinfo;
            errorinfo.iErrorCode = ERROR_COMMON_APP_FREEZE_FAILED;
            errorinfo.strDbName = "";
            vecAppFailedList.push_back(errorinfo);
            return ERROR_COMMON_APP_FREEZE_FAILED;
        }
        m_freezeState = DB_FREEZING;
    }

    mp_int32 iRet = CheckFreezeIntervalTime(tFreezeTime, vecAppFailedList);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Check freeze interval time failed, iRet %d.", iRet);
        m_freezeState = freezeOldState;
        return iRet;
    }

#ifdef WIN32
    iRet = FreezeVss(vecAppFailedList);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Vss freeze failed, iRet %d.", iRet);
        m_freezeState = freezeOldState;
        return iRet;
    }
#else
    iRet = FreezeApp(vecAppFailedList);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "APP freeze failed, iRet %d.", iRet);
        m_freezeState = freezeOldState;
        return iRet;
    }
#endif

    COMMLOG(OS_LOG_DEBUG, "Freeze app success.");
    m_freezeState = DB_FREEZE;
    return MP_SUCCESS;
}

mp_int32 App::UnFreeze(vector<app_failed_info_t>& vecAppFailedList)
{
    COMMLOG(OS_LOG_DEBUG, "Begin unfreeze app.");

    mp_int32 freezeOldState;
    {
        // 冻结和解冻, 解冻和解冻需要互斥.
        CThreadAutoLock lock(&m_freezeStateLock);
        freezeOldState = m_freezeState;
        if (m_freezeState == DB_FREEZING || m_freezeState == DB_UNFREEZING) {
            COMMLOG(OS_LOG_ERROR, "There hava another freeze or unfreeze task is running.");
            app_failed_info_t errorinfo;
            errorinfo.iErrorCode = ERROR_COMMON_APP_THAW_FAILED;
            errorinfo.strDbName = "";
            vecAppFailedList.push_back(errorinfo);
            return ERROR_COMMON_APP_THAW_FAILED;
        }
        m_freezeState = DB_UNFREEZING;
    }

    mp_int32 iRet = UnFreezeInner(vecAppFailedList);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Unfreeze all apps failed, iRet %d.", iRet);
        m_freezeState = freezeOldState;
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Unfreeze app success.");
    m_freezeState = DB_UNFREEZE;
    return MP_SUCCESS;
}

mp_int32 App::EndBackup(mp_int32 iBackupSucc, vector<app_failed_info_t>& vecAppFailedList)
{
    COMMLOG(OS_LOG_DEBUG, "Begin end backup.");
#ifdef WIN32
    mp_int32 iRet = EndBackupVss(iBackupSucc, vecAppFailedList);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Vss end backup failed, iRet %d.", iRet);
        return iRet;
    }
#else
    COMMLOG(OS_LOG_DEBUG, "Do nothing for non windows.");
    (mp_void)iBackupSucc;
    (mp_void)vecAppFailedList;
#endif
    COMMLOG(OS_LOG_DEBUG, "Endbackup success.");
    return MP_SUCCESS;
}

mp_int32 App::TruncateLog(mp_time tTruncateTime, vector<app_failed_info_t>& vecAppFailedList)
{
    (mp_void)tTruncateTime;
    (mp_void)vecAppFailedList;
    mp_int32 ret = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, "Not impplement.");
    return ret;
}

mp_int32 App::FreezeApp(vector<app_failed_info_t>& vecAppFailedList)
{
    LOGGUARD("");

#ifndef WIN32
    mp_int32 iRet;
    mp_string strParam;
    app_failed_info_t errorInfo;

    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_FREEZEAPP, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_APP_FREEZE_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Excute freeze app failed, iRet %d, start unfreeze app.", iRet);
        errorInfo.iErrorCode = iRet;
        errorInfo.strDbName = "";
        vecAppFailedList.push_back(errorInfo);
        vector<app_failed_info_t> vecUnfreezeAppFailedList;
        (void)UnFreezeApp(vecUnfreezeAppFailedList);
        return iRet;
    }
#endif
    (mp_void)vecAppFailedList;
    COMMLOG(OS_LOG_INFO, "End freeze all app instances.");
    return MP_SUCCESS;
}

mp_int32 App::UnFreezeApp(vector<app_failed_info_t>& vecAppFailedList)
{
    LOGGUARD("");

#ifndef WIN32
    mp_int32 iRet;
    mp_string strParam;
    app_failed_info_t errorInfo;

    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_THAWAPP, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_APP_THAW_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Excute unfreeze app failed, iRet %d.", iRet);
        errorInfo.iErrorCode = iRet;
        errorInfo.strDbName = "";
        vecAppFailedList.push_back(errorInfo);
        return iRet;
    }
#endif

    COMMLOG(OS_LOG_INFO, "End unfreeze all app instances.");
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: QueryFreezeState
Description  : 冻结保护机制查询当前主机上应用的冻结状态，查询失败返回未知，全部都已解冻返回1，存在一个未解冻应用返回0
Others       :------------------------------------------------------------- */
mp_int32 App::QueryFreezeState(mp_int32& iState)
{
    COMMLOG(OS_LOG_INFO, "Begin query freeze state of all apps.");

    CThreadAutoLock lock(&m_freezeStateLock);
    iState = ((DB_UNFREEZE == m_freezeState) ? DB_UNFREEZE : DB_FREEZE);
    mp_int32 ret = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, "Query freeze state of all apps success, freeze state is %d.", iState);
    return ret;
}

/* ---------------------------------------------------------------------------
Function Name: UnFreezeEx
Description  : 冻结保护机制使用，执行解冻和结束备份操作，重复解冻时不返回错误
Others       :------------------------------------------------------------- */
mp_int32 App::UnFreezeEx()
{
    COMMLOG(OS_LOG_INFO, "(EX)Begin unfreeze all apps.");

    mp_int32 freezeOldState;
    {
        // 冻结和解冻, 解冻和解冻需要互斥.
        CThreadAutoLock lock(&m_freezeStateLock);
        if (m_freezeState == DB_UNFREEZE) {  // 已经解冻, 则不再需要解冻.
            COMMLOG(OS_LOG_INFO, "(EX)All app is already unfreezed.");
            return MP_SUCCESS;
        }
#ifdef WIN32
        if (m_pVssRequester == NULL) {
            // 根据测试结果。windows当vss超时后会自动结束备份状态，此处不做处理
            COMMLOG(OS_LOG_INFO, "(EX)All app is already unfreezed.");
            m_freezeState = DB_UNFREEZE;
            return MP_SUCCESS;
        }
#endif
        freezeOldState = m_freezeState;
        if (m_freezeState == DB_FREEZING || m_freezeState == DB_UNFREEZING) {
            COMMLOG(OS_LOG_ERROR, "(EX)There hava another freeze or unfreeze task is running.");
            return ERROR_COMMON_APP_THAW_FAILED;
        }
        m_freezeState = DB_UNFREEZING;
    }

    vector<app_failed_info_t> vecAppFailedList;
    mp_int32 iRet = UnFreezeInner(vecAppFailedList);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "(EX)Unfreeze all apps failed, iRet %d.", iRet);
        m_freezeState = freezeOldState;
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "(EX)Unfreeze all apps success.");
    m_freezeState = DB_UNFREEZE;
    return MP_SUCCESS;
}

mp_int32 App::UnFreezeInner(vector<app_failed_info_t>& vecAppFailedList)
{
    LOGGUARD("");
    vecAppFailedList.clear();

#ifdef WIN32
    mp_int32 iRet = UnFreezeVss(vecAppFailedList);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Vss unfreeze failed, iRet %d.", iRet);
        return iRet;
    }
#else
    mp_int32 iRet = UnFreezeApp(vecAppFailedList);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Unfreeze all apps failed, iRet %d.", iRet);
        return iRet;
    }
#endif
    vecAppFailedList.clear();
    iRet = EndBackup(0, vecAppFailedList);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "EndBackup failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Unfreeze all apps success.");
    return MP_SUCCESS;
}

#ifdef WIN32
mp_int32 App::FreezeVss(vector<app_failed_info_t>& vecAppFailedList)
{
    mp_int32 iRet;
    app_failed_info_t appFailedInfo;

    COMMLOG(OS_LOG_INFO, "Begin vss freeze.");

    if (m_pVssRequester != NULL) {
        appFailedInfo.iErrorCode = ERROR_VSS_OTHER_FREEZE_RUNNING;
        appFailedInfo.strDbName = DBNAME_FOR_VSS_IN_ERR_RESPONSE;
        vecAppFailedList.push_back(appFailedInfo);
        COMMLOG(OS_LOG_ERROR, "Other freeze opertion is running.");
        return ERROR_VSS_OTHER_FREEZE_RUNNING;
    }

    NEW_CATCH(m_pVssRequester, VSSRequester);
    if (m_pVssRequester == NULL) {
        appFailedInfo.iErrorCode = ERROR_COMMON_APP_FREEZE_FAILED;
        appFailedInfo.strDbName = DBNAME_FOR_VSS_IN_ERR_RESPONSE;
        return ERROR_COMMON_APP_FREEZE_FAILED;
    }

    iRet = m_pVssRequester->FreezeAll();
    if (iRet != MP_SUCCESS) {
        FreeVssRequester();

        appFailedInfo.iErrorCode = iRet;
        appFailedInfo.strDbName = DBNAME_FOR_VSS_IN_ERR_RESPONSE;
        vecAppFailedList.push_back(appFailedInfo);
        COMMLOG(OS_LOG_ERROR, "Freeze all volumes failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Vss freeze success.");
    return MP_SUCCESS;
}

mp_int32 App::UnFreezeVss(vector<app_failed_info_t>& vecAppFailedList)
{
    mp_int32 iRet;
    app_failed_info_t appFailedInfo;

    COMMLOG(OS_LOG_INFO, "Begin vss unfreeze.");

    if (m_pVssRequester == NULL) {
        appFailedInfo.iErrorCode = ERROR_COMMON_APP_THAW_FAILED;
        appFailedInfo.strDbName = DBNAME_FOR_VSS_IN_ERR_RESPONSE;
        vecAppFailedList.push_back(appFailedInfo);
        COMMLOG(OS_LOG_ERROR, "No need to unfreeze.");
        return ERROR_COMMON_APP_THAW_FAILED;
    }

    iRet = m_pVssRequester->UnFreezeAll();
    if (iRet != MP_SUCCESS) {
        FreeVssRequester();

        appFailedInfo.iErrorCode = iRet;
        appFailedInfo.strDbName = DBNAME_FOR_VSS_IN_ERR_RESPONSE;
        vecAppFailedList.push_back(appFailedInfo);
        COMMLOG(OS_LOG_ERROR, "Unfreeze all volumes failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "VSS unfreeze success.");
    return MP_SUCCESS;
}

mp_int32 App::EndBackupVss(mp_int32 iBackupSucc, vector<app_failed_info_t>& vecAppFailedList)
{
    mp_int32 iRet;
    app_failed_info_t appFailedInfo;

    COMMLOG(OS_LOG_INFO, "Begin vss endbakup.");

    if (m_pVssRequester == NULL) {
        COMMLOG(OS_LOG_DEBUG, "No need to end backup.");
        return MP_SUCCESS;
    }

    iRet = m_pVssRequester->EndBackup(iBackupSucc);
    if (iRet != MP_SUCCESS) {
        FreeVssRequester();

        appFailedInfo.iErrorCode = iRet;
        appFailedInfo.strDbName = DBNAME_FOR_VSS_IN_ERR_RESPONSE;
        vecAppFailedList.push_back(appFailedInfo);
        COMMLOG(OS_LOG_ERROR, "End backup failed, iRet %d.", iRet);
        return iRet;
    }

    FreeVssRequester();
    COMMLOG(OS_LOG_INFO, "Vss endbakup success.");
    return MP_SUCCESS;
}

mp_void App::FreeVssRequester()
{
    if (m_pVssRequester != NULL) {
        delete m_pVssRequester;
        m_pVssRequester = NULL;
    }
}
#endif
