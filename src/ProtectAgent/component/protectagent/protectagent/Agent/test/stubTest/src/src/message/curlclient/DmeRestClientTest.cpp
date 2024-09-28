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
#include "message/curlclient/DmeRestClientTest.h"
#include "message/curlclient/DmeRestClient.h"
#include "stub.h"
#include "common/Ip.h"
#include "common/Log.h"
#include "common/Types.h"
#include "common/ConfigXmlParse.h"
#include "message/curlclient/CurlHttpClient.h"
#include "message/tcp/CSocket.h"

namespace {
mp_void CLogger_Log_Stub(mp_void* pthis)
{
    return;
}

static mp_void StubDoSleep(mp_uint32 ms)
{
    return;
}

IHttpClient* StubIHttpClientGetInstanceNull()
{
    IHttpClient* httpClient = nullptr;
    return httpClient;
}

mp_int32 StubGetHostEnvHyperdetect(const mp_string& strType, mp_string& strEnv)
{
    strEnv = "hyperdetect";
    return MP_SUCCESS;
}

mp_int32 StubGetHostEnvD4(const mp_string& strType, mp_string& strEnv)
{
    strEnv = "d4";
    return MP_SUCCESS;
}

mp_int32 StubGetHostEnv(const mp_string& strType, mp_string& strEnv)
{
    strEnv = "";
    return MP_SUCCESS;
}
}  // namespace

mp_void StubDmeRestClientTestLogVoid(mp_void* pthis)
{
    return;
}

mp_int32 StubUpdateSecureInfoSuccess()
{
    return MP_SUCCESS;
}

mp_int32 StubUpdateSecureInfoFail()
{
    return MP_FAILED;
}

