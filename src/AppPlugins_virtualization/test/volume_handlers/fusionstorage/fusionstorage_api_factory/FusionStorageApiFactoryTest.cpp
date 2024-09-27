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
#include <vector>
#include <string>
#include <system/System.hpp>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "common/Macros.h"
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
class FusionStorageApiFactoryTest : public testing::Test {
protected:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void FusionStorageApiFactoryTest::SetUp() {}
void FusionStorageApiFactoryTest::TearDown() {}
void FusionStorageApiFactoryTest::SetUpTestCase() {}
void FusionStorageApiFactoryTest::TearDownTestCase() {}

/**
 * 用例名称：创建VBSApi实例成功
 * 前置条件：无
 * check点：成功创建出VBSApi块客户端接口
 */
TEST_F(FusionStorageApiFactoryTest, CreateVBSApiSUCCESS)
{
    std::string apiMode = "VBS";
    std::string mgrIp = "111";
    std::string poolID = "222";

    std::shared_ptr<FusionStorageApi> fusionStorageApiPtr = nullptr;
    fusionStorageApiPtr = FusionStorageApiFactory::GetInstance()->CreateFusionStorageApi(apiMode, mgrIp, poolID);

    bool retValue = false;
    retValue = (fusionStorageApiPtr.get() == nullptr);
    EXPECT_EQ(retValue, false);
}

/**
 * 用例名称：创建VBSApi实例失败
 * 前置条件：无
 * check点：未能创建出VBSApi块客户端接口
 */
TEST_F(FusionStorageApiFactoryTest, CreateVBSApiFAILED)
{
    std::string apiMode = "VBS_ERR";
    std::string mgrIp = "111";
    std::string poolID = "222";
    
    std::shared_ptr<FusionStorageApi> fusionStorageApiPtr = nullptr;
    fusionStorageApiPtr = FusionStorageApiFactory::GetInstance()->CreateFusionStorageApi(apiMode, mgrIp, poolID);
    
    bool retValue = false;
    retValue = (fusionStorageApiPtr.get() == nullptr);
    EXPECT_EQ(retValue, true);
}

/**
 * 用例名称：创建RestApi实例成功
 * 前置条件：无
 * check点：成功创建出RestApiOperator
 */
TEST_F(FusionStorageApiFactoryTest, CreateRestApiSUCCESS){
    std::string apiMode = "ISCSI";
    std::string mgrIp = "111";
    std::string poolID = "222";

    std::shared_ptr<FusionStorageApi> fusionStorageApiPtr = nullptr;
    fusionStorageApiPtr = FusionStorageApiFactory::GetInstance()->CreateFusionStorageApi(apiMode, mgrIp, poolID);
    
    bool retValue = false;
    retValue = (fusionStorageApiPtr.get() == nullptr);
    EXPECT_EQ(retValue, false);
}

/**
 * 用例名称：创建RestApi实例失败
 * 前置条件：无
 * check点：成功创建出RestApiOperator
 */
TEST_F(FusionStorageApiFactoryTest, CreateRestApiFAILED)
{
    std::string apiMode = "ISCSI_ERR";
    std::string mgrIp = "111";
    std::string poolID = "222";

    std::shared_ptr<FusionStorageApi> fusionStorageApiPtr = nullptr;
    fusionStorageApiPtr = FusionStorageApiFactory::GetInstance()->CreateFusionStorageApi(apiMode, mgrIp, poolID);
    
    bool retValue = false;
    retValue = (fusionStorageApiPtr.get() == nullptr);
    EXPECT_EQ(retValue, true);
}
}  // namespace HDT_TEST