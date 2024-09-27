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
#include <vector>
#include "common/Utils.h"
#include "alarm/Trap.h"
#include "fuzz/HostAgent/FuzzHostAgent.h"
#include "secodeFuzz.h"
#include "plugins/host/HostPlugin.h"
#include "common/Types.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "message/tcp/TCPClientHandler.h"
#include "plugins/host/UpgradeHandle.h"
#include "plugins/host/ModifyPluginHandle.h"
#include "host/host.h"
#include "Cmd.h"

#ifndef WIN32
#include <sys/time.h>
#endif
#include "securecom/RootCaller.h"
using namespace std;

namespace {
    static const mp_int32 HOST_PLUGIN_NUM_65535 = 65535;
    const mp_string MESSAGE_BODY_IPV4 = "ipv4";
    const mp_string MESSAGE_BODY_IPV6 = "ipv6";
    static int g_iSybaseJsonStringCounter = 0;
    mp_int32 EXTERNAL_PLUGIN_MANAGER_MAX_APPTYPE_LEN = 32;
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
static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_int32 CMpThread_Create_stub(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_SUCCESS;
}

mp_int32 StubSUCCESS()
{
    return MP_SUCCESS;
}
mp_int32 AGENTSUCCESS()
{
    return MP_SUCCESS;
}
mp_int32 AnalyzeManageMsgStub(){
   return MP_SUCCESS;
 }
mp_int32 GenerateTrapInfoStub(CRequestMsg& req, std::vector<trap_server>& vecTrapServer, snmp_v3_param& stParam){
    return MP_SUCCESS;
}
   
}

TEST_F(FuzzHostAgent, GetHostIpstest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(CDppMessage, AnalyzeManageMsg),AnalyzeManageMsgStub);
    m_stub.set(ADDR(CJsonUtils, GetJsonString), SybaseStubJsonString);
    HostPlugin plugObj;
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("FuzzGetHostIpstest")
    {
        CDppMessage req;
        CDppMessage rspMsg;
        Json::Value jsonReq;
        Json::Value jsonbody;
        char dtfuzzip[4]={0,0,0,0};
        char dtfuzzipv6[16]={0};
        char strInit[] = "1234";
        mp_int32 index = 0;
        for(int i=0; i<5; ++i) {
            jsonbody[MESSAGE_BODY_IPV4].append(DT_SetGetString(&g_Element[0], 5, MAX_STRING_LEN, strInit));
            jsonbody[MESSAGE_BODY_IPV6].append(DT_SetGetString(&g_Element[1], 5, MAX_STRING_LEN, strInit));
        }
        jsonReq[MANAGECMD_KEY_BODY] = jsonbody;
        req.SetMsgBody(jsonReq);
        plugObj.GetHostIps(req, rspMsg);
    }
    DT_FUZZ_END()
}

TEST_F(FuzzHostAgent, ScanDiskByDpptest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(CDppMessage, AnalyzeManageMsg),AnalyzeManageMsgStub);
    m_stub.set(ADDR(CJsonUtils, GetJsonString), SybaseStubJsonString);
    m_stub.set(ADDR(CHost, ScanDisk), AGENTSUCCESS);
    HostPlugin plugObj;
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("FuzzScanDiskByDpptest")
    {
        CDppMessage req;
        CDppMessage rspMsg;
        Json::Value jsonReq;
        Json::Value jsonbody;
        mp_string taskId;
        char strInit[]="1234";
        jsonbody[taskId] = DT_SetGetString(&g_Element[0], 5, MAX_TASKID_LEN, strInit);
        jsonReq[MANAGECMD_KEY_BODY] = jsonbody;
        req.SetMsgBody(jsonReq);
        plugObj.ScanDiskByDpp(req, rspMsg);
    }
    DT_FUZZ_END()
}

TEST_F(FuzzHostAgent, UpgradeAgenttest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(CIPCFile, WriteFile), StubSUCCESS);
    m_stub.set(ADDR(UpgradeHandle, UpdateUpgradeStatus), StubSUCCESS);
    m_stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    HostPlugin plugObj;
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("FuzzScanDiskByDpptest")
    {
        CRequestMsg req;
        CResponseMsg rsp;
        Json::Value jsonReq;
        char strInit[] = "1234";
        jsonReq[REST_PARAM_AGENT_UPGRADE_DOWNLOADLINK] = DT_SetGetString(&g_Element[0], 5, MAX_STRING_LEN, strInit);
        jsonReq[REST_PARAM_AGENT_UPGRADE_AGENTID] = DT_SetGetString(&g_Element[1], 5, MAX_STRING_LEN, strInit);
        jsonReq[REST_PARAM_AGENT_UPGRADE_AGENTNAME] = DT_SetGetString(&g_Element[2], 5, MAX_STRING_LEN, strInit);
        req.GetMsgBody().SetJsonValue(jsonReq);
        plugObj.UpgradeAgent(req, rsp);
    }
    DT_FUZZ_END()
}

TEST_F(FuzzHostAgent, ModifyPlugintest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(CIPCFile, WriteFile), StubSUCCESS);
    m_stub.set(ADDR(ModifyPluginHandle, UpdateModifyPluginStatus), StubSUCCESS);
    m_stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    HostPlugin plugObj;
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("FuzzScanDiskByDpptest")
    {
        CRequestMsg req;
        CResponseMsg rsp;
        Json::Value jsonReq;
        char strInit[] = "1234";
        jsonReq[REST_PARAM_AGENT_UPGRADE_DOWNLOADLINK] = DT_SetGetString(&g_Element[0], 5, MAX_STRING_LEN, strInit);
        jsonReq[REST_PARAM_AGENT_UPGRADE_AGENTID] = DT_SetGetString(&g_Element[1], 5, MAX_STRING_LEN, strInit);
        jsonReq[REST_PARAM_AGENT_UPGRADE_AGENTNAME] = DT_SetGetString(&g_Element[2], 5, MAX_STRING_LEN, strInit);
        req.GetMsgBody().SetJsonValue(jsonReq);
        plugObj.ModifyPlugin(req, rsp);
    }
    DT_FUZZ_END()
}

TEST_F(FuzzHostAgent, UpdateTrapServertest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(HostPlugin, GenerateTrapInfo), GenerateTrapInfoStub);
    HostPlugin plugObj;
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("FuzzUpdateTrapServertest")
    {
        CRequestMsg req;
        CResponseMsg rsp;
        Json::Value jsonReq;
        req.SetJsonData(jsonReq);
        plugObj.UpdateTrapServer(req, rsp);
    }
    DT_FUZZ_END()
}

TEST_F(FuzzHostAgent, ConnectDMEtest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(TCPClientHandler, Connect), StubSUCCESS);
    HostPlugin plugObj;
     DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("ConnectDMEtest")
    {
        CRequestMsg req;
        CResponseMsg rsp;
        char dtfuzzip[4]={0,0,0,0};
        char strInit[] = "1234";
        Json::Value jsonReq;
        jsonReq[SNMP_CONNECT_DME_TYPE] = DT_SetGetNumberRange(&g_Element[0], 1, 1, 200);
        jsonReq[SNMP_CONNECT_DME_IP].append(DT_SetGetString(&g_Element[1], 5, MAX_STRING_LEN, strInit));
        jsonReq[SNMP_CONNECT_DME_PORT] = DT_SetGetNumberRange(&g_Element[2], 1, 1, 65535);
        req.SetJsonData(jsonReq);
        plugObj.ConnectDME(req, rsp);
    }
    DT_FUZZ_END()
}





