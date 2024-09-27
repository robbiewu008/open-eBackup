#include "plugins/oracle/OraclePlugin.h"
#include <sstream>
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "message/rest/interfaces.h"
#include "message/rest/message_process.h"
using namespace std;

REGISTER_PLUGIN(OraclePlugin);
OraclePlugin::OraclePlugin()
{
    REGISTER_ACTION(REST_ORACLE_QUERY_DB_INFO, REST_URL_METHOD_GET, &OraclePlugin::QueryInfo);
    REGISTER_ACTION(REST_ORACLE_QUERY_ASM, REST_URL_METHOD_GET, &OraclePlugin::QueryASMInstance);
    REGISTER_ACTION(REST_ORACLE_TEST, REST_URL_METHOD_PUT, &OraclePlugin::Test);
    REGISTER_ACTION(REST_ORACLE_QUERYTABLESPACE, REST_URL_METHOD_GET, &OraclePlugin::QueryTableSpace);
    REGISTER_ACTION(REST_ORACLE_QUERY_CLUSTERINFO, REST_URL_METHOD_GET, &OraclePlugin::QueryRACInfo);

    m_oracle.SetFlagTExitCheckArchiveArea(false);
    (mp_void) memset_s(&m_tid_CheckArchiveArea, sizeof(m_tid_CheckArchiveArea),
        0, sizeof(m_tid_CheckArchiveArea));
    if (CMpThread::Create(&m_tid_CheckArchiveArea, Oracle::CheckArchiveAreaThread, this) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init CheckArchiveAreaThread failed.");
    }
}

