#ifndef __AGENT_ORACLE_INFO_H__
#define __AGENT_ORACLE_INFO_H__

#include <vector>
#include <list>
#include "common/Types.h"
#include "common/Utils.h"
#include "common/JsonUtils.h"

#include "apps/oracle/OracleDefines.h"

class OracleInfo {
public:
    OracleInfo();
    ~OracleInfo();

    static mp_int32 GetDBInfo(std::list<oracle_inst_info_t>& lstOracleInstInfo, mp_string& strVer, mp_string& strHome);
    static mp_void BuildConsistentScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam,
        const mp_string& strFrushType);
    static mp_void BuildPDBInfoScriptParam(oracle_pdb_req_info_t& stPdbInfo, mp_string& strParam);
    static mp_int32 AnalyseClsInfoScriptRst(std::vector<mp_string>& vecResult, Json::Value& clsInfo);

private:
    static mp_int32 AnalyseInstInfoScriptRst(std::vector<mp_string>& vecResult, mp_string& strVer, mp_string& strHome,
        std::list<oracle_inst_info_t>& lstOracleInstInfo);
    static mp_int32 AnalyseDatabaseNameByConfFile(std::list<oracle_inst_info_t>& lstOracleInstInfo);
    static mp_int32 AnalysePDBInfoScriptRst(
        std::vector<mp_string>& vecResult, std::vector<oracle_pdb_rsp_info_t>& vecOraclePdbInfo);
    static mp_int32 TranslatePDBStatus(mp_string& strStatus, mp_int32& iStatus);
    static mp_int32 FillOracleInsInfo(mp_string vecResult, oracle_inst_info_t& oracleInstInfo);

private:
    static const int ORALCE_BYTES_256 = 256;
    static const int ORALCE_COMMON_FIELD_NUM = 2;
};

#endif  // __AGENT_ORACLE_H__
