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
#include "volume_handlers/fusionstorage/FusionStorageCleanFile.h"
#include "common/JsonUtils.h"

using namespace VirtPlugin;

namespace HDT_TEST {
    class FusionStorageCleanFileTest : public testing::Test {

    protected:
        void SetUp() {
            fusionStorageCleanFileTest = std::make_shared<FusionStorageCleanFile>();
        }
        void TearDown() {}

    public:
        std::shared_ptr<FusionStorageCleanFile> fusionStorageCleanFileTest;
    };

/**
 * 用例名称：检查主机是否存在,失败
 * 前置条件：无
 * check点：检查主机是否存在
 */
    TEST_F(FusionStorageCleanFileTest, AddOrReplaceItemNoLockFailed)
{
    Json::Value itemJson;
    itemJson["name"] = "iscsi";
    const std::string name = "name";
    const std::string value = "iscsi";
    fusionStorageCleanFileTest -> RemoveSpecificArrayElement(itemJson, name, value);

    const std::string key = "key1";
    const std::string item = "item1";
    const bool busy = true;
    fusionStorageCleanFileTest -> SetAllItemIdle();
    int32_t ret = fusionStorageCleanFileTest -> AddOrReplaceItemNoLock(key, item, busy);
    EXPECT_EQ(ret, -1);
}
}  // namespace HDT_TEST