OraclePlugin::~OraclePlugin()
{
    m_oracle.SetFlagTExitCheckArchiveArea(true);
    if (m_tid_CheckArchiveArea.os_id != 0) {
        CMpThread::WaitForEnd(&m_tid_CheckArchiveArea, NULL);
    }
}
/* ------------------------------------------------------------
Description  : Oracle组件的统一接口入口，在此分发调用具体的接口
Input        : req -- 输入信息
Output       : rsp -- 返回信息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 OraclePlugin::DoAction(CRequestMsg& req, CResponseMsg& rsp)
{
    DO_ACTION(OraclePlugin, req, rsp);
}

/* ------------------------------------------------------------
Description  : 查询oracle数据库信息入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OraclePlugin::QueryInfo(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    oracle_db_info_t stDBInfo;

    stDBInfo.strInstName = req.GetURL().GetSpecialQueryParam(RESPOND_ORACLE_PARAM_INSTNAME);
    stDBInfo.strOracleHome = req.GetURL().GetSpecialQueryParam(RESPOND_ORACLE_PARAM_ORACLE_HOME);

    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strInstName));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strOracleHome));
    
    // if InstName is not empty, turn to check cdb
    if (!stDBInfo.strInstName.empty()) {
        COMMLOG(OS_LOG_INFO, "Begin to check oracle is CDB...");
        // get username and password in http head
        iRet = GetDBAuthParam(req, stDBInfo);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
            return iRet;
        }
        mp_int32 iCDBType = ORACLE_TYPE_NON_CDB;
        iRet = m_oracle.CheckCDB(stDBInfo, iCDBType);
        Json::Value& jValue = rsp.GetJsonValueRef();
        if (iRet == MP_SUCCESS) {
            COMMLOG(OS_LOG_INFO, "Check oracle CDB succ.");
            jValue[RESPOND_ORACLE_PARAM_CDBTYPE] = iCDBType;
            return MP_SUCCESS;
        }
        COMMLOG(OS_LOG_ERROR, "Error occur in checking CDB.");
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Begin query oracle info...");
    mp_string strVer;
    mp_string strHome;
    list<oracle_inst_info_t> lstOracleInstInfo;
    iRet = m_oracle.GetDBInfo(lstOracleInstInfo, strVer, strHome);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get oracle info failed, iRet %d.", iRet);
        return iRet;
    }
    GetDBInfo2Rsp(lstOracleInstInfo, rsp);
    COMMLOG(OS_LOG_DEBUG, "Query oracle info succ.");
    return MP_SUCCESS;
}

mp_void OraclePlugin::GetDBInfo2Rsp(list<oracle_inst_info_t>& lstOracleInstInfo, CResponseMsg& rsp)
{
    Json::Value& val = rsp.GetJsonValueRef();
    list<oracle_inst_info_t>::iterator iter;
    for (iter = lstOracleInstInfo.begin(); iter != lstOracleInstInfo.end(); ++iter) {
        Json::Value jValue;
        jValue[RESPOND_ORACLE_PARAM_INSTNAME] = iter->strInstName;
        jValue[RESPOND_ORACLE_PARAM_DBNAME] = iter->strDBName;
        jValue[RESPOND_ORACLE_PARAM_VERSION] = iter->strVersion;
        jValue[RESPOND_ORACLE_PARAM_STATE] = iter->iState;
        jValue[RESPOND_ORACLE_PARAM_ISASMDB] = iter->iIsASMDB;
        jValue[RESPOND_ORACLE_PARAM_AUTHTYPE] = iter->authType;
        jValue[RESPOND_ORACLE_PARAM_DBROLE] = iter->dbRole;
        jValue[RESPOND_ORACLE_PARAM_ORACLE_HOME] = iter->strOracleHome;
        jValue[RESPOND_ORACLE_PARAM_DBUUID] = iter->strDBUUID;
        val.append(std::move(jValue));
    }
}

/* ------------------------------------------------------------
Description  : 获取下发的oracle认证信息
Input        : req -- rest请求结构体
Output       :  stDBInfo -- 数据库信息，获取信息放到此结构体
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OraclePlugin::GetPDBAuthParam(CRequestMsg& req, oracle_pdb_req_info_t& stPDBInfo)
{
    mp_string pHeadStr = "";
    pHeadStr = req.GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBUSERNAME);
    stPDBInfo.strDBUsername = CMpString::Trim(pHeadStr);

    pHeadStr = req.GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBPASSWORD);
    stPDBInfo.strDBPassword = CMpString::Trim(pHeadStr);

    if (!stPDBInfo.strDBPassword.empty() && stPDBInfo.strDBUsername.empty()) {
        COMMLOG(OS_LOG_ERROR, "Check parameter failed, strDBPassword is null and strDBUsername is not null.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    return MP_SUCCESS;
}

Json::Value OraclePlugin::ConvertLunInfo2Json(const oracle_lun_info_t& lunInfo)
{
    COMMLOG(OS_LOG_DEBUG,
        "lunid:%s, uudid:%s, arraysn:%s, wwn:%s, vgname:%s, devicename:%s,"
        " mainType:%d,subType:%d,devicepath=%s, udevrules:%s, lba=%s, asmdg=%s.",
        lunInfo.strLUNId.c_str(),
        lunInfo.strUUID.c_str(),
        lunInfo.strArraySn.c_str(),
        lunInfo.strWWN.c_str(),
        lunInfo.strVgName.c_str(),
        lunInfo.strDeviceName.c_str(),
        lunInfo.iStorMainType,
        lunInfo.iStorSubType,
        lunInfo.strDevicePath.c_str(),
        lunInfo.strUDEVRules.c_str(),
        lunInfo.strLBA.c_str(),
        lunInfo.strASMDiskGroup.c_str());

    Json::Value jValue;
    jValue[RESPOND_ORACLE_LUNID] = lunInfo.strLUNId;
    jValue[RESPOND_ORACLE_UUID] = lunInfo.strUUID;
    jValue[RESPOND_ORACLE_ARRAYSN] = lunInfo.strArraySn;
    jValue[RESPOND_ORACLE_WWN] = lunInfo.strWWN;
    jValue[RESPOND_ORACLE_VGNAME] = lunInfo.strVgName;
    jValue[RESPOND_ORACLE_DEVICENAME] = lunInfo.strDeviceName;
    jValue[RESPOND_ORACLE_PVNAME] = lunInfo.strPvName;
    jValue[RESPOND_ORACLE_STORMAINTYPE] = lunInfo.iStorMainType;
    jValue[RESPOND_ORACLE_STORSUBTYPE] = lunInfo.iStorSubType;
    jValue[RESPOND_ORACLE_DEVICEPATH] = lunInfo.strDevicePath;
    jValue[RESPOND_ORACLE_UDEVRULES] = lunInfo.strUDEVRules;
    jValue[RESPOND_ORACLE_LBA] = lunInfo.strLBA;
    jValue[RESPOND_ORACLE_ASMDISKGROUP] = lunInfo.strASMDiskGroup;
    return jValue;
}

mp_int32 OraclePlugin::GetRestDBnput(CRequestMsg& req, mp_int32& iISASM, oracle_db_info_t& stDBInfo, mp_bool isStart)
{
    ostringstream oss;
    const Json::Value& sendParam = req.GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_INSTNAME, stDBInfo.strInstName);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strInstName, 0, ORACLE_PLUGIN_MAX_DBINSTANCENAME));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strInstName));

    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_DBNAME, stDBInfo.strDBName);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strDBName, 0, ORACLE_PLUGIN_MAX_DBNAME));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strDBName));

    if (sendParam.isMember(RESPOND_ORACLE_PARAM_ASMINSTNAME)) {
        GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_ASMINSTNAME, stDBInfo.strASMInstance);
        CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strASMInstance, 0,
                                          ORACLE_PLUGIN_MAX_DBINSTANCENAME, mp_string("+")));
        CHECK_FAIL_EX(CheckParamValid(stDBInfo.strASMInstance));
    }
    if (sendParam.isMember(RESPOND_ORACLE_ASMDISKGROUPS)) {
        GET_JSON_STRING(sendParam, RESPOND_ORACLE_ASMDISKGROUPS, stDBInfo.strASMDiskGroup);
        CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strASMDiskGroup, 0, ORACLE_PLUGIN_MAX_STRING));
        CHECK_FAIL_EX(CheckParamValid(stDBInfo.strASMDiskGroup));
    }
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_ORACLE_HOME, stDBInfo.strOracleHome);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strOracleHome, 0, ORACLE_PLUGIN_MAX_STRING));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strOracleHome));
    GET_JSON_INT32(sendParam, RESPOND_ORACLE_ISASM, iISASM);
    CHECK_FAIL_EX(CheckParamInteger32(iISASM, 0, 1));
    oss << iISASM;
    stDBInfo.strIsASM = oss.str();

    if (isStart == MP_TRUE) {
        GET_JSON_INT32(sendParam, REST_PARAM_ORACLE_IS_INCLUDE_ARCH, stDBInfo.iIncludeArchLog);
        CHECK_FAIL_EX(CheckParamInteger32(stDBInfo.iIncludeArchLog, 0, 1));
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 测试连接oracle数据库入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OraclePlugin::Test(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) rsp;
    mp_int32 iRet;
    oracle_db_info_t stDBInfo;

    COMMLOG(OS_LOG_INFO, "Begin test oracle database info.");
    const Json::Value& sendParam = req.GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_INSTNAME, stDBInfo.strInstName);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strInstName, 0, ORACLE_PLUGIN_MAX_DBINSTANCENAME));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strInstName));

    GET_JSON_STRING_OPTION(sendParam, RESPOND_ORACLE_PARAM_DBNAME, stDBInfo.strDBName);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strDBName, 0, ORACLE_PLUGIN_MAX_DBNAME));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strDBName));
    
    GET_JSON_STRING_OPTION(sendParam, RESPOND_ORACLE_PARAM_ORACLE_HOME, stDBInfo.strOracleHome);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strOracleHome, 0, ORACLE_PLUGIN_MAX_STRING));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strOracleHome));

    // 获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stDBInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG,
        "instname:%s, dbname:%s, username:%s, ASM username:%s.",
        stDBInfo.strInstName.c_str(),
        stDBInfo.strDBName.c_str(),
        stDBInfo.strDBUsername.c_str(),
        stDBInfo.strASMUserName.c_str());

    iRet = m_oracle.Test(stDBInfo);
    ClearString(stDBInfo.strASMPassword);
    ClearString(stDBInfo.strDBPassword);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Test oracle database (%s-%s) failed, iRet %d.",
            stDBInfo.strInstName.c_str(),
            stDBInfo.strDBName.c_str(),
            iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Test oracle database succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : query oracle database tablespace list
Input        : req -- rest request message
                rsp -- rest response message
Return       : MP_SUCCESS -- successfully
               非MP_SUCCESS -- failed
------------------------------------------------------------- */
mp_int32 OraclePlugin::QueryTableSpace(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) rsp;
    oracle_db_info_t stDBInfo;
    COMMLOG(OS_LOG_INFO, "Begin to query oracle database tablespace info.");
    CRequestURL& vrequrl = req.GetURL();
    map<mp_string, mp_string> &vreqal = vrequrl.GetQueryParam();
    for (map<mp_string, mp_string>::iterator iter = vreqal.begin(); iter != vreqal.end(); ++iter) {
        if (iter->first == RESPOND_ORACLE_PARAM_INSTNAME) {
            stDBInfo.strInstName = iter->second;
        }
        if (iter->first == RESPOND_ORACLE_PARAM_DBNAME) {
            stDBInfo.strDBName = iter->second;
        }
        if (iter->first == RESPOND_ORACLE_PARAM_ORACLE_HOME) {
            stDBInfo.strOracleHome = iter->second;
        }
    }
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strInstName));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strDBName));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strOracleHome));
    // 获取消息头中的用户名密码
    mp_int32 iRet = GetDBAuthParam(req, stDBInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, "instname:%s, dbname:%s, username:%s.",
        stDBInfo.strInstName.c_str(),
        stDBInfo.strDBName.c_str(),
        stDBInfo.strDBUsername.c_str());
    vector<mp_string> tslist;
    iRet = m_oracle.QueryTableSpace(stDBInfo, tslist);
    ClearString(stDBInfo.strASMPassword);
    ClearString(stDBInfo.strDBPassword);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Query oracle database tablespace(%s-%s) failed, iRet %d.",
            stDBInfo.strInstName.c_str(),
            stDBInfo.strDBName.c_str(),
            iRet);
        return iRet;
    }
    QueryTableSpaceRsp(rsp, tslist, stDBInfo);
    COMMLOG(OS_LOG_INFO, "Query oracle database tablespace succ.");
    return MP_SUCCESS;
}

