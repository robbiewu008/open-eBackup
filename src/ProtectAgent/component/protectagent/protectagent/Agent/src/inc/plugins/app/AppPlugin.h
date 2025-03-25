#ifndef __AGENT_APP_PLUGIN_H__
#define __AGENT_APP_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "apps/app/App.h"
#include "host/host.h"

static const mp_string REST_PARAM_APP_ORALCE = "oracle";
static const mp_string REST_PARAM_APP_SQLSERVER = "sqlserver";
static const mp_string REST_PARAM_APP_DB2 = "db2";
static const mp_string REST_PARAM_APP_EXCHANGE = "exchange";
static const mp_string REST_PARAM_APP_INSTNAME = "instName";
static const mp_string REST_PARAM_APP_DBNAME  = "dbName";
static const mp_string REST_PARAM_APP_VERSION = "version";
static const mp_string REST_PARAM_APP_STORAGE_GROUP = "storageGroup";
static const mp_string REST_PARAM_APP_BACKUP_SUCC = "backupSucc";
static const mp_string REST_PARAM_APP_TIME  = "time";
static const mp_string REST_PARAM_APP_ERROR_CODE = "errorCode";
static const mp_string REST_PARAM_APP_STATE = "state";

class AppPlugin : public CServicePlugin {
public:
    AppPlugin();
    ~AppPlugin();

    mp_int32 DoAction(CRequestMsg& req, CResponseMsg& rsp);

private:
    App m_app;

private:
    CHost m_host;
    mp_void GetDBAuthInfo(CRequestMsg& req, app_auth_info_t& stAppAuthInfo);
    mp_int32 QueryInfo(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 Freeze(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 UnFreeze(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 EndBackup(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 TruncateLog(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 GetDBFreezeState(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 QueryFreezeState(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 UnFreezeEx(CRequestMsg& req, CResponseMsg& rsp);

    mp_int32 QueryOracleList(Json::Value& oraList);
};
REGISTER_PLUGIN(AppPlugin);
#endif  // __AGENT_APP_PLUGIN_H__
