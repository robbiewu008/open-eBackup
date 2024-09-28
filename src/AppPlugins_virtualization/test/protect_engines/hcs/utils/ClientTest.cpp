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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "protect_engines/hcs/common/HcsCommonInfo.h"
#include "common/Structs.h"
#include "ClientMock.h"
#include "common/model/ResponseModelMock.h"
#include "common/model/ModelBaseMock.h"
#include "common/httpclient/HttpClient.h"
#include "protect_engines/hcs/utils/HCSTokenMgr.h"
#include "IHttpResponseMock.h"
#include "IHttpClientMock.h"


using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ::testing::Invoke;;

using namespace VirtPlugin;
using namespace HcsPlugin;

namespace HDT_TEST {
class ClientTest : public testing::Test {
public:
    void InitLogger()
    {
        std::string logFileName = "client_test.log";
        std::string logFilePath = "/tmp/log/";
        int logLevel = DEBUG;
        int logFileCount = 10;
        int logFileSize = 30;
        Module::CLogger::GetInstance().Init(
            logFileName.c_str(), logFilePath, logLevel, logFileCount, logFileSize);
    }

protected:
    void SetUp()
    {
        m_requestInfo.m_method = "POST";
        m_requestInfo.m_resourcePath = "http://{path_param}";
        m_requestInfo.m_pathParams = {{"path_param", "PathParamTest"}};
        m_requestInfo.m_queryParams = {{"query_param_1", "queryParamTest_1"}, {"query_param_2", "queryParamTest_2"}};
        m_requestInfo.m_headerParams = {{"Content-Type", "application/json"}};
        m_requestInfo.m_body = "";
        InitLogger();
    }
    void TearDown() {}
    VirtPlugin::RequestInfo m_requestInfo;
    ClientMock m_clientMock;
};

int gStatusCode = 401;

int32_t Stub_HcsHttpClient_Send_SUCCESS(const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    return SUCCESS;
}

int32_t Stub_HcsHttpClient_Send_FAILED(const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    return FAILED;
}

static Module::IHttpClient* Stub_CreateClient_SendRequest_Success_errorCode_401()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    testing::Mock::AllowLeak(httpRespone.get());
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    if (gStatusCode == 401) {
        EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(401));
        gStatusCode = 200;
    } else {
        EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(200));
    }
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(""));
    std::map<std::string, std::set<std::string> > getHeadersReturn = {};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock* httpClient = new(std::nothrow) IHttpClientMock();
    testing::Mock::AllowLeak(httpClient);
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

static bool StubGetTokenFromMgrFailed(ModelBase &model, std::string &tokenValue)
{
    return false;
}

static bool Stub_get_tokenFromMgrSuccess()
{
    return true;
}

/*
 * 测试用例： 调用客户端发送成功
 * 前置条件： 发送接口返回SUCCESS
 * CHECK点： 调用hcsAPI成功
 */
TEST_F(ClientTest, ExecCallApiSuccess)
{
    ModelBaseMock request;
    std::shared_ptr<ResponseModelMock> response = std::make_shared<ResponseModelMock>();

    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_HcsHttpClient_Send_SUCCESS);
    int32_t ret = m_clientMock.CallApi(m_requestInfo, response, request);
    EXPECT_EQ(ret, SUCCESS);
    stub.reset(ADDR(HttpClient, Send));
}


/*
 * 测试用例： 调用客户端发送失败
 * 前置条件： 发送接口返回FAILED
 * CHECK点： 调用发送接口失败
 */
TEST_F(ClientTest, ExecCallApiFailed)
{
    ModelBaseMock request;
    std::shared_ptr<ResponseModelMock> response = std::make_shared<ResponseModelMock>();

    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_HcsHttpClient_Send_FAILED);
    int32_t ret = m_clientMock.CallApi(m_requestInfo, response, request);
    EXPECT_EQ(ret, FAILED);
    stub.reset(ADDR(HttpClient, Send));
}

/*
 * 测试用例： 路径参数替换正确
 * 前置条件： 1.requestInfo.m_pathParams. 2.path中有占位字符串
 * CHECK点： 判断路径返回值和预期一致
 */
TEST_F(ClientTest, GetResourcePathSuccess)
{
    std::string url = m_requestInfo.m_resourcePath;
    std::string returnValue = m_clientMock.GetResourcePath(url, m_requestInfo.m_pathParams);
    EXPECT_EQ(returnValue, "http://PathParamTest");
}

/*
 * 测试用例： 查询替换正确
 * 前置条件： requestInfo.m_queryParams有值.
 * CHECK点： 判断返回值和预期字符串相等
 */
TEST_F(ClientTest, GetQueryParamsSuccess)
{
    std::string returnValue = m_clientMock.GetQueryParams(m_requestInfo.m_queryParams);
    EXPECT_EQ(returnValue, "query_param_1=queryParamTest_1&query_param_2=queryParamTest_2");
}

/*
 * 测试用例： 为request设置head
 * 前置条件： requestInfo.m_headerParams有值
 * CHECK点： 判断request中head的值和requestInfo.m_headerParams的值一致
 */
TEST_F(ClientTest, AddHeaderParamsSuccess)
{
    Module::HttpRequest request;
    m_clientMock.AddHeaderParams(request, m_requestInfo.m_headerParams);
    std::set<std::pair<std::string, std::string> >::iterator it= request.heads.begin();
    std::string key = it->first;
    std::string key_value = it->second;
    EXPECT_EQ(key, "Content-Type");
    EXPECT_EQ(key_value, "application/json");
}
}
