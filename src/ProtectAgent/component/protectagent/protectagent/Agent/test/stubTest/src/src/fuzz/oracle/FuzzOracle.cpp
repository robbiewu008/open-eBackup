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
#include "fuzz/oracle/FuzzOracle.h"
#include "secodeFuzz.h"
#include "common/Types.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "plugins/oracle/OraclePlugin.h"
#ifndef WIN32
#include <sys/time.h>
#endif
#include "securecom/RootCaller.h"
using namespace std;

namespace {
char dt_fuzz_string_init[] = "1234";
int dt_fuzz_int_init = 1;
char dt_fuzz_ipv4_init[4] = {0,0,0,0};
int g_fuzz_index = 0;

mp_string DT_GetString_ParamValid(mp_int32 maxLen)
{
    mp_string strTmp = DT_SetGetString(&g_Element[g_fuzz_index++], 5, maxLen, dt_fuzz_string_init);
    if ((CheckParamValid(strTmp) != MP_SUCCESS)) {
        strTmp = "param";
    }
    return strTmp;
}

static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_void StubDoSleepTest100ms(mp_uint32 ms)
{
#ifdef WIN32
    Sleep(100);
#else
    struct timeval stTimeOut;
    const mp_int32 timeUnitChange = 100000;
    stTimeOut.tv_sec  = 100 / timeUnitChange;
    stTimeOut.tv_usec = (100 % timeUnitChange) * timeUnitChange;
    (mp_void)select(0, NULL, NULL, NULL, &stTimeOut);
#endif
}

mp_int32 CMpThread_Create_stub(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_SUCCESS;
}

mp_int32 StubSUCCESS(mp_void* pThis)
{
    return MP_SUCCESS;
}

mp_string StubGetSpecialQueryParam(mp_void* pThis, const mp_string& strKey)
{
    mp_string strTmp;
    if (strKey == RESPOND_ORACLE_PARAM_INSTNAME) {
        strTmp = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
    } else if (strKey == RESPOND_ORACLE_PARAM_ORACLE_HOME) {
        strTmp = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
    }
    return strTmp;
}

mp_string StubGetHeadNoCheck(mp_void* pThis, const mp_string& name)
{
    mp_string strTmp;
    if (name == HTTPPARAM_DBUSERNAME) {
        strTmp = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
    } else if (name == HTTPPARAM_DBPASSWORD) {
        strTmp = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
    } else if (name == HTTPPARAM_ASMSERNAME) {
        strTmp = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
    } else if (name == HTTPPARAM_ASMPASSWORD) {
        strTmp = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
    }
    if (strTmp.empty()) {
        strTmp = "empty";
    }
    return strTmp;
}

static std::map<mp_string, mp_string>&  StubGetQueryParam(mp_void* pThis)
{
    static std::map<mp_string, mp_string> urlMapObj;
    urlMapObj.clear();
    urlMapObj.insert(map<mp_string, mp_string>::value_type(RESPOND_ORACLE_PARAM_INSTNAME,
        DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME)));
    urlMapObj.insert(map<mp_string, mp_string>::value_type(RESPOND_ORACLE_PARAM_DBNAME,
        DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBNAME)));
    urlMapObj.insert(map<mp_string, mp_string>::value_type(RESPOND_ORACLE_PARAM_ORACLE_HOME,
        DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING)));
    return urlMapObj;
}

}

TEST_F(FuzzOracle, QueryInfo)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    m_stub.set(ADDR(CRequestURL, GetSpecialQueryParam), StubGetSpecialQueryParam);
    m_stub.set(ADDR(CHttpRequest, GetHeadNoCheck), StubGetHeadNoCheck);
    m_stub.set(ADDR(Oracle, CheckCDB), StubSUCCESS);
    m_stub.set(ADDR(Oracle, GetDBInfo), StubSUCCESS);
    OraclePlugin plugin;
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzOracle_QueryInfo";
    DT_FUZZ_START(0, 100000, strDtFuzzName ,0)
    {
        g_fuzz_index = 0;
        CRequestMsg req;
        CResponseMsg rsp;
        EXPECT_EQ(MP_SUCCESS, plugin.QueryInfo(req, rsp));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzOracle, Test)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    m_stub.set(ADDR(CHttpRequest, GetHeadNoCheck), StubGetHeadNoCheck);
    m_stub.set(ADDR(Oracle, Test), StubSUCCESS);
    OraclePlugin plugin;
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzOracle_Test";
    DT_FUZZ_START(0, 100000, strDtFuzzName ,0)
    {
        g_fuzz_index = 0;
        Json::Value jBody;
        jBody[RESPOND_ORACLE_PARAM_INSTNAME] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jBody[RESPOND_ORACLE_PARAM_DBNAME] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBNAME);
        jBody[RESPOND_ORACLE_PARAM_ORACLE_HOME] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        CRequestMsg req;
        req.m_msgBody.SetJsonValue(jBody);
        CResponseMsg rsp;
        EXPECT_EQ(MP_SUCCESS, plugin.Test(req, rsp));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzOracle, QueryTableSpace)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    m_stub.set(ADDR(CHttpRequest, GetHeadNoCheck), StubGetHeadNoCheck);
    m_stub.set(ADDR(CRequestURL, GetQueryParam), StubGetQueryParam);
    m_stub.set(ADDR(Oracle, QueryTableSpace), StubSUCCESS);
    OraclePlugin plugin;
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzOracle_QueryTableSpace";
    DT_FUZZ_START(0, 100000, strDtFuzzName ,0)
    {
        g_fuzz_index = 0;
        CRequestMsg req;
        CResponseMsg rsp;
        EXPECT_EQ(MP_SUCCESS, plugin.QueryTableSpace(req, rsp));
    }
    DT_FUZZ_END()
}
