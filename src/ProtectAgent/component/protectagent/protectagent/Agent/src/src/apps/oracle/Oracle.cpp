#include "apps/oracle/Oracle.h"
#include <sstream>
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/MpString.h"
#include "common/CSystemExec.h"
#include "common/Path.h"
#include "common/DB.h"
#include "common/ConfigXmlParse.h"
#include "common/Ip.h"
#include "securecom/RootCaller.h"
#include "securecom/CryptAlg.h"
#include "array/array.h"
#include "alarm/AlarmHandle.h"
#include "alarm/Trap.h"
#include "apps/oracle/OracleInfo.h"
#include "apps/oracle/OracleLunInfo.h"
#include "securecom/SecureUtils.h"

using namespace std;
Oracle::Oracle()
{
    m_bTExitCheckArchiveArea = false;
}

Oracle::~Oracle()
{}

namespace {
const int ORALCE_YEAR_1900 = 1900;
const mp_string ARCHIVE_THRESHOLD = "80";
const int ARCHIVE_THREAD_TIMEOUT = 30000;
}

/* ------------------------------------------------------------
Description  : get oracle list
Input        : lstOracleInstInfo -- oracle database list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 Oracle::GetDBInfo(list<oracle_inst_info_t>& lstOracleInstInfo, mp_string& strVer, mp_string& strHome)
{
    return OracleInfo::GetDBInfo(lstOracleInstInfo, strVer, strHome);
}

/* ------------------------------------------------------------
Description  : start oracle pdb
Input        : stPdbReqInfo -- oracle PDB info
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
------------------------------------------------------------- */
mp_int32 Oracle::StartPluginDB(oracle_pdb_req_info_t& stPdbReqInfo)
{
    mp_string strParam;
    COMMLOG(OS_LOG_INFO, "Start oracle PDB.");
    OracleInfo::BuildPDBInfoScriptParam(stPdbReqInfo, strParam);

#ifdef WIN32
    COMMLOG(OS_LOG_INFO, "Start PDB failed, this function is unimplement");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    vector<mp_string> vecResult;
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_STARTORACLEPDB, strParam, &vecResult);
    ClearString(strParam);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Excute starting oracle PDB script failed, iRet %d", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Start PDB succ.");
    return MP_SUCCESS;
#endif
}

mp_int32 Oracle::QueryHostRoleInCluster(
    const oracle_db_info_t& stDBinfo, vector<mp_string>& storHosts, vector<mp_string>& dbHosts)
{
    mp_string strParam;
    COMMLOG(OS_LOG_INFO, "Begin to query database(%s) role in cluster.", stDBinfo.strDBName.c_str());
#ifdef WIN32
    COMMLOG(OS_LOG_INFO, "Cluster isn't supported in windows cluster.");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    BuildTestScriptParam(stDBinfo, strParam);
    vector<mp_string> vecResult;
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLE_QUERYROLE, strParam, &vecResult);
    ClearString(strParam);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Excute query oracle(%s) host role failed, iRet %d", stDBinfo.strDBName.c_str(), iRet);
        return iRet;
    }

    static const mp_int32 RST_NUM = 2;
    if (vecResult.size() != RST_NUM) {
        COMMLOG(OS_LOG_ERROR,
            "Excute query oracle(%s) host role failed, result number(%d) is invalid.",
            stDBinfo.strDBName.c_str(),
            vecResult.size());
        return iRet;
    }

    mp_int32 idx = 1;
    CMpString::StrSplit(dbHosts, vecResult[idx++], CHAR_COMMA);
    CMpString::StrSplit(storHosts, vecResult[idx], CHAR_COMMA);
    COMMLOG(OS_LOG_INFO, "Query oracle(%s) host role succ.", stDBinfo.strDBName.c_str());
    return MP_SUCCESS;
#endif
}

mp_int32 Oracle::QueryTableSpace(const oracle_db_info_t& stDBInfo, std::vector<mp_string>& tablespaces)
{
    mp_string strParam;
    COMMLOG(OS_LOG_INFO, "Begin to query database(%s) tablespace.", stDBInfo.strDBName.c_str());
    BuildTestScriptParam(stDBInfo, strParam);

#ifdef WIN32
    // windows下调用脚本
    mp_int32 iRet = SecureCom::SysExecScript(WIN_ORACLE_QUERY_TABLESPACE, strParam, &tablespaces);
    ClearString(strParam);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR,
            "Query database tablespace failed, initial return code is %d, tranformed return code is %d",
            iRet,
            iNewRet);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLE_QUERYTABLESPACE, strParam, &tablespaces);
    ClearString(strParam);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(
            OS_LOG_ERROR, "Excute query oracle(%s) tablespace list failed, iRet %d", stDBInfo.strDBName.c_str(), iRet);
        return iRet;
    }
