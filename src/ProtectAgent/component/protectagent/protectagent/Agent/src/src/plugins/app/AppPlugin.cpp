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
#include "plugins/app/AppPlugin.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "message/rest/interfaces.h"
#include "common/Path.h"
#include "common/Defines.h"
#include "common/Utils.h"

#include <cmath>
using namespace std;

// 秒转换成小时
static mp_int32 SECONDS_PER_HOUR = 3600;
static mp_int32 SECONDS_PER_MINITE = 60;

AppPlugin::AppPlugin()
{
#ifdef REST_PUBLISH
    REGISTER_ACTION(REST_APP_FREEZE, REST_URL_METHOD_PUT, &AppPlugin::Freeze);
    REGISTER_ACTION(REST_APP_UNFREEZE, REST_URL_METHOD_PUT, &AppPlugin::UnFreeze);
    REGISTER_ACTION(REST_APP_QUERY_DB_FREEZESTATE, REST_URL_METHOD_GET, &AppPlugin::QueryFreezeState);
    REGISTER_ACTION(REST_APP_UNFREEZEEX, REST_URL_METHOD_PUT, &AppPlugin::UnFreezeEx);
#endif
}

AppPlugin::~AppPlugin()
{}

mp_int32 AppPlugin::DoAction(CRequestMsg& req, CResponseMsg& rsp)
{
    DO_ACTION(AppPlugin, req, rsp);
}

mp_void AppPlugin::GetDBAuthInfo(CRequestMsg& req, app_auth_info_t& stAppAuthInfo)
{
    mp_string pHeadStr = req.GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBUSERNAME);
    stAppAuthInfo.strUserName = CMpString::Trim(pHeadStr);

    pHeadStr = req.GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBPASSWORD);
    stAppAuthInfo.strPasswd = CMpString::Trim(pHeadStr);
}

mp_int32 AppPlugin::Freeze(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void)req;
    mp_int32 iRet;
    // CodeDex误报，UNINIT
    mp_time tFreezeTime;
    vector<app_failed_info_t> vecAppFailedList;
    vector<app_failed_info_t>::iterator iter;
    Json::Value& jRspValue = rsp.GetJsonValueRef();

    COMMLOG(OS_LOG_INFO, "Begin freeze app.");
    // CodeDex误报,KLOCWORK.ITER.END.DEREF.MIGHT
    iRet = m_app.Freeze(tFreezeTime, vecAppFailedList);
    if (iRet != MP_SUCCESS) {
        iter = vecAppFailedList.begin();
        Json::Value jValue;
        jValue[REST_PARAM_APP_ERROR_CODE] = iter->iErrorCode;
        jValue[REST_PARAM_APP_DBNAME] = iter->strDbName;
        (mp_void)jRspValue.append(std::move(jValue));
        COMMLOG(OS_LOG_ERROR, "Freeze app failed, iRet %d.", iRet);
        return iRet;
    }
    jRspValue[REST_PARAM_APP_TIME] = static_cast<Json::UInt64>(tFreezeTime);

    COMMLOG(OS_LOG_INFO, "Freeze app succ.");
    return MP_SUCCESS;
}

mp_int32 AppPlugin::UnFreeze(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void)req;
    mp_int32 iRet;
    vector<app_failed_info_t> vecAppFailedList;
    vector<app_failed_info_t>::iterator iter;
    Json::Value& jRspValue = rsp.GetJsonValueRef();

    COMMLOG(OS_LOG_INFO, "Begin unfreeze app.");
    mp_time tUnFreezeTime;
    CMpTime::Now(tUnFreezeTime);
    // CodeDex误报,KLOCWORK.ITER.END.DEREF.MIGHT
    iRet = m_app.UnFreeze(vecAppFailedList);
    if (iRet != MP_SUCCESS) {
        iter = vecAppFailedList.begin();
        Json::Value jValue;
        jValue[REST_PARAM_APP_ERROR_CODE] = iter->iErrorCode;
        jValue[REST_PARAM_APP_DBNAME] = iter->strDbName;
        (mp_void)jRspValue.append(std::move(jValue));
        COMMLOG(OS_LOG_ERROR, "Unfreeze app failed, iRet %d.", iRet);
        return iRet;
    }

    jRspValue[REST_PARAM_APP_TIME] = static_cast<Json::UInt64>(tUnFreezeTime);
    COMMLOG(OS_LOG_INFO, "Unfreeze app succ.");
    return MP_SUCCESS;
}

