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
#include "volume_handlers/fusionstorage/client/FusionStorageRestClient.h"
#include "volume_handlers/fusionstorage/client/GetFusionStorageRequest.h"

using namespace HcsPlugin;
namespace HDT_TEST {
    class FusionStorageRestClientTest : public testing::Test {
    public:
        void SetUp() {
            fusionStorageClientTest = std::make_shared<FusionStorageRestClient>();
        }
        void TearDown() {}

    public:
        std::shared_ptr<FusionStorageRestClient> fusionStorageClientTest;
    };

/**
 * 测试用例：
 * 前置条件：
 * CHECK点： 成功
 */
    TEST_F(FusionStorageRestClientTest, CheckParamsSuccess)
{
    FusionStorageRestClient fusionStorageRestClient;
    GetFusionStorageRequest modelRequest;;
    std::string tokenStr;
    bool ret = fusionStorageRestClient.CheckParams(modelRequest);
    EXPECT_EQ(ret, true);
};
}  // namespace HDT_TEST
