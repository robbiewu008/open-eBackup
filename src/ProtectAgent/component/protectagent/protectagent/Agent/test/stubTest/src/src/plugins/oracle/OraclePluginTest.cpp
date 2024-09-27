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
#include "plugins/oracle/OraclePluginTest.h"
#include "plugins/oracle/OraclePlugin.h"
#include <vector>
#include <list>
#include "common/Utils.h"
using namespace std;

namespace {
mp_void LogTest() {}
#define DoGetJsonStringTest() do { \
    stub11.set(ADDR(CLogger, Log), LogTest); \
    stub11.set(ADDR(CMpThread, Create), CMpThread_Create_stub);\
} while (0)
static int iOracleCounter = 0;
static int iOracleRet = 0;
static int iOracleCDBCounter = 0;
static int g_iSybaseJsonStringCounter = 0;
mp_int32 GetJsonInt32Test(const Json::Value& jsValue, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}
mp_int32 GetRestDBnputS(CRequestMsg& req, mp_int32& iISASM, oracle_db_info_t& stDBInfo, mp_bool isStart)
{
    return MP_SUCCESS;
}
mp_void StrSplitTest(vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep)
{
    vecTokens.push_back("aaa");
}
mp_int32 QueryASMInstanceTest(mp_string& asmInst)
{
    return MP_SUCCESS;
}
mp_int32 QueryTableSpaceTest(const oracle_db_info_t& stDBInfo, std::vector<mp_string>& tablespaces)
{
    tablespaces.push_back("aaa");
    return MP_SUCCESS;
}
static bool  stub_return_bool_StopResouceGroup(mp_string keyStorage)
{
    return MP_TRUE;
}
mp_int32 GetRestDBnputTest(CRequestMsg& req, mp_int32& iISASM, oracle_db_info_t& stDBInfo, mp_bool isStart)
{
    return MP_SUCCESS;
}
static mp_int32 SybaseStubJsonString(const Json::Value& jsValue, const mp_string& strKey, mp_string& strValue)
{
    if (g_iSybaseJsonStringCounter++ == 0)
    {
        strValue = "";
        return MP_SUCCESS;
    }
    strValue = "test123";
    
    return MP_SUCCESS;
}
mp_int32 stub_return_ret_xxx(mp_void)
{
    static mp_int32 iCounter = 0;
    if (iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}
mp_int32 stubGetPDBInfo(mp_void *ptr, oracle_pdb_req_info_t &stPdbReqInfo, vector<oracle_pdb_rsp_info_t> &vecOraclePdbInfo)
{
    static int iCounter = 0;
    oracle_pdb_rsp_info_t stOraclePdbInfo;
    if (iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        stOraclePdbInfo.iConID = 0;
        stOraclePdbInfo.iStatus = 0;
        stOraclePdbInfo.strPdbName = "aaa";
        vecOraclePdbInfo.push_back(stOraclePdbInfo);
        return MP_SUCCESS;
    }
}

mp_string StubGetSpecialQueryParam(const mp_string& strKey)
{
    static int iCounter = 0;
    if (iCounter++ == 0)
    {
        mp_string strParam;
        return strParam;
    }
    else
    {   
        mp_string strParam("123");
        return strParam;
    }

}
static mp_int32 stubReturnRet(mp_void)
{

    return MP_SUCCESS;

}
mp_int32 stubStartPluginDB(mp_void *ptr, oracle_pdb_req_info_t &stPdbReqInfo)
{
    static int iCounter = 0;
    if (iCounter++ == 0)
    { 
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

static std::map<mp_string, mp_string>& stubGetQueryParam(mp_void)
{
    static std::map<mp_string, mp_string> urlMapObj;
    
    urlMapObj.insert(map<mp_string, mp_string>::value_type("resourceGroup", "123"));
    urlMapObj.insert(map<mp_string, mp_string>::value_type("clusterType", "oracleCLuster"));
    urlMapObj.insert(map<mp_string, mp_string>::value_type("instName", "oracleInstance"));
    urlMapObj.insert(map<mp_string, mp_string>::value_type("dbName", "agent"));
    urlMapObj.insert(map<mp_string, mp_string>::value_type("clusterType", "666"));
    urlMapObj.insert(map<mp_string, mp_string>::value_type("appType", "555"));
    
    return urlMapObj;
}

static mp_string stubGetHeadNoCheck(mp_void *ptr, const mp_string& name)
{                                                               
    static int iCounter = 0;
    iCounter++;
    switch (iCounter)
    {   //1. 用户名为空密码不空 -- ERROR_COMMON_INVALID_PARAM
        case 1: return "";
        case 2: return "123";
        //2. 用户名为空密码为空 -- success
        case 3: return "";
        case 4: return "";
        //3. 用户名不为空密码为空 -- sucess
        case 5: return "123";
        case 6: return "";
        //4. 都不空 -- success
        case 7: return "123";
        case 8: return "123";
        
        default: return "123";
    }
}

static mp_string stubPDBGetHeadNoCheck(mp_void *ptr, const mp_string& name)
{
    static int iCounter = 0;
    if (iCounter++ == 0)
    {
        return "";
    }
    else
    {
        return "123";
    }
}

mp_int32 stubGetDBLUNInfo(mp_void *ptr, oracle_db_info_t &stDBInfo, vector<oracle_lun_info_t> &vecLUNInfos)
{
    static int iCounter = 0;
    if (iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        vecLUNInfos.push_back(oracle_lun_info_t());
        return MP_SUCCESS;
    }
}
static mp_int32 stubGetDBInfo(void *ptr, list<oracle_inst_info_t> &lstOracleInstInfo)
{
    if (iOracleCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        lstOracleInstInfo.push_back(oracle_inst_info_t());
        lstOracleInstInfo.push_back(oracle_inst_info_t());
        lstOracleInstInfo.push_back(oracle_inst_info_t());
        lstOracleInstInfo.push_back(oracle_inst_info_t());
        lstOracleInstInfo.push_back(oracle_inst_info_t());
        lstOracleInstInfo.push_back(oracle_inst_info_t());
        return MP_SUCCESS;
    }
}
static mp_int32 stubGetDBAuthParamFalse(mp_void)
{
    return 0x4003291A;
}
static mp_int32 stubCheckCDB(void *ptr, oracle_db_info_t &stDBInfo, mp_int32& iCDBType)
{
    if (iOracleCDBCounter == 0)
    {
        iCDBType=0;
        iOracleCDBCounter++;
        return MP_SUCCESS;
    }
    else if (iOracleCDBCounter == 1)
    {
        iCDBType=1;
        iOracleCDBCounter++;
        return MP_SUCCESS;
    }
    else
    {
        return ERROR_SCRIPT_ORACLE_ASM_INSTANCE_NOSTART;
    }
}
static mp_int32 stubGetDBAuthParamTrue(mp_void)
{
    return MP_SUCCESS;
}

mp_int32 GetDBAuthParamTest(CRequestMsg& req, oracle_db_info_t& stDBInfo)
{
    return MP_SUCCESS;
}
}

mp_int32 CMpThread_Create_stub(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_SUCCESS;
}

TEST_F(COraclePluginTest, QueryTableSpaceTest)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    OraclePlugin plugObj;
    // stub.set(ADDR(CRequestURL, GetQueryParam), stubGetQueryParam);
    // stub.set(ADDR(OraclePlugin, GetDBAuthParam), GetDBAuthParamTest);
    // stub.reset(ADDR(OraclePlugin, GetDBAuthParam));
    // plugObj.QueryTableSpace(req, rsp);
    // plugObj.QueryTableSpace(req, rsp);
    // stub.set(ADDR(Oracle, QueryTableSpace), QueryTableSpaceTest);
    // plugObj.QueryTableSpace(req, rsp);
}

TEST_F(COraclePluginTest, QueryInfo)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    OraclePlugin plugObj;
    CRequestURL testurl;
    testurl.m_procURL = "test";
    testurl.m_oriURL = "test";
    testurl.m_id = "test";
    testurl.m_queryParam["instName"] = "cdb12c";

    stub.set(ADDR(Oracle, GetDBInfo), stubGetDBInfo);
    
    iRet = plugObj.QueryInfo(req, rsp); 
        
    iRet = plugObj.QueryInfo(req, rsp); 
    stub.set(ADDR(Oracle, CheckCDB), stubCheckCDB);
    
    stub.set(ADDR(OraclePlugin, GetDBAuthParam), stubGetDBAuthParamTrue);

    req.m_url = testurl;
    Json::Value jvalue;
    jvalue["test"] = "test";
    rsp.m_msgJsonData = jvalue;
    iRet = plugObj.QueryInfo(req, rsp);

    iRet = plugObj.QueryInfo(req, rsp);

    iRet = plugObj.QueryInfo(req, rsp);

    stub.set(ADDR(OraclePlugin, GetDBAuthParam), stubGetDBAuthParamFalse);
    iRet = plugObj.QueryInfo(req, rsp);
}


