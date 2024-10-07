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
#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"
#include "application/ApplicationManager.h"

using namespace std;
using ::testing::_;
using testing::AllOf;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using testing::ByMove;
using testing::DoAll;
using ::testing::Eq;
using ::testing::Field;
using ::testing::Ge;
using ::testing::Gt;
using testing::InitGoogleMock;
using ::testing::Invoke;
using ::testing::Ne;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::SetArgumentPointee;
using ::testing::Throw;
using namespace AppProtect;

namespace FilePlugin {
class ApplicationManagerMock : public ApplicationManager {
public:
    ApplicationManagerMock() = default;
    virtual ~ApplicationManagerMock() noexcept {}
    void ListNativeResource(FilePlugin::FileResourceInfo& resourceInfo,
                            const FilePlugin::ListResourceParam& listResourceParam) override;
    void ListAggregateResource(FilePlugin::FileResourceInfo& resourceInfo,
                            const FilePlugin::ListResourceParam& listResourceParams) override;
    void ListVolumeResource(FilePlugin::FileResourceInfo& resourceInfo,
                            const FilePlugin::ListResourceParam& listResourceParams) override;
};

void ApplicationManagerMock::ListNativeResource(FileResourceInfo& resourceInfo,
                                                const ListResourceParam& listResourceParam)
{
    NasShareResourceInfo nasShareResourceInfo;
    nasShareResourceInfo.path = listResourceParam.path;
    nasShareResourceInfo.modifyTime = "1970-01-01 12:00:00";
    nasShareResourceInfo.size = listResourceParam.pageSize;
    resourceInfo.resourceDetailVec.push_back(nasShareResourceInfo);
    resourceInfo.totalCount = 1;
    return;
}

void ApplicationManagerMock::ListVolumeResource(FileResourceInfo& resourceInfo,
                                                   const ListResourceParam& listResourceParam)
{
    NasShareResourceInfo nasShareResourceInfo;
    nasShareResourceInfo.path = listResourceParam.path;
    nasShareResourceInfo.modifyTime = "1970-01-01 12:00:00";
    nasShareResourceInfo.size = listResourceParam.pageSize;
    resourceInfo.resourceDetailVec.push_back(nasShareResourceInfo);
    resourceInfo.totalCount = 1;
    return;
}

void ApplicationManagerMock::ListAggregateResource(FileResourceInfo& resourceInfo,
                                                   const ListResourceParam& listResourceParam)
{
    NasShareResourceInfo nasShareResourceInfo;
    nasShareResourceInfo.path = listResourceParam.path;
    nasShareResourceInfo.modifyTime = "1970-01-01 12:00:00";
    nasShareResourceInfo.size = listResourceParam.pageSize;
    resourceInfo.resourceDetailVec.push_back(nasShareResourceInfo);
    resourceInfo.totalCount = 1;
    return;
}
}

class ApplicationManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    FilePlugin::ApplicationManagerMock applicationManagerObj;
    ResourceResultByPage returnValue;
    Stub stub;
};

FilePlugin::ListResourceParam SetUpListResourceParam()
{
    FilePlugin::ListResourceParam listResourceParam;
    listResourceParam.applicationId = "AppId123";
    listResourceParam.sharePath = "";
    listResourceParam.path = "";
    listResourceParam.pageNo = 1;
    listResourceParam.pageSize = 20;
    listResourceParam.resourceExtendInfo.fileType = "native";  // value:native or aggregate
    return listResourceParam;
}

static bool StructToJsonString_False(void *obj)
{
    return false;
}

void ApplicationManagerTest::SetUp()
{}

void ApplicationManagerTest::TearDown()
{}

/*
 * 用例名称:ListApplicationResource原生格式
 * 前置条件：无
 * check点：returnValue结构体的内部属性
 */
TEST_F(ApplicationManagerTest, ListApplicationResource_native)
{
    FilePlugin::ListResourceParam listResourceParam = SetUpListResourceParam();
    applicationManagerObj.ListApplicationResource(returnValue, listResourceParam);
    EXPECT_EQ(returnValue.pageNo, listResourceParam.pageNo);
    EXPECT_EQ(returnValue.pageSize, listResourceParam.pageSize);
    EXPECT_EQ(returnValue.total, 1);
    EXPECT_EQ(returnValue.items.size(), 1);
}

/*
 * 用例名称: ListApplicationResource原生格式
 * 前置条件：给StructToJsonString打桩返回false
 * check点：returnValue结构体的内部属性
 */
TEST_F(ApplicationManagerTest, ListApplicationResource_native_St2JsonFalse)
{
    stub.set(Module::JsonHelper::StructToJsonString<FilePlugin::NasShareResourceInfo>, StructToJsonString_False);
    FilePlugin::ListResourceParam listResourceParam = SetUpListResourceParam();
    applicationManagerObj.ListApplicationResource(returnValue, listResourceParam);
    EXPECT_EQ(returnValue.pageNo, listResourceParam.pageNo);
    EXPECT_EQ(returnValue.pageSize, listResourceParam.pageSize);
    EXPECT_EQ(returnValue.total, 1);
    EXPECT_EQ(returnValue.items.size(), 0);
}

/*
 * 用例名称:ListApplicationResource聚合格式
 * 前置条件：
 * check点：returnValue结构体的内部属性
 */
TEST_F(ApplicationManagerTest, ListApplicationResource_aggregate)
{
    FilePlugin::ListResourceParam listResourceParam = SetUpListResourceParam();
    listResourceParam.resourceExtendInfo.fileType = "aggregate";
    applicationManagerObj.ListApplicationResource(returnValue, listResourceParam);
    EXPECT_EQ(returnValue.pageNo, listResourceParam.pageNo);
    EXPECT_EQ(returnValue.pageSize, listResourceParam.pageSize);
    EXPECT_EQ(returnValue.total, 1);
    EXPECT_EQ(returnValue.items.size(), 1);
}
