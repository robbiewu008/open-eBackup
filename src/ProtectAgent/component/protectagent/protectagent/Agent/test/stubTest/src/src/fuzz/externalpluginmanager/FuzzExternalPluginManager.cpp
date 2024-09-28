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
#include "fuzz/externalpluginmanager/FuzzExternalPluginManager.h"
#include "secodeFuzz.h"
#include "Cmd.h"
#include "plugins/appprotect/AppProtectPlugin.h"
#include "common/Uuid.h"
#include "pluginfx/ExternalPluginManager.h"
#include "common/Types.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "host/host.h"
#ifndef WIN32
#include <sys/time.h>
#endif
#include "securecom/RootCaller.h"
using namespace std;

namespace {
const mp_int32 EXTERNAL_PLUGIN_MANAGER_MAX_STRING = 512;
const mp_int32 EXTERNAL_PLUGIN_MANAGER_MAX_APPTYPE_LEN = 32;
const mp_int32 EXTERNAL_PLUGIN_MANAGER_DTFUZZ_MAX_ID_LEN = 128;
const mp_int32 EXTERNAL_PLUGIN_MANAGER_DTFUZZ_MIN_ID_LEN = 32;
const mp_int32 EXTERNAL_PLUGIN_MANAGER_DTFUZZ_MIN_ID_NUMS = 1;
const mp_int32 EXTERNAL_PLUGIN_MANAGER_DTFUZZ_MAX_ID_NUMS = 4;
const mp_string EXTERNAL_PLUGIN_MANAGER_DTFUZZ_NOTIFY_APPTYPE = "appType";
const mp_string EXTERNAL_PLUGIN_MANAGER_DTFUZZ_NOTIFY_DMEIPLISTS = "dmeIpList";

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
    const mp_int32 timeUnitChange = 1000;
    stTimeOut.tv_sec  = 100 / timeUnitChange;
    stTimeOut.tv_usec = (100 % timeUnitChange) * timeUnitChange;
    (mp_void)select(0, NULL, NULL, NULL, &stTimeOut);
#endif
}

mp_int32 CMpThread_Create_stub(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_SUCCESS;
}

mp_int32 StubSUCCESS(mp_string& strSN)
{
    return MP_SUCCESS;
}
mp_int32 StubGetTaskIdFromUrl(mp_string &url)
{
    return MP_SUCCESS;
}
}

TEST_F(FuzzExternalPluginManager, QueryPluginResourceTest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(CHost, GetHostSN), StubSUCCESS);
    ExternalPluginManager plugMngr;
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("QueryPluginResource_Test")
    {
        char strInit[] = "1234";
        mp_string strAppType;
        strAppType = DT_SetGetString(&g_Element[0], 5, EXTERNAL_PLUGIN_MANAGER_MAX_APPTYPE_LEN, strInit);
        CRequestMsg requestMsg;
        CResponseMsg responseMsg;
        EXPECT_EQ(MP_FAILED, plugMngr.QueryPluginResource(strAppType, requestMsg, responseMsg));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzExternalPluginManager, QueryPluginDetailTest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    ExternalPluginManager plugMngr;
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("QueryPluginDetail_Test")
    {
        char strInit[] = "1234";
        mp_string strAppType;
        strAppType = DT_SetGetString(&g_Element[0], 5, EXTERNAL_PLUGIN_MANAGER_MAX_APPTYPE_LEN, strInit);
        CRequestMsg requestMsg;
        CResponseMsg responseMsg;
        EXPECT_EQ(MP_FAILED, plugMngr.QueryPluginDetail(strAppType, requestMsg, responseMsg));
    }
    DT_FUZZ_END()
}
/*
 * 用例名称：列举资源分页失败
 * 前置条件：NA
 * check点：环境资源类型不存在
 */
