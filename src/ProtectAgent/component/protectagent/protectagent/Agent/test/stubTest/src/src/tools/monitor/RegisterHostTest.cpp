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
#include "tools/agentcli/RegisterHostTest.h"
#include "common/Utils.h"
#include "host/host.h"
#include "common/Ip.h"

namespace {
const mp_string REGISTER_HOST("RegisterHost");

mp_void LogReturn(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...)
{
    return;
}

mp_int32 RegisterHost2PMFailed()
{
    return MP_FAILED;
}

mp_int32 RegisterHost2PMSucc()
{
    return MP_SUCCESS;
}

mp_int32 DeleteHostFromPMFailed()
{
    return MP_FAILED;
}

mp_int32 DeleteHostFromPMSucc()
{
    return MP_SUCCESS;
}

mp_int32 CopyFileCoverDestFailed(const mp_string& pszSourceFilePath, const mp_string& pszDestFilePath)
{
    return MP_FAILED;
}

mp_int32 CopyFileCoverDestSucc(const mp_string& pszSourceFilePath, const mp_string& pszDestFilePath)
{
    return MP_SUCCESS;
}

mp_int32 GetInfoFailed(host_info_t& hostInfo)
{
    return MP_FAILED;
}

mp_int32 GetInfoSucc(mp_void* pThis)
{
    return MP_SUCCESS;
}

mp_int32 SendRequestFailed(const HttpRequest& req, mp_string& responseBody)
{
    return MP_FAILED;
}

mp_int32 SendRequestSucc(const HttpRequest& req, mp_string& responseBody)
{
    return MP_SUCCESS;
}

mp_int32 ConvertStringtoJsonFailed(const mp_string& rawBuffer, Json::Value& jsValue)
{
    return MP_FAILED;
}

mp_int32 ConvertStringtoJsonSucc(const mp_string& rawBuffer, Json::Value& jsValue)
{
    return MP_SUCCESS;
}

mp_int32 GetListenIPAndPortFailed(mp_string& strIP, mp_string& strPort)
{
    return MP_FAILED;
}

mp_int32 GetListenIPAndPortSucc(mp_string& strIP, mp_string& strPort)
{
    return MP_SUCCESS;
}

mp_int32 GetHostInfoFailed(Json::Value& hostMsg)
{
    return MP_FAILED;
}

mp_int32 GetHostInfoSucc(Json::Value& hostMsg)
{
    return MP_SUCCESS;
}

mp_int32 RestartNginxFailed()
{
    return MP_FAILED;
}

mp_int32 RestartNginxSucc()
{
    return MP_SUCCESS;
}

mp_int32 StubCConfigXmlParserGetValueInt32ReturnSuccess(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_int32 StubCConfigXmlParserGetValueInt32ReturnFail(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_FAILED;
}

static mp_void StubDoSleep(mp_uint32 ms)
{
    return;
}

static mp_int32 StubHasDataTurboRuning(const mp_string& strCommand, std::vector<mp_string>& strEcho, mp_bool bNeedRedirect)
{
    strEcho.push_back("datatur+ 11404     1  6 16:14 ?        00:02:55 /opt/oceanstor/dataturbo/bin/dpc");
    return MP_SUCCESS;
}

static mp_int32 StubDataTurboNotRuning(const mp_string& strCommand, std::vector<mp_string>& strEcho, mp_bool bNeedRedirect)
{
    return MP_SUCCESS;
}
}

TEST_F(CRegisterHostTest, handleTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    stub.set(DoSleep, StubDoSleep);
    RegisterHost registerHost;
    mp_string actionType;
    mp_string actionType2;
    mp_string actionType3;

    actionType = "Agent";
    mp_int32 iRet = registerHost.Handle(actionType, actionType2, actionType3);
    EXPECT_EQ(iRet, MP_FAILED);

    actionType = REGISTER_HOST;
    iRet = registerHost.Handle(actionType, actionType2, actionType3);
    EXPECT_EQ(iRet, MP_FAILED);

    actionType2 = "Agent";
    iRet = registerHost.Handle(actionType, actionType2, actionType3);
    EXPECT_EQ(iRet, MP_FAILED);

    actionType3 = "Agent";
    iRet = registerHost.Handle(actionType, actionType2, actionType3);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(CRegisterHostTest, ReportHostTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    RegisterHost registerHost;
    
    stub.set(ADDR(RegisterHost, RegisterHost2PM), RegisterHost2PMFailed);
    mp_int32 iRet = registerHost.ReportHost();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(RegisterHost, RegisterHost2PM), RegisterHost2PMSucc);
    iRet = registerHost.ReportHost();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CRegisterHostTest, DeleteHostTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    RegisterHost registerHost;

    stub.set(ADDR(RegisterHost, DeleteHostFromPM), DeleteHostFromPMFailed);
    mp_int32 iRet = registerHost.DeleteHost();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(RegisterHost, DeleteHostFromPM), DeleteHostFromPMSucc);
    iRet = registerHost.DeleteHost();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CRegisterHostTest, GetPMIPandPortTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    RegisterHost registerHost;
    mp_int32 iRet = registerHost.GetPMIPandPort();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CRegisterHostTest, UpdateNginxConfTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    RegisterHost registerHost;

    stub.set(ADDR(CMpFile, CopyFileCoverDest), CopyFileCoverDestFailed);
    mp_int32 iRet = registerHost.UpdateNginxConf();
    EXPECT_EQ(iRet, MP_FAILED);
    
