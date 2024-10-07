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
#include "protect_engines/hcs/utils/DistributedStorageMgr.h"
#include "common/CommonMock.h"
#include "stub.h"

using namespace HcsPlugin;
 
 /*
 * 测试用例： 检查存储联通性
 * 前置条件： 消息发送成功
 * CHECK点： getapi失败
 */
namespace HDT_TEST {
    class DistributedStorageMgrTest : public testing::Test {
    protected:
        void SetUp() {}
        void TearDown() {}
        
    };

    std::string httpBodyString = "";
    uint32_t httpCode = 200;

    Module::IHttpClient *Stub_CallApi()
    {
        std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
        EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
        EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(g_httpStatusCode));
        EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
        EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
        EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
        EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(httpCode));
        EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));

        std::set<std::string> GetHeadByNameReturn = {};
        EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
        std::string bodyString = httpBodyString;
        EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(bodyString));
        std::set<std::string> headerValue;
        std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
        EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
        std::set<std::string> getCookiesReturn = {};
        EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

        IHttpClientMock *httpClient = new (std::nothrow) IHttpClientMock();
        EXPECT_CALL(*httpClient, SendMemCertRequest(_, _)).WillRepeatedly(Return(httpRespone));
        EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
        return httpClient;
    }

    TEST_F(DistributedStorageMgrTest, CheckDistributedConnectionFalied)
    {
        Stub stub;
        stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
        httpBodyString = "{}";
        httpCode = 200;

        ControlDeviceInfo controlDeviceInfo;
        controlDeviceInfo.m_ip = "127.0.0.1";
        controlDeviceInfo.m_ipList = {"127.0.0.1"};
        controlDeviceInfo.m_port = "80";
        controlDeviceInfo.m_userName = "name";
        controlDeviceInfo.m_password = "123456";
        std::string errorStr;
        int ret = DistributedStorageMgr::CheckDistributedConnection(controlDeviceInfo, errorStr);
        EXPECT_TRUE(ret == Module::FAILED);
    }

    TEST_F(DistributedStorageMgrTest, CheckDistributedConnectionSuccess)
    {
        Stub stub;
        stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
        httpBodyString = "{x_auth_token}";
        httpCode = 200;

        ControlDeviceInfo controlDeviceInfo;
        controlDeviceInfo.m_ip = "127.0.0.1";
        controlDeviceInfo.m_ipList = {"127.0.0.1","127.0.0.2"};
        controlDeviceInfo.m_port = "80";
        controlDeviceInfo.m_userName = "name";
        controlDeviceInfo.m_password = "123456";
        std::string errorStr;
        int ret = DistributedStorageMgr::CheckDistributedConnection(controlDeviceInfo, errorStr);
        EXPECT_TRUE(ret == Module::SUCCESS);
    }
}
