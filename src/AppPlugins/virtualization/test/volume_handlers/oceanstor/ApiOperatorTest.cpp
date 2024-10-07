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
#include <iostream>
#include <list>
#include <cstdio>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include <system/System.hpp>
#include <common/Macros.h>
#include <common/Constants.h>
#include <log/Log.h>
#include <securec.h>
#include <curl_http/HttpClientInterface.h>
#include <curl_http/CurlHttpClient.h>
#include <volume_handlers/oceanstor/ApiOperator.h>
#include <protect_engines/hcs/utils/IHttpResponseMock.h>
#include <protect_engines/hcs/utils/IHttpClientMock.h>
#include <volume_handlers/oceanstor/DiskScannerHandler.h>

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

using namespace VirtPlugin;

namespace HDT_TEST {
const std::string g_hostId = "AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE";
class ApiOperatorTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    void InitLogger()
    {
        std::string logFileName = "virt_plugin_test.log";
        std::string logFilePath = "/tmp/log/";
        int logLevel = DEBUG;
        int logFileCount = 10;
        int logFileSize = 30;
        Module::CLogger::GetInstance().Init(
            logFileName.c_str(), logFilePath, logLevel, logFileCount, logFileSize);
    }
public:
    std::shared_ptr<ApiOperator> m_apiOperator;
};

void ApiOperatorTest::SetUp()
{
    InitLogger();
    ControlDeviceInfo deviceInfo;
    m_apiOperator = std::make_shared<ApiOperator>(deviceInfo);
}
void ApiOperatorTest::TearDown(){}
void ApiOperatorTest::SetUpTestCase() {}
void ApiOperatorTest::TearDownTestCase() {}

const std::string g_restSuccess = "{\"data\": {},\"error\": {\"code\": 0,\"description\": \"\"}}";
const std::string g_restNotSession = "{\"data\": {},\"error\": {\"code\": -401,\"description\": \"\"}}";

int32_t Stub_SendRequest_GetHostGroup_OK(void* obj, Module::HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode, bool lockSession)
{
    errorCode = SUCCESS;
    Json::Value cont;
    cont["ID"] = "hoist_id_123";
    cont["NAME"] = "host_name_123";
    data.append(cont);
    return SUCCESS;
}

int32_t Stub_SendRequest_GetHostGroup_Failed(void* obj, Module::HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode, bool lockSession)
{
    errorCode = SUCCESS;
    return SUCCESS;
}

int32_t Stub_SendRequest_GetLunGroup_OK(void* obj, Module::HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode, bool lockSession)
{
    errorCode = SUCCESS;
    Json::Value cont;
    cont["ID"] = "lun_id_123";
    cont["NAME"] = "lun_name_123";
    data.append(cont);
    return SUCCESS;
}

int32_t Stub_SendRequest_GetLunGroup_Failed(void* obj, Module::HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode, bool lockSession)
{
    errorCode = SUCCESS;
    return SUCCESS;
}

int32_t Stub_Logout_Success(void* obj, Module::HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode, StorageSessionInfo &sessionInfo)
{
    errorCode = SUCCESS;
    return SUCCESS;
}

int32_t Stub_Logout_SessionNotExist(void* obj, Module::HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode, StorageSessionInfo &sessionInfo)
{
    errorCode = ApiErrorCode::UNAUTH;
    data["error"]["code"] = errorCode;
    return FAILED;
}

int32_t Stub_DiskScannerHandler_GetFCInitor(void* obj, std::vector<std::string> &vecWWPN)
{
    std::string fcWWPN = "2101f4deafa98b3f";
    vecWWPN.push_back(fcWWPN);
    return SUCCESS;
}

int32_t Stub_DiskScannerHandler_GetFCInitor_Failed(void* obj, std::vector<std::string> &vecWWPN)
{
    return FAILED;
}

int32_t Stub_DiskScannerHandler_GetLoginedTargetIP(void* obj, std::vector<std::string> &vecTargetIP)
{
    std::string ip = "1.1.1.1";
    vecTargetIP.push_back(ip);
    return SUCCESS;
}

