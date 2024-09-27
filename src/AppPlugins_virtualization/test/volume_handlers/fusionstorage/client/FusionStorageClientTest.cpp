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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include <common/Constants.h>
#include "common/CommonMock.h"
#include "volume_handlers/fusionstorage/client/FusionStorageClient.h"
#include "volume_handlers/fusionstorage/FusionStorageRestApiErrorCode.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ErrorCode = FusionStorageRestApiErrorCode;

using namespace HcsPlugin;

namespace HDT_TEST {

    static Module::IHttpClient *Stub_CallApi()
    {
        std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
        EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
        EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(g_httpStatusCode));
        EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
        EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
        EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
        EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(200));
        EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));

        std::set<std::string> GetHeadByNameReturn = {};
        EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
        std::string bodyString = "success";
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

    class FusionStorageClientTest : public testing::Test {
    public:
        void SetUp() {
            fusionStorageClientTest = std::make_shared<FusionStorageClient>();
        }
        void TearDown() {}

    public:
        std::shared_ptr<FusionStorageClient> fusionStorageClientTest;
    };

/**
 * 测试用例： 获取Token
 * 前置条件：
 * CHECK点： 成功
 */
    TEST_F(FusionStorageClientTest, GetTokenSuccess)
{
    GetFusionStorageRequest request;
    request.SetFusionStorageUserName("test");
    request.SetFusionStoragePassword("test");
    request.SetFusionStoragePort("8088");
    AuthObj storageAuth;
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    std::shared_ptr<ResponseModel> response = fusionStorageClientTest -> GetToken(request, storageAuth);
    std::string ret = response -> GetBody();
    EXPECT_EQ(ret, "success");
};
}  // namespace HDT_TEST