#endif
    if (tablespaces.empty()) {
        COMMLOG(OS_LOG_ERROR, "Excute query oracle(%s) tablespace list failed.", stDBInfo.strDBName.c_str());
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "Query oracle(%s) tablespace succ.", stDBInfo.strDBName.c_str());
    return MP_SUCCESS;
}

mp_int32 Oracle::QueryASMInstance(mp_string& asmInst)
{
    COMMLOG(OS_LOG_DEBUG, "Begin to query oracle asm instance.");

    vector<mp_string> vecResult;
#ifdef WIN32
    // windows下调用脚本
    mp_int32 iRet = SecureCom::SysExecScript(WIN_ORACLE_QUERY_ASM, "", &vecResult);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR,
            "Query ASM instance failed, initial return code is %d, tranformed return code is %d",
            iRet,
            iNewRet);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLE_ASM, "", &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query ASM instance failed, iRet %d", iRet);
        return iRet;
    }
#endif
    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "Host have no asm instance.");
        return MP_SUCCESS;
    }
    asmInst = vecResult.front();
    COMMLOG(OS_LOG_INFO, "Query oracle asm succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : get the storage information of the oracle database
Input        : stDBInfo -- the oracle database information
Output       : vecLUNInfos -- the result to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 Oracle::GetDBLUNInfo(oracle_db_info_t& stDBInfo, vector<oracle_lun_info_t>& vecLUNInfos)
{
    return OracleLunInfo::GetDBLUNInfo(stDBInfo, vecLUNInfos);
}

/* ------------------------------------------------------------
Description  : build the script parameter for check threshold
Input        : stDBInfo -- the database structor
Output      : strParam -- the prameter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_void Oracle::BuildCheckThresholdScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ARCHIVETHRESHOLD) + stDBInfo.strArchThreshold + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stDBInfo.strDBPassword + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMINSTANCE) + stDBInfo.strASMInstance + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stDBInfo.strOracleHome;
    ClearString(stDBInfo.strDBPassword);
    ClearString(stDBInfo.strASMPassword);
}

/* ------------------------------------------------------------
Description  : build the script parameter for test database
Input        : stDBInfo -- the database structor
Output      : strParam -- the prameter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_void Oracle::BuildTestScriptParam(const oracle_db_info_t& stDBInfo, mp_string& strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBNAME) + stDBInfo.strDBName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON) +
               ORACLE_SCRIPTPARAM_DBPASSWORD + stDBInfo.strDBPassword + NODE_COLON +
               mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stDBInfo.strOracleHome;
}

/* ------------------------------------------------------------
Description  : execute srcipt to test oracle database
Input        : stDBInfo -- the database structor
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 Oracle::Test(oracle_db_info_t& stDBInfo)
{
    mp_int32 iRet;
    mp_string strParam;

    COMMLOG(OS_LOG_INFO, "Begin test oracle database.");
    BuildTestScriptParam(stDBInfo, strParam);

#ifdef WIN32
    // windows下调用脚本
    iRet = SecureCom::SysExecScript(mp_string(WIN_ORACLE_TEST), strParam, NULL);
    ClearString(strParam);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(
            OS_LOG_ERROR, "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    // Oracle下获取数据库LUN信息需要切换到Oracle用户下，必须在root或者实例用户下执行
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_TESTORACLE, strParam, NULL);
    ClearString(strParam);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
#endif
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Test oracle database(%s-%s) failed, iRet %d",
            stDBInfo.strInstName.c_str(),
            stDBInfo.strDBName.c_str(),
            iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Test oracle database succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : execute srcipt to check threshold
Input        : stDBInfo -- the database structor
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 Oracle::CheckArchiveThreshold(oracle_db_info_t& stDBInfo)
{
    mp_int32 iRet;
    mp_string strParam;

    COMMLOG(OS_LOG_INFO, "Begin check oracle database archive threshold.");
    BuildCheckThresholdScriptParam(stDBInfo, strParam);

#ifdef WIN32
    vector<mp_string> vecResult;
    // windows下调用脚本
    iRet = SecureCom::SysExecScript(mp_string(WIN_ORACLE_CHECK_ARCHIVE), strParam, &vecResult);
    ClearString(strParam);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(
            OS_LOG_ERROR, "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    // Oracle下获取数据库LUN信息需要切换到Oracle用户下，必须在root或者实例用户下执行
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_CHECKARCHIVETHRESHOLD, strParam, NULL);
    ClearString(strParam);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
#endif
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Check oracle database(%s-%s) archive threshold failed, iRet %d",
            stDBInfo.strInstName.c_str(),
            stDBInfo.strDBName.c_str(),
            iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Check oracle database archive threshold succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 构造脚本参数
Input        : stDBInfo -- 数据库信息
               truncTime -- 删除归档日志的截止时间，此时间之前的归档日志都删除
Output       : strParam -- 脚本参数字符串
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 Oracle::BuildTruncateLogScriptParam(oracle_db_info_t& stDBInfo, mp_time truncTime, mp_string& strParam)
{
    // CodeDex误报，UNUSED_VALUE
    mp_tm stTime;
    mp_tm* pTime = CMpTime::LocalTimeR(truncTime, stTime);
    if (pTime == NULL) {
        COMMLOG(OS_LOG_ERROR, "Convert truncTime(%lld) to localTime failed", truncTime);
        return ERROR_COMMON_INVALID_PARAM;
    }
    std::ostringstream strBuf;
    strBuf << ORACLE_SCRIPTPARAM_INSTNAME << stDBInfo.strInstName << STR_COLON << ORACLE_SCRIPTPARAM_DBUSERNAME
           << stDBInfo.strDBUsername << STR_COLON << ORACLE_SCRIPTPARAM_DBPASSWORD << stDBInfo.strDBPassword
           << STR_COLON << ORACLE_SCIPRTPARAM_TRUNCATE_LOG_TIME << (pTime->tm_year + ORALCE_YEAR_1900) << STR_DASH
           << (pTime->tm_mon + 1) << STR_DASH << pTime->tm_mday << STR_DASH << pTime->tm_hour << STR_DASH
           << pTime->tm_min << STR_DASH << pTime->tm_sec << STR_COLON << ORACLE_SCRIPTPARAM_FRUSHTYPE
           << ORACLE_SCRIPTPARAM_TRUNCATEARCHIVELOG;

    strParam = strBuf.str();
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : build the script parameter for check if oracle is cdb
Input        : stDBInfo --the structor of database infomation
Output       : strParam -- string of script parameter
Return       : MP_SUCCESS -- success
               not MP_SUCCESS -- failed，return error code
Create By    : zoubing wx373611
------------------------------------------------------------- */
mp_void Oracle::BuildCheckCDBScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBNAME) + stDBInfo.strDBName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stDBInfo.strDBPassword + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stDBInfo.strOracleHome;
}