mp_void OraclePlugin::QueryTableSpaceRsp(CResponseMsg &rsp, vector<mp_string> &tslist,
    const oracle_db_info_t &stDBInfo)
{
    static const mp_int32 tableSpaceFldNum = 5;
    static const mp_int32 dbuuidFldNum = 2;
    Json::Value& jValue = rsp.GetJsonValueRef();
    for (vector<mp_string>::iterator iter = tslist.begin(); iter != tslist.end(); ++iter) {
        vector<mp_string> tsInfo;
        CMpString::StrSplit(tsInfo, *iter, CHAR_SEMICOLON);
        if (tsInfo.size() == tableSpaceFldNum) {
            Json::Value tsValue;
            mp_int32 idx = 1;
            tsValue[RESPOND_ORACLE_TS_CONNAME] = tsInfo[idx++];
            tsValue[RESPOND_ORACLE_TS_TSNAME] = tsInfo[idx++];
            tsValue[RESPOND_ORACLE_TS_FILENO] = tsInfo[idx++];
            tsValue[RESPOND_ORACLE_TS_FILENAME] = tsInfo[idx++];
            jValue["TableSpace"].append(std::move(tsValue));
        }
        if (tsInfo.size() == dbuuidFldNum) {
            mp_int32 idx = 1;
            jValue[RESPOND_ORACLE_PARAM_DBUUID] = tsInfo[idx++];
        }
    }
}