mp_int32 StubGetConfigValueInt32Success(const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_int32 StubGetConfigValueInt32Fail(const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_FAILED;
}

mp_int32 StubGetValueStringSuccess(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}

mp_int32 StubGetValueStringFailed(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_FAILED;
}

mp_int32 StubCConfigXmlParserInitSuccess(mp_string strCfgFilePath)
{
    return MP_SUCCESS;
}

IHttpResponse* StubSendRequestSuccess(void* curlHttpClientObj, const HttpRequest& req, const mp_uint32 time_out = 90)
{
    CurlHttpClient* client = (CurlHttpClient*)curlHttpClientObj;
    IHttpResponse* rsp = new CurlHttpResponse();
    return rsp;
}

mp_bool StubHttpRspSuccess(void* httpRsp)
{
    CurlHttpResponse* httpResponse = (CurlHttpResponse*)httpRsp;
    return MP_TRUE;
}

mp_bool StubHttpRspSuccessGetFail(void* httpRsp)
{
    CurlHttpResponse* httpResponse = (CurlHttpResponse*)httpRsp;
    return MP_FALSE;
}

mp_bool StubIsDmeIpValid(const mp_string& dmeIp)
{
    return MP_TRUE;
}

mp_int32 StubCheckHostLinkStatusSucc(const mp_string& strSrcIp, const mp_string& strHostIp,
    mp_uint16 uiPort, mp_int32 timeout)
{
    return MP_SUCCESS;
}

mp_int32 StubCheckHostLinkStatusFail(const mp_string& strSrcIp, const mp_string& strHostIp,
    mp_uint16 uiPort, mp_int32 timeout)
{
    return MP_FAILED;
}

/*
 * 用例名称：实例化类失败
 * 前置条件：1、读取secure channel配置失败
 * check点：读取配置失败会导致实例化类失败
 */
TEST_F(DmeRestClientTest, InitDmeRestClientFail)
{
    Stub mp_stub;
    mp_stub.set((mp_void(CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(
                    CLogger, Log),
        StubDmeRestClientTestLogVoid);
    mp_stub.set(
        (mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser, Init), StubCConfigXmlParserInitSuccess);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetConfigValueInt32Fail);
    EXPECT_EQ(nullptr, DmeRestClient::GetInstance());
}
/*
 * 用例名称：实例化类失败
 * 前置条件：1、读取secure channel配置成功，读取domain name失败
 * check点：读取配置失败会导致实例化类失败
 */
TEST_F(DmeRestClientTest, InitDmeRestClientFailTwo)
{
    Stub mp_stub;
    mp_stub.set((mp_void(CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(
                    CLogger, Log),
        StubDmeRestClientTestLogVoid);
    mp_stub.set(
        (mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser, Init), StubCConfigXmlParserInitSuccess);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetConfigValueInt32Success);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubGetValueStringFailed);
    EXPECT_EQ(nullptr, DmeRestClient::GetInstance());
}
/*
 * 用例名称：实例化类成功
 * 前置条件：1、读取相关配置成功
 * check点：实例化类成功
 */
TEST_F(DmeRestClientTest, InitDmeRestClientSucess)
{
    Stub mp_stub;
    mp_stub.set((mp_void(CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(
                    CLogger, Log),
        StubDmeRestClientTestLogVoid);
    mp_stub.set(
        (mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser, Init), StubCConfigXmlParserInitSuccess);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetConfigValueInt32Success);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubGetValueStringSuccess);
    EXPECT_NE(nullptr, DmeRestClient::GetInstance());
}
/*
 * 用例名称：更新DME信息成功
 * 前置条件：DmeRestClient实例化成功
 * check点：更新DME信息成功
 */
TEST_F(DmeRestClientTest, UpdateDmeinfoSucess)
{
    Stub mp_stub;
    mp_stub.set((mp_void(CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(
        CLogger, Log), StubDmeRestClientTestLogVoid);
    mp_stub.set(
        (mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser, Init), StubCConfigXmlParserInitSuccess);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetConfigValueInt32Success);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubGetValueStringFailed);
    std::vector<mp_string> dmeIpList;
    DmeRestClient::GetInstance()->UpdateDmeAddr(dmeIpList, 30062);
}
/*
 * 用例名称：发送HTTP请求成功
 * 前置条件：1. 与DME通信正常 2. 请求格式正确
 * check点：发送HTTP请求成功
 */
TEST_F(DmeRestClientTest, SendRequestSucess)
{
    Stub mp_stub;
    mp_stub.set((mp_void(CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(
                    CLogger, Log),
        StubDmeRestClientTestLogVoid);
    mp_stub.set(
        (mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser, Init), StubCConfigXmlParserInitSuccess);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetConfigValueInt32Success);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubGetValueStringSuccess);
    mp_stub.set(&DmeRestClient::IsDmeIpValid, StubIsDmeIpValid);

    typedef int (*SendRequestPtr)(CurlHttpClient*, const HttpRequest&, const mp_uint32);
    SendRequestPtr sendRequestP = (SendRequestPtr)(&CurlHttpClient::SendRequest);
    mp_stub.set(sendRequestP, StubSendRequestSuccess);

    typedef int (*SuccessPtr)(CurlHttpResponse*);
    SuccessPtr successP = (SuccessPtr)(&CurlHttpResponse::Success);
    mp_stub.set(successP, StubHttpRspSuccess);

    HttpResponse response;
    DmeRestClient::HttpReqParam param;
    param.method = "GET";
    param.url = "/version";
    param.body = "";
    std::vector<mp_string> dmeIpList;
    dmeIpList.push_back("192.168.1.1");
    DmeRestClient::GetInstance()->UpdateDmeAddr(dmeIpList, 30062);
    mp_int32 iRet = DmeRestClient::GetInstance()->SendRequest(param, response);
    EXPECT_EQ(iRet, MP_SUCCESS);
}
/*
 * 用例名称：发送HTTP请求失败，没有DME可以用
 * 前置条件：1. DME信息列表为空
 * check点：没有DME的地址信息，发送HTTP请求失败
 */
TEST_F(DmeRestClientTest, SendRequestFailForEmptyDmeInfo)
{
    Stub mp_stub;
    mp_stub.set((mp_void(CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(
                    CLogger, Log),
        StubDmeRestClientTestLogVoid);
    mp_stub.set(
        (mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser, Init), StubCConfigXmlParserInitSuccess);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetConfigValueInt32Success);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubGetValueStringSuccess);
    mp_stub.set(&DmeRestClient::IsDmeIpValid, StubIsDmeIpValid);

    typedef int (*SendRequestPtr)(CurlHttpClient*, const HttpRequest&, const mp_uint32);
    SendRequestPtr sendRequestP = (SendRequestPtr)(&CurlHttpClient::SendRequest);
    mp_stub.set(sendRequestP, StubSendRequestSuccess);

    typedef int (*SuccessPtr)(CurlHttpResponse*);
    SuccessPtr successP = (SuccessPtr)(&CurlHttpResponse::Success);
    mp_stub.set(successP, StubHttpRspSuccess);

    HttpResponse response;
    DmeRestClient::HttpReqParam param;
    param.method = "GET";
    param.url = "/version";
    param.body = "";
    std::vector<mp_string> dmeIpList;
    DmeRestClient::GetInstance()->UpdateDmeAddr(dmeIpList, 30062);
    mp_int32 iRet = DmeRestClient::GetInstance()->SendRequest(param, response);
    EXPECT_EQ(iRet, MP_FAILED);
}
/*
 * 用例名称：发送HTTP请求失败因为发送失败
 * 前置条件：1. 与DME通信失败
 * check点：无法与DME通信会导致发送HTTP请求失败
 */
TEST_F(DmeRestClientTest, SendRequestFailForDmeNotAccess)
{
    Stub mp_stub;
    mp_stub.set((mp_void(CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(
        CLogger, Log), StubDmeRestClientTestLogVoid);
    mp_stub.set(ADDR(CMpTime, DoSleep), StubDoSleep);
    mp_stub.set(
        (mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser, Init), StubCConfigXmlParserInitSuccess);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetConfigValueInt32Success);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubGetValueStringSuccess);
    mp_stub.set(&DmeRestClient::IsDmeIpValid, StubIsDmeIpValid);

    typedef int (*SendRequestPtr)(CurlHttpClient*, const HttpRequest&, const mp_uint32);
    SendRequestPtr sendRequestP = (SendRequestPtr)(&CurlHttpClient::SendRequest);
    mp_stub.set(sendRequestP, StubSendRequestSuccess);

    typedef int (*SuccessPtr)(CurlHttpResponse*);
    SuccessPtr successP = (SuccessPtr)(&CurlHttpResponse::Success);
    mp_stub.set(successP, StubHttpRspSuccessGetFail);

    HttpResponse response;
    DmeRestClient::HttpReqParam param;
    param.method = "GET";
    param.url = "/version";
    param.body = "";
    std::vector<mp_string> dmeIpList;
    dmeIpList.push_back("192.168.1.1");
    DmeRestClient::GetInstance()->UpdateDmeAddr(dmeIpList, 30062);
    DmeRestClient* restClient = DmeRestClient::GetInstance();
    if (restClient == nullptr) {
        printf("null\n");
    }
    EXPECT_EQ(MP_FAILED, restClient->SendRequest(param, response));
}
/*
 * 用例名称：获得IHttpClient失败
 * 前置条件：1. 与DME通信失败
 * check点：无法与DME通信会导致发送HTTP请求失败
 */
TEST_F(DmeRestClientTest, SendRequestFailForDmeNotIHttpClient)
{
    Stub mp_stub;
    mp_stub.set((mp_void(CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(
                    CLogger, Log),
        StubDmeRestClientTestLogVoid);
    mp_stub.set(ADDR(CMpTime, DoSleep), StubDoSleep);
    mp_stub.set(
        (mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser, Init), StubCConfigXmlParserInitSuccess);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetConfigValueInt32Success);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubGetValueStringSuccess);
    mp_stub.set(&DmeRestClient::IsDmeIpValid, StubIsDmeIpValid);

    DmeRestClient* restClient = DmeRestClient::GetInstance();
    DmeRestClient::HttpReqParam param;
    param.method = "GET";
    param.url = "/version";
    param.body = "";
    HttpResponse response;
    mp_stub.set(&IHttpClient::GetInstance, StubIHttpClientGetInstanceNull);
    EXPECT_EQ(MP_FAILED, restClient->SendRequest(param, response));
}

/*
 * 用例名称：更新UrlIP
 * 前置条件：无
 * check点：返回值结果
 */
TEST_F(DmeRestClientTest, UpdateUrlIp)
{
    Stub mp_stub;
    mp_stub.set((mp_void(CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(
        CLogger, Log), StubDmeRestClientTestLogVoid);
    mp_stub.set(ADDR(CMpTime, DoSleep), StubDoSleep);
    mp_stub.set(
        (mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser, Init), StubCConfigXmlParserInitSuccess);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetConfigValueInt32Success);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubGetValueStringSuccess);

    DmeRestClient* restClient = DmeRestClient::GetInstance();
    HttpRequest req;
    mp_string ip;
    DmeRestClient::HttpReqParam httpParam("PUT", "url", "context");
    HttpResponse response;
    restClient->m_secureChannel = 1;
    mp_stub.set(&CIP::GetHostEnv, StubGetHostEnvHyperdetect);
    EXPECT_EQ(MP_SUCCESS, restClient->UpdateUrlIp(req, ip, httpParam));

    mp_stub.set(&CIP::GetHostEnv, StubGetHostEnvD4);
    EXPECT_EQ(MP_SUCCESS, restClient->UpdateUrlIp(req, ip, httpParam));

    mp_stub.set(&CIP::GetHostEnv, StubGetHostEnv);
    EXPECT_EQ(MP_SUCCESS, restClient->UpdateUrlIp(req, ip, httpParam));
}

/*
 * 用例名称：更新UrlIP
 * 前置条件：无
 * check点：返回值结果
 */
TEST_F(DmeRestClientTest, UpdateSecureInfo)
{
    Stub mp_stub;
    mp_stub.set((mp_void(CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(
                    CLogger, Log),
        StubDmeRestClientTestLogVoid);
    mp_stub.set(ADDR(CMpTime, DoSleep), StubDoSleep);
    mp_stub.set(
        (mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser, Init), StubCConfigXmlParserInitSuccess);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetConfigValueInt32Success);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubGetValueStringSuccess);

    DmeRestClient* restClient = DmeRestClient::GetInstance();
    mp_stub.set(&CIP::GetHostEnv, StubGetHostEnvHyperdetect);
    EXPECT_EQ(MP_SUCCESS, restClient->UpdateSecureInfo());

    mp_stub.set(&CIP::GetHostEnv, StubGetHostEnvD4);
    EXPECT_EQ(MP_SUCCESS, restClient->UpdateSecureInfo());

    mp_stub.set(&CIP::GetHostEnv, StubGetHostEnv);
    EXPECT_EQ(MP_SUCCESS, restClient->UpdateSecureInfo());
}

/*
 * 用例名称：dmeip有效性校验
 * 前置条件：无
 * check点：1.无法与dmeip连通，返回FALSE;2.与dmeip连通，返回True
 */
TEST_F(DmeRestClientTest, IsDmeIpValidTestFalse)
{
    Stub mp_stub;
    mp_stub.set((mp_void(CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(
                    CLogger, Log),
        StubDmeRestClientTestLogVoid);
    mp_stub.set(&CSocket::CheckHostLinkStatus, StubCheckHostLinkStatusFail);
    DmeRestClient* restClient = DmeRestClient::GetInstance();
    mp_string dmeIp = "192.168.0.1";
    EXPECT_EQ(MP_FALSE, restClient->IsDmeIpValid(dmeIp));
    dmeIp = "192.168.0.2";
    mp_stub.set(&CSocket::CheckHostLinkStatus, StubCheckHostLinkStatusSucc);
    EXPECT_EQ(MP_TRUE, restClient->IsDmeIpValid(dmeIp));
}
