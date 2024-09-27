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

#include <common/JsonHelper.h>
#include "common/CommonMock.h"

#include "volume_handlers/fusionstorage/FusionStorageRestApiOperator.h"
#include "volume_handlers/fusionstorage/FusionStorageRestApiErrorCode.h"
#include "volume_handlers/fusionstorage/client/FusionStorageRestApiRequest.h"
#include "volume_handlers/fusionstorage/client/FusionStorageRestClient.h"

#include "volume_handlers/fusionstorage/FusionStorageRestApiOperator.h"
#include "volume_handlers/fusionstorage/fusionstorage_api_factory/FusionStorageApiFactory.h"

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
class FusionStorageRestApiRequestTest : public testing::Test {
protected:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void FusionStorageRestApiRequestTest::SetUp()
{}
void FusionStorageRestApiRequestTest::TearDown()
{}
void FusionStorageRestApiRequestTest::SetUpTestCase()
{}
void FusionStorageRestApiRequestTest::TearDownTestCase()
{}

/**
 * 用例名称：创建请求体成功
 * 前置条件：无
 * check点：成功创建出请求体
 */
TEST_F(FusionStorageRestApiRequestTest, RestRequestIntanceSuccess)
{
    std::shared_ptr<FusionStorageRestApiRequest> restApiRequestPtr = nullptr;
    restApiRequestPtr = std::make_shared<FusionStorageRestApiRequest>();

    bool retValue = false;
    retValue = (restApiRequestPtr.get() == nullptr);
    EXPECT_EQ(retValue, false);
}

/**
 * 测试用例： 获得ScopeType
 * 前置条件： 无
 * CHECK点： 获得ScopeType成功
 */
TEST_F(FusionStorageRestApiRequestTest, GetScopeTypeSuccess)
{
    std::shared_ptr<FusionStorageRestApiRequest> restApiRequestPtr = std::make_shared<FusionStorageRestApiRequest>();

    bool retValue = false;
    retValue = (restApiRequestPtr->GetScopeType() == Scope::NONE);
    EXPECT_EQ(retValue, true);
}
/**
 * 测试用例： 获得ApiType
 * 前置条件： 无
 * CHECK点： 获得ApiType成功
 */
TEST_F(FusionStorageRestApiRequestTest, GetApiTypeSuccess)
{
    std::shared_ptr<FusionStorageRestApiRequest> restApiRequestPtr = std::make_shared<FusionStorageRestApiRequest>();

    bool retValue = false;
    retValue = (restApiRequestPtr->GetApiType() == ApiType::FUSIONSTORAGE);
    EXPECT_EQ(retValue, true);
}

/**
 * 测试用例： 获得tokentype
 * 前置条件： 无
 * CHECK点： 获得tokentype
 */
TEST_F(FusionStorageRestApiRequestTest, GetApiTokenTypeSuccess)
{
    std::shared_ptr<FusionStorageRestApiRequest> restApiRequestPtr = std::make_shared<FusionStorageRestApiRequest>();

    bool retValue = false;
    restApiRequestPtr->m_tokenType = Scope::NONE;
    retValue = (restApiRequestPtr->GetTokenType() == Scope::NONE);
    EXPECT_EQ(retValue, true);
}

/**
 * 测试用例： 获得token
 * 前置条件： 无
 * CHECK点： 获得token
 */
TEST_F(FusionStorageRestApiRequestTest, GetTokenSuccess)
{
    std::shared_ptr<FusionStorageRestApiRequest> restApiRequestPtr = std::make_shared<FusionStorageRestApiRequest>();

    bool retValue = false;
    const std::string token = "fakfar14r3u8049thjfd";
    restApiRequestPtr->m_token = "fakfar14r3u8049thjfd";
    retValue = (restApiRequestPtr->GetToken() == token);
    EXPECT_EQ(retValue, true);
}

/**
 * 测试用例： 传入tokentype
 * 前置条件： 无
 * CHECK点： 传入tokentype
 */
TEST_F(FusionStorageRestApiRequestTest, SetTokenSuccess)
{
    std::shared_ptr<FusionStorageRestApiRequest> restApiRequestPtr = std::make_shared<FusionStorageRestApiRequest>();

    bool retValue = false;
    const std::string token = "fakfar14r3u8049thjfd";
    restApiRequestPtr->SetToken(token);
    retValue = (restApiRequestPtr->m_token == token);
    EXPECT_EQ(retValue, true);
}

/**
 * 测试用例： 获得管理端口
 * 前置条件： 无
 * CHECK点： 获得管理端口
 */

TEST_F(FusionStorageRestApiRequestTest, GetMgrPortSuccess)
{
    std::shared_ptr<FusionStorageRestApiRequest> restApiRequestPtr = std::make_shared<FusionStorageRestApiRequest>();

    bool retValue = false;
    const std::string mgrPort = "192.168.1.1:8088";
    restApiRequestPtr->m_mgrPort = mgrPort;
    retValue = (restApiRequestPtr->GetMgrPort() == mgrPort);
    EXPECT_EQ(retValue, true);
}
/**
 * 测试用例： 传入管理端口
 * 前置条件： 无
 * CHECK点： 传入管理端口
 */

TEST_F(FusionStorageRestApiRequestTest, SetMgrPortSuccess)
{
    std::shared_ptr<FusionStorageRestApiRequest> restApiRequestPtr = std::make_shared<FusionStorageRestApiRequest>();

    bool retValue = false;
    const std::string mgrPort = "192.168.1.1:8088";
    restApiRequestPtr->SetMgrPort(mgrPort);
    retValue = (restApiRequestPtr->m_mgrPort == mgrPort);
    EXPECT_EQ(retValue, true);
}

}  // namespace HDT_TEST