    stub.set(ADDR(CMpFile, CopyFileCoverDest), CopyFileCoverDestSucc);
    iRet = registerHost.UpdateNginxConf();
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CRegisterHostTest, DeleteHostFromPMTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    RegisterHost registerHost;

    stub.set(ADDR(CHost, GetHostSN), DeleteHostFromPMFailed);
    mp_int32 iRet = registerHost.DeleteHostFromPM();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CHost, GetHostSN), DeleteHostFromPMSucc);
    stub.set(ADDR(RegisterHost, SendRequest), SendRequestFailed);
    iRet = registerHost.DeleteHostFromPM();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(RegisterHost, SendRequest), SendRequestSucc);
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), ConvertStringtoJsonFailed);
    iRet = registerHost.DeleteHostFromPM();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), ConvertStringtoJsonSucc);
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), ConvertStringtoJsonSucc);
    iRet = registerHost.DeleteHostFromPM();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CRegisterHostTest, InitDeleteHostReqTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
        StubCConfigXmlParserGetValueInt32ReturnSuccess);
    RegisterHost registerHost;
    mp_string hostid;
    HttpRequest req;
    mp_string PMIp;

    stub.set(ADDR(RegisterHost, GetHostInfo), GetHostInfoFailed);
    mp_int32 iRet = registerHost.InitDeleteHostReq(hostid, req, PMIp);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(RegisterHost, GetHostInfo), GetHostInfoSucc);
    iRet = registerHost.InitDeleteHostReq(hostid, req, PMIp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CRegisterHostTest, GetHostInfoTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    RegisterHost registerHost;
    Json::Value hostMsg;

    stub.set(ADDR(CHost, GetInfo), GetInfoFailed);
    mp_int32 iRet = registerHost.GetHostInfo(hostMsg);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(CRegisterHostTest, SecurityConfigurationTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    RegisterHost registerHost;
    HttpRequest req;
    mp_string actionType;

    actionType = "caInfo";
    registerHost.SecurityConfiguration(req, actionType);

    actionType = "sslCert";
    registerHost.SecurityConfiguration(req, actionType);

    actionType = "sslKey";
    registerHost.SecurityConfiguration(req, actionType);

    actionType = "cert";
    registerHost.SecurityConfiguration(req, actionType);
}

TEST_F(CRegisterHostTest, RegisterHost2PMTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    RegisterHost registerHost;

    stub.set(ADDR(RegisterHost, SendRequest), SendRequestFailed);
    mp_int32 iRet = registerHost.RegisterHost2PM();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(RegisterHost, SendRequest), SendRequestSucc);
    stub.set(ADDR(RegisterHost, RestartNginx), RestartNginxFailed);
    iRet = registerHost.RegisterHost2PM();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(RegisterHost, SendRequest), SendRequestSucc);
    stub.set(ADDR(RegisterHost, RestartNginx), RestartNginxSucc);
    iRet = registerHost.RegisterHost2PM();
    EXPECT_NE(iRet, MP_SUCCESS);
}


TEST_F(CRegisterHostTest, ParseRegisterHostRespondsV2Test)
{
    Stub stub;
    RegisterHost registerHost;
    mp_string rspBody;
    mp_uint32 statusCode = 200;

    mp_int32 iRet = registerHost.ParseRegisterHostRespondsV2(rspBody, statusCode);
    EXPECT_EQ(iRet, MP_SUCCESS);

    statusCode = 404;
    iRet = registerHost.ParseRegisterHostRespondsV2(rspBody, statusCode);
    EXPECT_EQ(iRet, MP_FAILED);

    rspBody= "{\"errorCode\": \"1677929223\"}";
    iRet = registerHost.ParseRegisterHostRespondsV2(rspBody, statusCode);
    EXPECT_EQ(iRet, MP_FAILED);
}


TEST_F(CRegisterHostTest, InitUrlTest)
{
    Stub stub;
    RegisterHost registerHost;
    HttpRequest req;
    mp_string m_PMIp;

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
        StubCConfigXmlParserGetValueInt32ReturnFail);
    mp_int32 iRet = registerHost.InitUrl(req, m_PMIp);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
        StubCConfigXmlParserGetValueInt32ReturnSuccess);
    iRet = registerHost.InitUrl(req, m_PMIp);
    EXPECT_EQ(iRet, MP_SUCCESS);

}

TEST_F(CRegisterHostTest, GetHostInfoV2Test)
{
    Stub stub;
    RegisterHost registerHost;
    Json::Value hostMsg;
    
    stub.set(ADDR(CHost, GetInfo), GetInfoFailed);
    mp_int32 iRet = registerHost.GetHostInfoV2(hostMsg);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CHost, GetInfo), GetInfoSucc);
    stub.set(ADDR(CHost, GetHostExtendInfo), GetInfoSucc);
    stub.set(ADDR(CHost, GetHostAgentIplist), GetInfoSucc);
    stub.set(ADDR(CIP, GetApplications), GetInfoSucc);
    iRet = registerHost.GetHostInfoV2(hostMsg);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CRegisterHostTest, IsInstallDataTurbo)
{
    Stub stub;
    RegisterHost registerHost;
    
    stub.set(ADDR(CSystemExec, ExecSystemWithEcho), StubHasDataTurboRuning);
    mp_bool iRet = registerHost.IsInstallDataTurbo();
    EXPECT_EQ(iRet, MP_TRUE);

    stub.set(ADDR(CSystemExec, ExecSystemWithEcho), StubDataTurboNotRuning);
    iRet = registerHost.IsInstallDataTurbo();
    EXPECT_EQ(iRet, MP_FALSE);
    stub.reset(ADDR(CSystemExec, ExecSystemWithEcho));
}