TEST_F(FuzzExternalPluginManager, QueryPluginDetailV2Test)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    ExternalPluginManager plugMngr;
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("QueryPluginDetailV2_Test")
    {
        char strInit[] = "1234";
        mp_string strAppType;
        strAppType = DT_SetGetString(&g_Element[0], 5, EXTERNAL_PLUGIN_MANAGER_MAX_APPTYPE_LEN, strInit);
        CRequestMsg requestMsg;
        CResponseMsg responseMsg;
        EXPECT_EQ(MP_FAILED, plugMngr.QueryPluginDetailV2(strAppType, requestMsg, responseMsg));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzExternalPluginManager, CheckPluginTest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    ExternalPluginManager plugMngr;
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("CheckPlugin_Test")
    {
        char strInit[] = "1234";
        mp_string strAppType;
        strAppType = DT_SetGetString(&g_Element[0], 5, EXTERNAL_PLUGIN_MANAGER_MAX_APPTYPE_LEN, strInit);
        CRequestMsg requestMsg;
        CResponseMsg responseMsg;
        EXPECT_EQ(MP_FAILED, plugMngr.CheckPlugin(strAppType, requestMsg, responseMsg));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzExternalPluginManager, WakeUpJobTest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    AppProtectPlugin appProtPlug;
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("WakeUpJob_Test")
    {
        s32 FUZZ_INT_INITVALUE = 1;
        s32 FUZZ_INT_MIN = 1;
        s32 FUZZ_INT_MAX = 8;
        CRequestMsg req; 
        CResponseMsg rsp;
        char strInit[] = "1234";
        mp_uint32 gElementIndex = 0;
        char dtFuzzIpv4Init[4] = {0,0,0,0};
        s32 uuidNums = *(s32*)DT_SetGetNumberRange(&g_Element[gElementIndex++], FUZZ_INT_INITVALUE, FUZZ_INT_MIN, FUZZ_INT_MAX);
        mp_string procUrl = "/v1/tasks/";
        for(s32 j = 0; j < uuidNums; ++j){
            mp_string rdmUuIds;
            CUuidNum::GetUuidNumber(rdmUuIds);
            procUrl += rdmUuIds;
        }
        procUrl += "/notify";
        req.SetProcURL(procUrl);
        Json::Value vl;
        vl[EXTERNAL_PLUGIN_MANAGER_DTFUZZ_NOTIFY_APPTYPE] =
            DT_SetGetString(&g_Element[gElementIndex++], 5, EXTERNAL_PLUGIN_MANAGER_MAX_STRING, strInit);
        for(int i = 0; i < 5; ++i){
            vl[EXTERNAL_PLUGIN_MANAGER_DTFUZZ_NOTIFY_DMEIPLISTS].append(
                DT_SetGetString(&g_Element[gElementIndex++], 5, EXTERNAL_PLUGIN_MANAGER_MAX_STRING, strInit));
        }
        req.SetJsonData(vl);
        appProtPlug.WakeUpJob(req, rsp);
    }
    DT_FUZZ_END()
}

TEST_F(FuzzExternalPluginManager, AbortJobTest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    AppProtectPlugin appProtPlug;
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("AbortJob_Test")
    {
        s32 FUZZ_INT_INITVALUE = 1;
        s32 FUZZ_INT_MIN = 1;
        s32 FUZZ_INT_MAX = 8;
        CRequestMsg req; 
        CResponseMsg rsp;
        mp_uint32 gElementIndex = 0;
        char dtFuzzIpv4Init[4] = {0,0,0,0};
        char strInit[5] = "1234";
        s32 uuidNums = *(s32*)DT_SetGetNumberRange(&g_Element[gElementIndex++], FUZZ_INT_INITVALUE, FUZZ_INT_MIN, FUZZ_INT_MAX);
        mp_string procUrl = "/v1/tasks/";
        for(s32 j = 0; j < uuidNums; ++j){
            mp_string rdmUuIds;
            CUuidNum::GetUuidNumber(rdmUuIds);
            procUrl += rdmUuIds;
        }
        procUrl += "/abort";
        req.SetProcURL(procUrl);
        mp_string strQueryString;
        mp_string strQueryStringPre = "subTaskId=";
        mp_string strQueryStringLast = DT_SetGetString(&g_Element[gElementIndex++], EXTERNAL_PLUGIN_MANAGER_DTFUZZ_MIN_ID_LEN, EXTERNAL_PLUGIN_MANAGER_DTFUZZ_MAX_ID_LEN, strInit);
        strQueryString = strQueryStringPre + strQueryStringLast + "&";
        req.GetURL().SetQueryParam(strQueryString);
        Json::Value vl;
        vl[EXTERNAL_PLUGIN_MANAGER_DTFUZZ_NOTIFY_APPTYPE] =
            DT_SetGetString(&g_Element[gElementIndex++], 5, EXTERNAL_PLUGIN_MANAGER_MAX_STRING, strInit);
        for(int i = 0; i < 5; ++i){
            vl[EXTERNAL_PLUGIN_MANAGER_DTFUZZ_NOTIFY_DMEIPLISTS].append(
                DT_SetGetString(&g_Element[gElementIndex++], 5, EXTERNAL_PLUGIN_MANAGER_MAX_STRING, strInit));
        }
        req.SetJsonData(vl);
        appProtPlug.AbortJob(req, rsp);
    }
    DT_FUZZ_END()
}

TEST_F(FuzzExternalPluginManager, QueryPluginConfigTest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    ExternalPluginManager plugMngr;
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("QueryPluginConfig_Test")
    {
        char strInit[] = "1234";
        mp_string strAppType;
        strAppType = DT_SetGetString(&g_Element[0], 5, EXTERNAL_PLUGIN_MANAGER_MAX_APPTYPE_LEN, strInit);
        CRequestMsg requestMsg;
        CResponseMsg responseMsg;
        EXPECT_EQ(MP_FAILED, plugMngr.QueryPluginConfig(strAppType, requestMsg, responseMsg));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzExternalPluginManager, DeliverJobStatusTest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("DeliverJobStatus_Test")
    {
        char strInit[] = "1234";
        mp_string strAppType;
        strAppType = DT_SetGetString(&g_Element[0], 5, EXTERNAL_PLUGIN_MANAGER_MAX_APPTYPE_LEN, strInit);
        CRequestMsg requestMsg;
        CResponseMsg responseMsg;
        EXPECT_EQ(MP_FAILED, AppProtectService::GetInstance()->DeliverJobStatus(strAppType, requestMsg, responseMsg));
    }
    DT_FUZZ_END()
}