mp_int32 AppPlugin::EndBackup(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_int32 iRet;
    vector<app_failed_info_t> vecAppFailedList;
    vector<app_failed_info_t>::iterator iter;
    Json::Value& jRspValue = rsp.GetJsonValueRef();
    mp_int32 iBackupSucc = 0;

    COMMLOG(OS_LOG_INFO, "Begin end backup.");

    const Json::Value& jReqValue = req.GetMsgBody().GetJsonValueRef();
    GET_JSON_INT32(jReqValue, REST_PARAM_APP_BACKUP_SUCC, iBackupSucc);

    // 参数校验
    vector<mp_int32> vecExclude;
    CHECK_FAIL_EX(CheckParamInteger32(iBackupSucc, 0, 1, vecExclude));

    COMMLOG(OS_LOG_DEBUG, "Get backupSucc param %d.", iBackupSucc);
    // CodeDex误报，Dead Code
    iRet = m_app.EndBackup(iBackupSucc, vecAppFailedList);
    if (iRet != MP_SUCCESS) {
        iter = vecAppFailedList.begin();
        Json::Value jValue;
        jValue[REST_PARAM_APP_ERROR_CODE] = iter->iErrorCode;
        jValue[REST_PARAM_APP_DBNAME] = iter->strDbName;
        (mp_void)jRspValue.append(std::move(jValue));
        COMMLOG(OS_LOG_ERROR, "Unfreeze app failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "End backup succ.");
    return MP_SUCCESS;
}

mp_int32 AppPlugin::TruncateLog(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_int32 iRet;
    mp_time tTruncateTime;
    vector<app_failed_info_t> vecAppFailedList;
    vector<app_failed_info_t>::iterator iter;
    Json::Value& jRspValue = rsp.GetJsonValueRef();

    COMMLOG(OS_LOG_INFO, "Begin truncage log.");

    const Json::Value& jReqValue = req.GetMsgBody().GetJsonValueRef();
    mp_int64 i64Tmp = 0;
    GET_JSON_INT64(jReqValue, REST_PARAM_APP_TIME, i64Tmp);

    // 参数校验
    vector<mp_int64> vecExclude;
    mp_int64 begValue, endValue;
    begValue = 1;
#ifdef HP_UX_IA
    endValue = LONG_LONG_MAX - 1;
#else
    endValue = LLONG_MAX - 1;
#endif
    CHECK_FAIL_EX(CheckParamInteger64(i64Tmp, begValue, endValue, vecExclude));

    tTruncateTime = (mp_time)i64Tmp;
    COMMLOG(OS_LOG_DEBUG, "Get truncate time param %lld.", tTruncateTime);
    // CodeDex误报,KLOCWORK.ITER.END.DEREF.MIGHT
    iRet = m_app.TruncateLog(tTruncateTime, vecAppFailedList);
    if (iRet != MP_SUCCESS) {
        iter = vecAppFailedList.begin();
        Json::Value jValue;
        jValue[REST_PARAM_APP_ERROR_CODE] = iter->iErrorCode;
        jValue[REST_PARAM_APP_DBNAME] = iter->strDbName;
        (mp_void)jRspValue.append(std::move(jValue));
        COMMLOG(OS_LOG_ERROR, "Truncate log failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Truncate log succ.");
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: QueryFreezeState
Description  : 冻结保护机制查询当前主机上应用的冻结状态，查询失败返回未知，全部都已解冻返回1，存在一个未解冻应用返回0
Others       :------------------------------------------------------------- */
mp_int32 AppPlugin::QueryFreezeState(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void)req;
    mp_int32 iRet;
    mp_int32 iFreezeState = DB_UNFREEZE;
    iRet = m_app.QueryFreezeState(iFreezeState);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query freeze state failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& jRspValue = rsp.GetJsonValueRef();
    jRspValue[REST_PARAM_APP_STATE] = iFreezeState;

    COMMLOG(OS_LOG_INFO, "Query freeze state succ.");
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: UnFreezeEx
Description  : 冻结保护机制使用，执行解冻和结束备份操作，重复解冻时不返回错误
Others       :------------------------------------------------------------- */
mp_int32 AppPlugin::UnFreezeEx(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void)req;
    (mp_void)rsp;
    COMMLOG(OS_LOG_INFO, "(EX)Begin unfreeze.");
    return m_app.UnFreezeEx();
}