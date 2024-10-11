#ifndef _ORACLE_NATIVE_BACKUP_H_
#define _ORACLE_NATIVE_BACKUP_H_

#include <map>
#include <set>
#include <vector>

#include "apps/oracle/Oracle.h"
#include "apps/oracle/OracleDefines.h"
#include "jsoncpp/include/json/json.h"
#include "jsoncpp/include/json/value.h"

class OracleNativeBackup {
public:
    OracleNativeBackup();
    virtual ~OracleNativeBackup();

    mp_int32 GetDBStorInfo(const oracle_db_info_t& stDBInfo, Json::Value& dbInfo);
    mp_int32 QueryBackupLevel(const Json::Value& reqBody, Json::Value& rspBody);
    mp_int32 PrepareMedia(const mp_string& msgBody, mp_int32 taskType, mp_int32 storType, Json::Value& respMsg);
    mp_int32 BackupData(const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId);
    mp_int32 BackupLog(const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId);
    mp_int32 RestoreDB(const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId);
    mp_int32 LiveMount(const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId);
    mp_int32 CancelLiveMount(const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId);
    mp_int32 InstanceRestore(const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId);
    mp_int32 ExpireCopy(const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId);
    mp_int32 DisMountMedium(const mp_string& msgBody, mp_int32 taskType);
    mp_int32 StopTask(const mp_string& msgBody, mp_string& taskId);
    mp_int32 CreateCheckMountTask();
    bool GetCMExitFlag();
    mp_int32 CheckMount();

private:
    mp_int32 GetTaskidByReq(const mp_string& msgBody, mp_string& taskid);
    mp_int32 BackupOracle(
        const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_int32 backupMode, mp_string& taskId);
    typedef mp_int32 (OracleNativeBackup::*FUNC_ANALYSEINFO)(
        const std::vector<mp_string>& storInfos, Json::Value& dbInfo);
    std::map<mp_string, FUNC_ANALYSEINFO> HandlerStoreInfos;
    mp_void BuildQueryStorageTypeScriptParam(const oracle_db_info_t& stDBInfo, mp_string& strParam);
    mp_void BuildQueryStorageInfoScriptParam(const oracle_db_info_t& stDBInfo, mp_string& strParam);
    mp_int32 RunBackuplevelScript(Json::Value& rspBody, const mp_string& strParam);
    mp_int32 AnalyseStorInfoScriptRst(std::vector<mp_string>& vecResult, Json::Value& dbInfo);

    // analyze database storage information handler
    mp_int32 DataCapHandler(const std::vector<mp_string>& storInfos, Json::Value& dbInfo);
    mp_int32 LogCapHandler(const std::vector<mp_string>& storInfos, Json::Value& dbInfo);
    mp_int32 DBTypeHandler(const std::vector<mp_string>& storInfos, Json::Value& dbInfo);
    mp_int32 DBInsLstHandler(const std::vector<mp_string>& storInfos, Json::Value& dbInfo);
#ifdef WIN32
    static DWORD CheckMountTaskInThread(void* pThis);
#else
    static mp_void* CheckMountTaskInThread(void* pThis);
#endif
    void QueryMountPointInfo(std::map<mp_string, std::set<mp_string> >& mapMountInfo);
    thread_id_t m_tidCheckMount;
    volatile bool m_bTExitCheckMount;

private:
    Oracle m_oracle;
};
#endif  // _ORACLE_NATIVE_BACKUP_H_