/* ------------------------------------------------------------
Description  : execute script to check if oracle is cdb
Input         : stDBInfo -- the structor of database infomation
Output       : iCDBType  -- the type code of oracle cdb
Return       : MP_SUCCESS -- success
               not MP_SUCCESS -- failed ,  return error code
Create By    : zoubing wx373611
------------------------------------------------------------- */
mp_int32 Oracle::CheckCDB(oracle_db_info_t& stDBInfo, mp_int32& iCDBType)
{
    mp_int32 iRet;
    mp_string strParam;
    mp_int32 iCDBTypeTmp;
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_INFO, "Begin check CDB of oracle database %s.", stDBInfo.strInstName.c_str());
    BuildCheckCDBScriptParam(stDBInfo, strParam);

#ifdef WIN32
    // windows下调用脚本
    iRet = SecureCom::SysExecScript(mp_string(WIN_ORACLE_CHECK_CDB), strParam, &vecResult);
    ClearString(strParam);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, "Check CDB of oracle database(%s) failed, iRet %d", stDBInfo.strInstName.c_str(), iRet);
        return iNewRet;
    }
#else
    // Oracle下需要切换到Oracle用户下，必须在root或者实例用户下执行
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLECHECKCDB, strParam, &vecResult);
    ClearString(strParam);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Check CDB of oracle database(%s) failed, iRet %d", stDBInfo.strInstName.c_str(), iRet);
        return iRet;
    }