int32_t Stub_DiskScannerHandler_GetIscsiInitor(void* obj, std::string &iqnNumber)
{
    iqnNumber = "iqnNumber";
    return SUCCESS;
}

int32_t Stub_DiskScannerHandler_LoginIscsiTarget(void* obj, const std::string &targetIP)
{
    return SUCCESS;
}

int32_t Stub_DiskScannerHandler_LoginIscsiTargetFailed(void* obj, const std::string &targetIP)
{
    return FAILED;
}

static std::shared_ptr<Module::IHttpClient> ApiStub_CreateInitiatorAndHostWhenFcNotCreate()
{
    auto rspMock = std::make_shared<IHttpResponseMock>();
    std::string sessionData = "{\"data\":{\"deviceid\":\"id123\",\"iBaseToken\":\"Token123\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getHostByName = "{\"data\":[{\"NAME\":\"hostName\",\"ID\":\"hostId\",\"RUNNINGSTATUS\":\"27\",\"HEALTHSTATUS\":\"1\",\"OPERATIONSYSTEM\":\"0\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getFcInitiatorByID = "{\"data\":{},\"error\":{\"code\":1077948996,\"description\":\"0\"}}";
    std::string createFcInitiator = "{\"data\":{\"NAME\":\"2101f4deafa98b3f\",\"ID\":\"FcId\",\"PARENTNAME\":\"\",\"RUNNINGSTATUS\":\"27\",\"HEALTHSTATUS\":\"1\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string addFcInitiatorToHost = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string logout = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::set<std::string> cookieValues;
    cookieValues.insert("cookie;values");
    EXPECT_CALL(*rspMock, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*rspMock, GetBody()).WillOnce(Return(sessionData))
                                    .WillOnce(Return(getHostByName))
                                    .WillOnce(Return(getFcInitiatorByID))
                                    .WillOnce(Return(createFcInitiator))
                                    .WillOnce(Return(addFcInitiatorToHost))
                                    .WillOnce(Return(logout));
    EXPECT_CALL(*rspMock, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*rspMock, GetErrCode()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*rspMock, GetCookies()).WillRepeatedly(Return(cookieValues));
    std::shared_ptr<IHttpClientMock> httpClient = std::make_unique<IHttpClientMock>();
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(rspMock));
    return httpClient;
}

static std::shared_ptr<Module::IHttpClient> ApiStub_CreateInitiatorAndHost()
{
    auto rspMock = std::make_shared<IHttpResponseMock>();
    std::string sessionData = "{\"data\":{\"deviceid\":\"id123\",\"iBaseToken\":\"Token123\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getHostByName = "{\"data\":[{\"NAME\":\"hostName\",\"ID\":\"hostId\",\"RUNNINGSTATUS\":\"27\",\"HEALTHSTATUS\":\"1\",\"OPERATIONSYSTEM\":\"0\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getFcInitiatorByID = "{\"data\":{\"NAME\":\"2101f4deafa98b3f\",\"ID\":\"FcId\",\"PARENTNAME\":\"hostName\",\"RUNNINGSTATUS\":\"27\",\"HEALTHSTATUS\":\"1\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string logout = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::set<std::string> cookieValues;
    cookieValues.insert("cookie;values");
    EXPECT_CALL(*rspMock, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*rspMock, GetBody()).WillOnce(Return(sessionData))
                                    .WillOnce(Return(getHostByName))
                                    .WillOnce(Return(getFcInitiatorByID))
                                    .WillOnce(Return(logout));
    EXPECT_CALL(*rspMock, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*rspMock, GetErrCode()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*rspMock, GetCookies()).WillRepeatedly(Return(cookieValues));
    std::shared_ptr<IHttpClientMock> httpClient = std::make_unique<IHttpClientMock>();
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(rspMock));
    return httpClient;
}

static std::shared_ptr<Module::IHttpClient> ApiStub_CreateInitiatorAndHostWhenFcNotOnline()
{
    auto rspMock = std::make_shared<IHttpResponseMock>();
    std::string sessionData = "{\"data\":{\"deviceid\":\"id123\",\"iBaseToken\":\"Token123\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getHostByName = "{\"data\":[{\"NAME\":\"hostName\",\"ID\":\"hostId\",\"RUNNINGSTATUS\":\"27\",\"HEALTHSTATUS\":\"1\",\"OPERATIONSYSTEM\":\"0\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getFcInitiatorByID = "{\"data\":{},\"error\":{\"code\":1077948996,\"description\":\"0\"}}";
    std::string createFcInitiator = "{\"data\":{\"NAME\":\"2101f4deafa98b3f\",\"ID\":\"FcId\",\"PARENTNAME\":\"hostName\",\"RUNNINGSTATUS\":\"28\",\"HEALTHSTATUS\":\"1\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getEthPortIP = "{\"data\":[{\"IPV4ADDR\":\"1.1.1.1\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getBondPortIP = "{\"data\":[{\"IPV4ADDR\":\"1.1.1.1\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getIscsiLogicPortIP = "{\"data\":[{\"SUPPORTPROTOCOL\":\"4\",\"ROLE\":\"4\",\"RUNNINGSTATUS\":\"10\",\"IPV4ADDR\":\"1.1.1.1\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getIscsiInitiatorByID = "{\"data\":{\"ID\":\"iqnNumber\",\"PARENTNAME\":\"hostName\",\"RUNNINGSTATUS\":\"27\",\"HEALTHSTATUS\":\"1\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string logout = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::set<std::string> cookieValues;
    cookieValues.insert("cookie;values");
    EXPECT_CALL(*rspMock, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*rspMock, GetBody()).WillOnce(Return(sessionData))
                                    .WillOnce(Return(getHostByName))
                                    .WillOnce(Return(getFcInitiatorByID))
                                    .WillOnce(Return(createFcInitiator))
                                    .WillOnce(Return(getEthPortIP))
                                    .WillOnce(Return(getBondPortIP))
                                    .WillOnce(Return(getIscsiLogicPortIP))
                                    .WillOnce(Return(getIscsiInitiatorByID))
                                    .WillOnce(Return(getHostByName))
                                    .WillOnce(Return(logout));
    EXPECT_CALL(*rspMock, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*rspMock, GetErrCode()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*rspMock, GetCookies()).WillRepeatedly(Return(cookieValues));
    std::shared_ptr<IHttpClientMock> httpClient = std::make_unique<IHttpClientMock>();
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(rspMock));
    return httpClient;
}

static std::shared_ptr<Module::IHttpClient> ApiStub_CreateInitiatorAndHostWhenAtLeastOneIPConnectable()
{
    auto rspMock = std::make_shared<IHttpResponseMock>();
    std::string sessionData = "{\"data\":{\"deviceid\":\"id123\",\"iBaseToken\":\"Token123\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getHostByName = "{\"data\":[{\"NAME\":\"hostName\",\"ID\":\"hostId\",\"RUNNINGSTATUS\":\"27\",\"HEALTHSTATUS\":\"1\",\"OPERATIONSYSTEM\":\"0\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getEthPortIP = "{\"data\":[{\"IPV4ADDR\":\"1.1.1.1\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getBondPortIP = "{\"data\":[{\"IPV4ADDR\":\"1.1.1.1\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getIscsiLogicPortIP = "{\"data\":[{\"SUPPORTPROTOCOL\":\"4\",\"ROLE\":\"4\",\"RUNNINGSTATUS\":\"10\",\"IPV4ADDR\":\"1.1.1.1\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getIscsiInitiatorByID = "{\"data\":{\"ID\":\"iqnNumber\",\"PARENTNAME\":\"hostName\",\"RUNNINGSTATUS\":\"27\",\"HEALTHSTATUS\":\"1\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string logout = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::set<std::string> cookieValues;
    cookieValues.insert("cookie;values");
    EXPECT_CALL(*rspMock, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*rspMock, GetBody()).WillOnce(Return(sessionData))
                                    .WillOnce(Return(getHostByName))
                                    .WillOnce(Return(getEthPortIP))
                                    .WillOnce(Return(getBondPortIP))
                                    .WillOnce(Return(getIscsiLogicPortIP))
                                    .WillOnce(Return(getIscsiInitiatorByID))
                                    .WillOnce(Return(getHostByName))
                                    .WillOnce(Return(logout));
    EXPECT_CALL(*rspMock, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*rspMock, GetErrCode()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*rspMock, GetCookies()).WillRepeatedly(Return(cookieValues));
    std::shared_ptr<IHttpClientMock> httpClient = std::make_unique<IHttpClientMock>();
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(rspMock));
    return httpClient;
}

static std::shared_ptr<Module::IHttpClient> ApiStub_CreateInitiatorAndHostWhenAllIPUnreachable()
{
    auto rspMock = std::make_shared<IHttpResponseMock>();
    std::string sessionData = "{\"data\":{\"deviceid\":\"id123\",\"iBaseToken\":\"Token123\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getHostByName = "{\"data\":[{\"NAME\":\"hostName\",\"ID\":\"hostId\",\"RUNNINGSTATUS\":\"27\",\"HEALTHSTATUS\":\"1\",\"OPERATIONSYSTEM\":\"0\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getEthPortIP = "{\"data\":[{\"IPV4ADDR\":\"1.1.1.1\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getBondPortIP = "{\"data\":[{\"IPV4ADDR\":\"1.1.1.1\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string getIscsiLogicPortIP = "{\"data\":[{\"SUPPORTPROTOCOL\":\"4\",\"ROLE\":\"4\",\"RUNNINGSTATUS\":\"10\",\"IPV4ADDR\":\"1.1.1.1\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string logout = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::set<std::string> cookieValues;
    cookieValues.insert("cookie;values");
    EXPECT_CALL(*rspMock, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*rspMock, GetBody()).WillOnce(Return(sessionData))
                                    .WillOnce(Return(getHostByName))
                                    .WillOnce(Return(getEthPortIP))
                                    .WillOnce(Return(getBondPortIP))
                                    .WillOnce(Return(getIscsiLogicPortIP))
                                    .WillOnce(Return(logout));
    EXPECT_CALL(*rspMock, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*rspMock, GetErrCode()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*rspMock, GetCookies()).WillRepeatedly(Return(cookieValues));
    std::shared_ptr<IHttpClientMock> httpClient = std::make_unique<IHttpClientMock>();
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(rspMock));
    return httpClient;
}

static std::shared_ptr<Module::IHttpClient> ApiStub_CreateHostWhenHostExist()
{
    auto rspMock = std::make_shared<IHttpResponseMock>();
    std::string sessionData = "{\"data\":{\"deviceid\":\"id123\",\"iBaseToken\":\"Token123\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string createHost = "{\"data\":{},\"error\":{\"code\":1077948993,\"description\":\"0\"}}";
    std::string getHostByName = "{\"data\":[{\"NAME\":\"hostName\",\"ID\":\"hostId\",\"RUNNINGSTATUS\":\"27\",\"HEALTHSTATUS\":\"1\",\"OPERATIONSYSTEM\":\"0\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::string logout = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    std::set<std::string> cookieValues;
    cookieValues.insert("cookie;values");
    EXPECT_CALL(*rspMock, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*rspMock, GetBody()).WillOnce(Return(sessionData))
                                    .WillOnce(Return(createHost))
                                    .WillOnce(Return(getHostByName))
                                    .WillOnce(Return(logout));
    EXPECT_CALL(*rspMock, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*rspMock, GetErrCode()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*rspMock, GetCookies()).WillRepeatedly(Return(cookieValues));
    std::shared_ptr<IHttpClientMock> httpClient = std::make_unique<IHttpClientMock>();
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(rspMock));
    return httpClient;
}

static int Stub_RunShell_HostUuid_OK(const std::string &moduleName, const std::size_t &requestID,
    const std::string &cmd, const std::vector<std::string> params, std::vector<std::string> &cmdoutput,
    std::vector<std::string> &stderroutput, std::stringstream &outstring, const unsigned int &runShellType)
{
    cmdoutput.push_back(g_hostId);
    return SUCCESS;
}

/*
 * 测试用例： 从映射视图中获取主机组
 * 前置条件： 主机组存在
 * CHECK点： 成功获取主机组信息
 */
TEST_F(ApiOperatorTest, GetHostGroupFromMappingViewWhenExist)
{
    Stub stub;
    typedef int32_t (*fptr)(ApiOperator*,Module::HttpRequest &, Json::Value &, std::string &, int &, bool);
    fptr ApiOperator_SendRequest = (fptr)(&ApiOperator::SendRequest);
    stub.set(ApiOperator_SendRequest, Stub_SendRequest_GetHostGroup_OK);
    std::string mapViewID;
    HostGroupMO hostGroupMO;
    std::string errorDes;
    int32_t iRet = m_apiOperator->GetHostGroupFromMappingView(mapViewID, hostGroupMO, errorDes);
    EXPECT_EQ(hostGroupMO.m_id, "hoist_id_123");
    EXPECT_EQ(iRet, SUCCESS);
}

/*
 * 测试用例： 从映射视图中获取主机组
 * 前置条件： 主机组不存在存在
 * CHECK点： 返回失败
 */
TEST_F(ApiOperatorTest, GetHostGroupFromMappingViewWhenNotExist)
{
    Stub stub;
    typedef int32_t (*fptr)(ApiOperator*,Module::HttpRequest &, Json::Value &, std::string &, int &, bool);
    fptr ApiOperator_SendRequest = (fptr)(&ApiOperator::SendRequest);
    stub.set(ApiOperator_SendRequest, Stub_SendRequest_GetHostGroup_Failed);
    std::string mapViewID;
    HostGroupMO hostGroupMO;
    std::string errorDes;
    int32_t iRet = m_apiOperator->GetHostGroupFromMappingView(mapViewID, hostGroupMO, errorDes);
    EXPECT_EQ(iRet, FAILED);
}

/*
 * 测试用例： 从映射视图中获取Lun组
 * 前置条件： Lun组存在
 * CHECK点： 成功获取Lun组信息
 */
TEST_F(ApiOperatorTest, GetLunGroupFromMappingViewWhenExist)
{
    Stub stub;
    typedef int32_t (*fptr)(ApiOperator*,Module::HttpRequest &, Json::Value &, std::string &, int &, bool);
    fptr ApiOperator_SendRequest = (fptr)(&ApiOperator::SendRequest);
    stub.set(ApiOperator_SendRequest, Stub_SendRequest_GetLunGroup_OK);
    std::string mapViewID;
    LunGroupMO lunGroupMO;
    std::string errorDes;
    int32_t iRet = m_apiOperator->GetLunGroupFromMappingView(mapViewID, lunGroupMO, errorDes);
    EXPECT_EQ(lunGroupMO.m_id, "lun_id_123");
    EXPECT_EQ(iRet, SUCCESS);
}

/*
 * 测试用例： 从映射视图中获取Lun组
 * 前置条件： Lun不存在存在
 * CHECK点： 返回失败
 */
TEST_F(ApiOperatorTest, GetLunGroupFromMappingViewWhenNotExist)
{
    Stub stub;
    typedef int32_t (*fptr)(ApiOperator*,Module::HttpRequest &, Json::Value &, std::string &, int &, bool);
    fptr ApiOperator_SendRequest = (fptr)(&ApiOperator::SendRequest);
    stub.set(ApiOperator_SendRequest, Stub_SendRequest_GetLunGroup_Failed);
    std::string mapViewID;
    LunGroupMO lunGroupMO;
    std::string errorDes;
    int32_t iRet = m_apiOperator->GetLunGroupFromMappingView(mapViewID, lunGroupMO, errorDes);
    EXPECT_EQ(iRet, FAILED);
}

/*
 * 测试用例： 登出设备
 * 前置条件： session存在
 * CHECK点： 返回成功
 */
TEST_F(ApiOperatorTest, LogoutSuccess)
{
    Stub stub;
    typedef int32_t (*fptr)(ApiOperator*, Module::HttpRequest&, Json::Value&, std::string&, int&, StorageSessionInfo&);
    fptr ApiOperator_SendRequestEx = (fptr)(&ApiOperator::SendRequestEx);
    stub.set(ApiOperator_SendRequestEx, Stub_Logout_Success);
    StorageSessionInfo sessionInfo;
    int32_t iRet = m_apiOperator->Logout(sessionInfo);
    EXPECT_EQ(iRet, SUCCESS);
}

/*
 * 测试用例： 登出设备
 * 前置条件： session不存在在
 * CHECK点： 返回失败
 */
TEST_F(ApiOperatorTest, LogoutWhenSessionNotExist)
{
    Stub stub;
    typedef int32_t (*fptr)(ApiOperator*, Module::HttpRequest&, Json::Value&, std::string&, int&, StorageSessionInfo&);
    fptr ApiOperator_SendRequestEx = (fptr)(&ApiOperator::SendRequestEx);
    stub.set(ApiOperator_SendRequestEx, Stub_Logout_SessionNotExist);
    StorageSessionInfo sessionInfo;
    int32_t iRet = m_apiOperator->Logout(sessionInfo);
    EXPECT_EQ(iRet, ApiErrorCode::UNAUTH);
}

/*
 * 测试用例： 用所有目标IP登录ISCSI启动器
 * 前置条件： 1、存在至少一个IP可连接
 * CHECK点： 创建ISCSI启动器成功
 */
TEST_F(ApiOperatorTest, CreateInitiatorAndHostWhenAtLeastOneIPConnectable)
{
    Stub stub;
    stub.set(ADDR(DiskScannerHandler, GetFCInitor), Stub_DiskScannerHandler_GetFCInitor_Failed);
    stub.set(ADDR(DiskScannerHandler, GetIscsiInitor), Stub_DiskScannerHandler_GetIscsiInitor);
    stub.set(ADDR(DiskScannerHandler, LoginIscsiTarget), Stub_DiskScannerHandler_LoginIscsiTarget);
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_HostUuid_OK);
    stub.set(ADDR(Module::IHttpClient, CreateClient), ApiStub_CreateInitiatorAndHostWhenAtLeastOneIPConnectable);
    std::string hostName = "hostName";
    HostMO hostMO;
    ControlDeviceInfo deviceInfo;
    ApiOperator apiOperator(deviceInfo);
    int32_t iRet = apiOperator.CreateInitiatorAndHost(hostName, hostMO);
    EXPECT_EQ(iRet, SUCCESS);
    stub.reset(ADDR(DiskScannerHandler, GetFCInitor));
    stub.reset(ADDR(DiskScannerHandler, GetIscsiInitor));
    stub.reset(ADDR(Module::IHttpClient, CreateClient));
}

/*
 * 测试用例： 用所有目标IP登录ISCSI启动器
 * 前置条件： 1、所有IP不可连接
 * CHECK点： 创建ISCSI启动器失败
 */
TEST_F(ApiOperatorTest, CreateInitiatorAndHostWhenAllIPUnreachable)
{
    Stub stub;
    stub.set(ADDR(DiskScannerHandler, GetFCInitor), Stub_DiskScannerHandler_GetFCInitor_Failed);
    stub.set(ADDR(DiskScannerHandler, GetIscsiInitor), Stub_DiskScannerHandler_GetIscsiInitor);
    stub.set(ADDR(DiskScannerHandler, LoginIscsiTarget), Stub_DiskScannerHandler_LoginIscsiTargetFailed);
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_HostUuid_OK);
    stub.set(ADDR(Module::IHttpClient, CreateClient), ApiStub_CreateInitiatorAndHostWhenAllIPUnreachable);
    std::string hostName = "hostName";
    HostMO hostMO;
    ControlDeviceInfo deviceInfo;
    ApiOperator apiOperator(deviceInfo);
    int32_t iRet = apiOperator.CreateInitiatorAndHost(hostName, hostMO);
    EXPECT_EQ(iRet, FAILED);
    stub.reset(ADDR(DiskScannerHandler, GetFCInitor));
    stub.reset(ADDR(DiskScannerHandler, GetIscsiInitor));
    stub.reset(ADDR(Module::IHttpClient, CreateClient));
}

/*
 * 测试用例： 创建HOST主机，当主机存在时返回已存在主机信息
 * 前置条件： HOST主机已存在
 * CHECK点： 创建HOST成功
 */
TEST_F(ApiOperatorTest, CreateInitiatorAndHostWhenFcNotCreate)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, CreateClient), ApiStub_CreateHostWhenHostExist);
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_HostUuid_OK);
    std::string hostName = "hostName";
    std::string hostIp = "";
    HostMO hostMO;
    std::string errorDes;
    ControlDeviceInfo deviceInfo;
    ApiOperator apiOperator(deviceInfo);
    int32_t iRet = apiOperator.CreateHost(hostName, hostMO, hostIp, errorDes);
    EXPECT_EQ(iRet, SUCCESS);
    stub.reset(ADDR(Module::IHttpClient, CreateClient));
}

/*
 * 测试用例： 创建启动器
 * 前置条件： 本机FC存在，存储上未创建FC启动器
 * CHECK点： 创建FC启动器成功
 */
// TEST_F(ApiOperatorTest, CreateInitiatorAndHostWhenFcNotCreate)
// {
//     Stub stub;
//     stub.set(ADDR(DiskScannerHandler, GetFCInitor), Stub_DiskScannerHandler_GetFCInitor);
//     stub.set(ADDR(Module::IHttpClient, CreateClient), ApiStub_CreateInitiatorAndHostWhenFcNotCreate);
//     stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_HostUuid_OK);
//     std::string hostName = "hostName";
//     HostMO hostMO;
//     ControlDeviceInfo deviceInfo;
//     ApiOperator apiOperator(deviceInfo);
//     int32_t iRet = apiOperator.CreateInitiatorAndHost(hostName, hostMO);
//     EXPECT_EQ(iRet, SUCCESS);
//     stub.reset(ADDR(DiskScannerHandler, GetFCInitor));
//     stub.reset(ADDR(Module::IHttpClient, CreateClient));
// }

/*
 * 测试用例： 创建启动器
 * 前置条件： 本机FC存在，存储上已存在FC启动器
 * CHECK点： 创建FC启动器成功
 */
// TEST_F(ApiOperatorTest, CreateInitiatorAndHostWhenFcAvailable)
// {
//     Stub stub;
//     stub.set(ADDR(DiskScannerHandler, GetFCInitor), Stub_DiskScannerHandler_GetFCInitor);
//     stub.set(ADDR(Module::IHttpClient, CreateClient), ApiStub_CreateInitiatorAndHost);
//     std::string hostName = "hostName";
//     HostMO hostMO;
//     ControlDeviceInfo deviceInfo;
//     ApiOperator apiOperator(deviceInfo);
//     int32_t iRet = apiOperator.CreateInitiatorAndHost(hostName, hostMO);
//     EXPECT_EQ(iRet, SUCCESS);
//     stub.reset(ADDR(DiskScannerHandler, GetFCInitor));
//     stub.reset(ADDR(Module::IHttpClient, CreateClient));
// }

/*
 * 测试用例： 创建启动器
 * 前置条件： 本机FC存在，存储上未创建FC启动器，启动器均不在线
 * CHECK点： FC启动器创建失败后，检查创建iscsi启动器
 */
// TEST_F(ApiOperatorTest, CreateInitiatorAndHostWhenFcNotOnline)
// {
//     Stub stub;
//     stub.set(ADDR(DiskScannerHandler, GetFCInitor), Stub_DiskScannerHandler_GetFCInitor);
//     stub.set(ADDR(DiskScannerHandler, GetLoginedTargetIP), Stub_DiskScannerHandler_GetLoginedTargetIP);
//     stub.set(ADDR(DiskScannerHandler, GetIscsiInitor), Stub_DiskScannerHandler_GetIscsiInitor);
//     stub.set(ADDR(Module::IHttpClient, CreateClient), ApiStub_CreateInitiatorAndHostWhenFcNotOnline);
//     std::string hostName = "hostName";
//     HostMO hostMO;
//     ControlDeviceInfo deviceInfo;
//     ApiOperator apiOperator(deviceInfo);
//     int32_t iRet = apiOperator.CreateInitiatorAndHost(hostName, hostMO);
//     EXPECT_EQ(iRet, SUCCESS);
//     stub.reset(ADDR(DiskScannerHandler, GetFCInitor));
//     stub.reset(ADDR(DiskScannerHandler, GetLoginedTargetIP));
//     stub.reset(ADDR(DiskScannerHandler, GetIscsiInitor));
//     stub.reset(ADDR(Module::IHttpClient, CreateClient));
// }
}
