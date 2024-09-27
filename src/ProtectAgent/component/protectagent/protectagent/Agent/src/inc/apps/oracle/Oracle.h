#ifndef __AGENT_ORACLE_H__
#define __AGENT_ORACLE_H__

#include <vector>
#include <list>
#include <map>
#include "common/Types.h"
#include "host/host.h"
#include "apps/oracle/OracleDefines.h"

class Oracle {
public:
    Oracle();
    ~Oracle();

    mp_int32 GetDBInfo(std::list<oracle_inst_info_t>& lstOracleInstInfo, mp_string& strVer, mp_string& strHome);
    mp_int32 GetDBLUNInfo(oracle_db_info_t& stDBInfo, std::vector<oracle_lun_info_t>& vecLUNInfos);
    mp_int32 Test(oracle_db_info_t& stDBInfo);
    mp_int32 CheckArchiveThreshold(oracle_db_info_t& stDBInfo);
    mp_int32 CheckCDB(oracle_db_info_t& stDBInfo, mp_int32& iCDBType);
    mp_int32 GetPDBInfo(oracle_pdb_req_info_t& stPdbReqInfo, std::vector<oracle_pdb_rsp_info_t>& vecOraclePdbInfo);
    mp_int32 StartPluginDB(oracle_pdb_req_info_t& stPdbReqInfo);

    // query database host role in cluster
    mp_int32 QueryHostRoleInCluster(
        const oracle_db_info_t& stDBinfo, std::vector<mp_string>& storHosts, std::vector<mp_string>& dbHosts);
    // query database tablespace list
    mp_int32 QueryTableSpace(const oracle_db_info_t& stDBInfo, std::vector<mp_string>& tablespaces);
    mp_int32 QueryASMInstance(mp_string& asmInst);
    mp_int32 GetCluterInfo(Json::Value& clsInfo);

#ifdef WIN32
    static DWORD CheckArchiveAreaThread(void* pThis);
#else
    EXTER_ATTACK static mp_void* CheckArchiveAreaThread(void* pThis);
#endif
    void CheckArchiveArea();
    std::vector<oracle_db_info_t> QueryOracleDbInfo();
    bool GetFlagTExitCheckArchiveArea();
    void SetFlagTExitCheckArchiveArea(bool bExit);

private:
    mp_void BuildTestScriptParam(const oracle_db_info_t& stDBInfo, mp_string& strParam);
    mp_int32 BuildTruncateLogScriptParam(oracle_db_info_t& stDBInfo, mp_time truncTime, mp_string& strParam);
    mp_void BuildCheckThresholdScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam);
    mp_void BuildStartASMScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam);
    mp_void BuildStopASMScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam);
    mp_void BuildStartOracleScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam);
    mp_void BuildStopOracleScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam);
    mp_void BuildCheckCDBScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam);

private:
    volatile bool m_bTExitCheckArchiveArea;
};

#endif  // __AGENT_ORACLE_H__
