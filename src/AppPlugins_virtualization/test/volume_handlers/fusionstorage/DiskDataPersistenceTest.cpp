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
#include "volume_handlers/fusionstorage/DiskDataPersistence.h"
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

using namespace VirtPlugin;

namespace HDT_TEST {
    class DiskDataPersistenceTest : public testing::Test {

    protected:
        void SetUp() {
            diskDataPersistenceTest = std::make_shared<DiskDataPersistence>();
        }
        void TearDown() {}

    public:
        std::shared_ptr<DiskDataPersistence> diskDataPersistenceTest;
    };

/**
 * 测试用例： 获取数据
 * 前置条件：
 * CHECK点： 成功
 */
    TEST_F(DiskDataPersistenceTest, GetObjectSuccess)
{
    const std::string key = "name";
    const std::string value = "disk1";
    diskDataPersistenceTest -> AddObject(key, value);
    diskDataPersistenceTest -> RemoveObject(key);
    diskDataPersistenceTest -> AddObject(key, value);

    std::string ret = diskDataPersistenceTest -> GetObject(key);
    EXPECT_EQ(ret, value);
};

/**
 * 测试用例： 获取数据
 * 前置条件：
 * CHECK点： 成功
 */
TEST_F(DiskDataPersistenceTest, AppendArrayElementSuccess)
{
const std::string key = "name";
const std::string value = "disk1";
diskDataPersistenceTest -> AppendArrayElement(key, value);
diskDataPersistenceTest -> RemoveArrayElement(key, value);
diskDataPersistenceTest -> AppendArrayElement(key, value);
std::string ret = diskDataPersistenceTest -> GetFirstArrayElement(key);
EXPECT_EQ(ret, value);
};

/**
 * 测试用例： 转成json
 * 前置条件：
 * CHECK点： 成功
 */
TEST_F(DiskDataPersistenceTest, ToJsonValueSuccess)
{
diskDataPersistenceTest -> GetCachedUUID();
diskDataPersistenceTest -> ToJsonString();
diskDataPersistenceTest -> Finish();
const std::string jsonStr = "{}";
bool ret = diskDataPersistenceTest -> ToJsonValue(jsonStr);
EXPECT_EQ(ret, false);
};
}  // namespace HDT_TEST