/* ------------------------------------------------------------
Description  : query oracle asm instance information
Input        : req -- rest request message
                rsp -- rest response message
Return       : MP_SUCCESS -- successfully
               非MP_SUCCESS -- failed
------------------------------------------------------------- */
mp_int32 OraclePlugin::QueryASMInstance(CRequestMsg& req, CResponseMsg& rsp)
{
    static const mp_int32 tableSpaceFldNum = 3;
    (mp_void) rsp;

    COMMLOG(OS_LOG_DEBUG, "Begin to query oracle asm instance info.");
    mp_string asmInst;
    mp_int32 iRet = m_oracle.QueryASMInstance(asmInst);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query oracle asm instance failed, iRet %d.", iRet);
        return iRet;
    }

    if (!asmInst.empty()) {
        Json::Value& jValue = rsp.GetJsonValueRef();
        vector<mp_string> asmInfos;
        CMpString::StrSplit(asmInfos, asmInst, CHAR_SEMICOLON);

        if (asmInfos.size() != tableSpaceFldNum) {
            COMMLOG(OS_LOG_ERROR, "Analyse oracle asm(%s) failed.", asmInst);
            return MP_FAILED;
        }

        Json::Value tsValue;
        mp_int32 idx = 0;
        tsValue[RESPOND_ORACLE_PARAM_INSTNAME] = asmInfos[idx++];
        tsValue[RESPOND_ORACLE_PARAM_AUTHTYPE] = asmInfos[idx++];
        tsValue[RESPOND_ORACLE_PARAM_ISCLUSTER] = asmInfos[idx++];
        jValue.append(std::move(tsValue));
    }

    COMMLOG(OS_LOG_DEBUG, "Query oracle asm infor succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 获取下发的oracle认证信息
Input        : req -- rest请求结构体
Output       :  stDBInfo -- 数据库信息，获取信息放到此结构体
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OraclePlugin::GetDBAuthParam(CRequestMsg& req, oracle_db_info_t& stDBInfo)
{
    mp_string pHeadStr = req.GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBUSERNAME);
    stDBInfo.strDBUsername = CMpString::Trim(pHeadStr);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strDBUsername, 0, ORACLE_PLUGIN_MAX_NSERNAME));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strDBUsername));

    pHeadStr = req.GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBPASSWORD);
    stDBInfo.strDBPassword = CMpString::Trim(pHeadStr);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strDBPassword, 0, ORACLE_PLUGIN_MAX_STRING));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strDBPassword));

    pHeadStr = req.GetHttpReq().GetHeadNoCheck(HTTPPARAM_ASMSERNAME);
    stDBInfo.strASMUserName = CMpString::Trim(pHeadStr);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strASMUserName, 0, ORACLE_PLUGIN_MAX_NSERNAME));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strASMUserName));

    pHeadStr = req.GetHttpReq().GetHeadNoCheck(HTTPPARAM_ASMPASSWORD);
    stDBInfo.strASMPassword = CMpString::Trim(pHeadStr);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strASMPassword, 0, ORACLE_PLUGIN_MAX_STRING));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strASMPassword));

    mp_bool bParamCheck = (!stDBInfo.strASMPassword.empty() && stDBInfo.strASMUserName.empty()) ||
                          (!stDBInfo.strDBPassword.empty() && stDBInfo.strDBUsername.empty());
    if (bParamCheck) {
        COMMLOG(OS_LOG_ERROR,
            "Check parameter failed, strASMPassword is null"
            " and strASMUserName is not null, strDBPassword is null and strDBUsername is not null.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    return MP_SUCCESS;
}

mp_int32 OraclePlugin::QueryRACInfo(CRequestMsg& req, CResponseMsg& rsp)
{
    Json::Value clsInfo;
    mp_int32 iRet = m_oracle.GetCluterInfo(clsInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get Cluter Info failed, iRet %d", iRet);
        return iRet;
    }
    Json::Value& val = rsp.GetJsonValueRef();
    val = std::move(clsInfo);

    return MP_SUCCESS;
}