#endif
    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "The result of get oracle archive log mode is empty.");
        return ERROR_COMMON_OPER_FAILED;
    }

    iCDBTypeTmp = atoi(vecResult.front().c_str());
    iCDBType = ((0 == iCDBTypeTmp) ? ORACLE_TYPE_NON_CDB : ORACLE_TYPE_CDB);
#ifdef WIN32
    if (iCDBType == ORACLE_TYPE_CDB) {
        COMMLOG(OS_LOG_ERROR, "Windows does't support oracle CDB.");
        return ERROR_COMMON_FUNC_UNIMPLEMENT;
    }
#endif

    COMMLOG(OS_LOG_INFO, "End check CDB of oracle database %s.", stDBInfo.strInstName.c_str());
    return iRet;
}

/* ------------------------------------------------------------
Description  : build the script parameter for start asm instance
Input        : stDBInfo -- the database structor
Output      : strParam -- the prameter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_void Oracle::BuildStartASMScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMUSERNAME) + stDBInfo.strASMUserName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMPASSWOD) + stDBInfo.strASMPassword + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMDISKGROUPS) + stDBInfo.strASMDiskGroup + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ACTION) + "0";
}

/* ------------------------------------------------------------
Description  : build the script parameter for stop asm instance
Input        : stDBInfo -- the database structor
Output      : strParam -- the prameter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_void Oracle::BuildStopASMScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMUSERNAME) + stDBInfo.strASMUserName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMPASSWOD) + stDBInfo.strASMPassword + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMDISKGROUPS) + stDBInfo.strASMDiskGroup + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ACTION) + "1";
}

/* ------------------------------------------------------------
Description  : build the script parameter for start oracle database
Input        : stDBInfo -- the database structor
Output      : strParam -- the prameter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_void Oracle::BuildStartOracleScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam)
{
    ostringstream oss;
    oss << stDBInfo.iIncludeArchLog;
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBNAME) + stDBInfo.strDBName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stDBInfo.strDBPassword + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ISASM) + stDBInfo.strIsASM + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMINSTANCE) + stDBInfo.strASMInstance + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMUSERNAME) + stDBInfo.strASMUserName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMPASSWOD) + stDBInfo.strASMPassword + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMDISKGROUPS) + stDBInfo.strASMDiskGroup + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stDBInfo.strOracleHome + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCIPRTPARAM_IS_INCLUDE_ARCH) + oss.str() + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ACTION) + "0";
}

/* ------------------------------------------------------------
Description  : build the script parameter for stop oracle database
Input        : stDBInfo -- the database structor
Output      : strParam -- the prameter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_void Oracle::BuildStopOracleScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBNAME) + stDBInfo.strDBName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stDBInfo.strDBPassword + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ISASM) + stDBInfo.strIsASM + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMINSTANCE) + stDBInfo.strASMInstance + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMUSERNAME) + stDBInfo.strASMUserName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMPASSWOD) + stDBInfo.strASMPassword + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMDISKGROUPS) + stDBInfo.strASMDiskGroup + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stDBInfo.strOracleHome + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ACTION) + "1";
}

mp_int32 Oracle::GetCluterInfo(Json::Value& clsInfo)
{
    mp_int32 iRet;
    mp_string strParam;
    vector<mp_string> vecResult;
    COMMLOG(OS_LOG_DEBUG, "Begin to query oracle cluster info.");
#ifdef WIN32
    static const mp_string KEYCLUSTERTYPE = "ClusterType";
    clsInfo[KEYCLUSTERTYPE] = 0;
    return MP_SUCCESS;
#else
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYORACLE_CLUSTERINFO, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Excute oracle cluster info script failed, iRet %d", iRet);
        return iRet;
    }
#endif
    iRet = OracleInfo::AnalyseClsInfoScriptRst(vecResult, clsInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Analyse cluster info result failed, iRet %d", iRet);
        return iRet;
    }
    return MP_SUCCESS;
}

#ifdef WIN32
DWORD WINAPI Oracle::CheckArchiveAreaThread(void* pThis)
#else
EXTER_ATTACK mp_void* Oracle::CheckArchiveAreaThread(void* pThis)
#endif
{
    Oracle* pOracle = static_cast<Oracle*>(pThis);
    if (!pOracle) {
        COMMLOG(OS_LOG_ERROR, "convert to Oracle failed.");
        CMPTHREAD_RETURN;
    }

    COMMLOG(OS_LOG_INFO, "Start CheckArchiveAreaThread thread.");
    while (!pOracle->GetFlagTExitCheckArchiveArea()) {
        pOracle->CheckArchiveArea();
        
        int nTimeout = 0;
        if (MP_SUCCESS != CConfigXmlParser::GetInstance().GetValueInt32(
            CFG_BACKUP_SECTION, CFG_ARCHIVE_THREAD_TIMEOUT, nTimeout)) {
            COMMLOG(OS_LOG_ERROR, "get Progress Report Interval failed, set Default value %d.", ARCHIVE_THREAD_TIMEOUT);
            nTimeout = ARCHIVE_THREAD_TIMEOUT;
        }
        CMpTime::DoSleep(nTimeout);
    }
    COMMLOG(OS_LOG_INFO, "Finish CheckArchiveAreaThread thread.");

    CMPTHREAD_RETURN;
}

void Oracle::CheckArchiveArea()
{
    mp_string strDBs;
    mp_string strArchThreshold;
    mp_string listenIp;
    mp_string listenPort;
    CIP::GetListenIPAndPort(listenIp, listenPort);

    std::vector<oracle_db_info_t> vecInfo = QueryOracleDbInfo();
    for (std::vector<oracle_db_info_t>::iterator iter = vecInfo.begin(); iter != vecInfo.end(); ++iter) {
        if (CheckArchiveThreshold(*iter) != MP_SUCCESS) {
            strDBs.append((*iter).strDBName).append(";");
            strArchThreshold = (*iter).strArchThreshold;
            break;
        }
    }

    alarm_param_t alarmParam;
    alarmParam.iAlarmID = "0x6403400001";
    AlarmHandle alarmHandle;
    if (strDBs.empty()) {
        alarmHandle.ClearAlarm(alarmParam);
    } else {
        strDBs = strDBs.substr(0, strDBs.size() - 1);
        alarmParam.strAlarmParam = listenIp + "," + strDBs + "," + strArchThreshold;
        if (alarmHandle.Alarm(alarmParam) != MP_SUCCESS) {
            CTrapSenderManager::CreateSender(A8000).SendAlarm(alarmParam);
        }
    }
}

std::vector<oracle_db_info_t> Oracle::QueryOracleDbInfo()
{
    std::vector<oracle_db_info_t> vecInfo;

    DbParamStream dps;
    DBReader readBuff;
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;

    std::ostringstream buff;
    buff << "select * from " << g_OracleDbInfo;
    mp_int32 iRet = CDB::GetInstance().QueryTable(buff.str(), dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query oracle db info failed, iRet %d.", iRet);
        return vecInfo;
    }

    mp_string strDecrypt;
    vecInfo.reserve(iRowCount);
    for (int i = 0; i < iRowCount; ++i) {
        oracle_db_info_t info;
        if (MP_SUCCESS != CConfigXmlParser::GetInstance().GetValueString(
            CFG_BACKUP_SECTION, CFG_ARCHIVE_THRESHOLD, info.strArchThreshold)) {
            COMMLOG(OS_LOG_ERROR, "get Progress Report Interval failed, set Default value %s.", ARCHIVE_THRESHOLD);
            info.strArchThreshold = ARCHIVE_THRESHOLD;
        }
        readBuff >> info.strDBName;
        readBuff >> info.strInstName;
        readBuff >> info.strDBUsername;
        readBuff >> strDecrypt;
        DecryptStr(strDecrypt, info.strDBPassword);
        readBuff >> info.strASMInstance;
        readBuff >> info.strASMUserName;
        readBuff >> strDecrypt;
        DecryptStr(strDecrypt, info.strASMPassword);
        vecInfo.emplace_back(info);
    }
    return vecInfo;
}

bool Oracle::GetFlagTExitCheckArchiveArea()
{
    return m_bTExitCheckArchiveArea;
}

void Oracle::SetFlagTExitCheckArchiveArea(bool bExit)
{
    m_bTExitCheckArchiveArea = bExit;
}