TEST_F(COraclePluginTest, TestTest)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    OraclePlugin plugObj;
    iOracleRet= 0;
    iOracleCounter = 0;
    
    stub.set(ADDR(CJsonUtils, GetJsonString), SybaseStubJsonString);
    stub.set(ADDR(OraclePlugin, GetDBAuthParam), stubReturnRet);
    iRet = plugObj.Test(req, rsp);

    iRet = plugObj.Test(req, rsp);

    iRet = plugObj.Test(req, rsp);

    stub.set(ADDR(Oracle, Test), stub_return_ret);
    iRet = plugObj.Test(req, rsp);
}

TEST_F(COraclePluginTest, GetDBAuthParam)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    CRequestMsg req;
    CResponseMsg rsp;
    oracle_db_info_t stDBInfo;
    mp_int32 iRet = MP_SUCCESS;
    OraclePlugin plugObj;


    stub.set(ADDR(CHttpRequest, GetHeadNoCheck), stubGetHeadNoCheck);
    stub.set(ADDR(OraclePlugin, GetDBAuthParam), stubReturnRet);

    iRet = plugObj.GetDBAuthParam(req, stDBInfo);
    
    iRet = plugObj.GetDBAuthParam(req, stDBInfo);
}

TEST_F(COraclePluginTest, GetPDBAuthParam)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    CRequestMsg req;
    CResponseMsg rsp;
    oracle_pdb_req_info_t stPDBInfo;
    mp_int32 iRet = MP_SUCCESS;
    OraclePlugin plugObj;

    stub.set(ADDR(CHttpRequest, GetHeadNoCheck), stubPDBGetHeadNoCheck);

    iRet = plugObj.GetPDBAuthParam(req, stPDBInfo);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
    
    iRet = plugObj.GetPDBAuthParam(req, stPDBInfo);
    EXPECT_EQ(MP_SUCCESS, iRet);
}


