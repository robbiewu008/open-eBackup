#include "plugins/app/AppPlugin.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "message/rest/interfaces.h"
#include "common/Path.h"
#include "common/Defines.h"
#include "common/Utils.h"
#include "apps/oracle/Oracle.h"

#include <cmath>
using namespace std;

// 秒转换成小时
static mp_int32 SECONDS_PER_HOUR = 3600;
static mp_int32 SECONDS_PER_MINITE = 60;

AppPlugin::AppPlugin()
{
    REGISTER_ACTION(REST_APP_QUERY_DB_INFO, REST_URL_METHOD_GET, &AppPlugin::QueryInfo);
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

mp_int32 AppPlugin::QueryOracleList(Json::Value& oraList)
{
    // Get oracle information
    Oracle oracle;
    list<oracle_inst_info_t> lstOraInstInfo;
    mp_string strVer;
    mp_string strHome;
    mp_int32 iRet = oracle.GetDBInfo(lstOraInstInfo, strVer, strHome);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query database info failed.");
        return iRet;
    }

    oraList["version"] = strVer;
    oraList["oracleHome"] = strHome;
    Json::Value oraIntances;
    for (list<oracle_inst_info_t>::iterator iter = lstOraInstInfo.begin(); iter != lstOraInstInfo.end(); ++iter) {
        Json::Value jValue;
        jValue[RESPOND_ORACLE_PARAM_INSTNAME] = iter->strInstName;
        jValue[RESPOND_ORACLE_PARAM_DBNAME] = iter->strDBName;
        jValue[RESPOND_ORACLE_PARAM_STATE] = iter->iState;
        jValue[RESPOND_ORACLE_PARAM_ISASMDB] = iter->iIsASMDB;
        jValue[RESPOND_ORACLE_PARAM_AUTHTYPE] = iter->authType;
        jValue[RESPOND_ORACLE_PARAM_DBROLE] = iter->dbRole;
        oraIntances.append(std::move(jValue));
    }
    oraList["instances"] = std::move(oraIntances);

    return iRet;
}

mp_int32 AppPlugin::QueryInfo(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void)req;
    Json::Value& jRspValue = rsp.GetJsonValueRef();

    Json::Value oraList;
    mp_int32 iRet = QueryOracleList(oraList);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query database info failed.");
    } else {
        jRspValue["oracle"] = std::move(oraList);
    }

    timezone_info_t sttimezone;
    iRet = m_host.GetTimeZone(sttimezone);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query host time zone info failed, iRet is %d.", ERROR_HOST_GET_TIMEZONE_FAILED);

        return ERROR_HOST_GET_TIMEZONE_FAILED;
    }
    jRspValue["timezone"] = sttimezone.strTzBias;

    return MP_SUCCESS;
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
