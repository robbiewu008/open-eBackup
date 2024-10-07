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


#include "FilePlugin.h"
#include "common/application/ApplicationServiceDataType.h"
#include "common/Path.h"
#include "define/Types.h"
#include "utils/PluginUtilities.h"
#include "config_reader/ConfigIniReader.h"


using namespace std;
using namespace AppProtect;
using namespace FilePlugin;

class FilePluginTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void FilePluginTest::SetUp(){}

void FilePluginTest::TearDown(){}

void FilePluginTest::SetUpTestCase(){}

void FilePluginTest::TearDownTestCase(){}

/*
* 用例名称：AppInit
* 前置条件：无
* check点：日志log配置初始化
*/
TEST_F(FilePluginTest, AppInit)
{

    string logPath = "/xxx";
    int ret = AppInit(logPath);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/*
* 用例名称：DiscoverHostCluster
* 前置条件：无
* check点：找主机集群，函数体为空
*/
TEST_F(FilePluginTest, DiscoverHostCluster)
{
    ApplicationEnvironment returnEnv;
    ApplicationEnvironment appEnv;
    EXPECT_NO_THROW(DiscoverHostCluster(returnEnv, appEnv));
}

/*
* 用例名称：DiscoverAppCluster
* 前置条件：无
* check点：找应用集群，函数体为空，无返回值
*/
TEST_F(FilePluginTest, DiscoverAppCluster)
{

    ApplicationEnvironment returnEnv;
    ApplicationEnvironment appEnv;
    Application application;
    EXPECT_NO_THROW(DiscoverAppCluster(returnEnv, appEnv, application));
}

/*
* 用例名称：CreateFactory
* 前置条件：无
* check点：创建工厂
*/
TEST_F(FilePluginTest, CreateFactory)
{

    JobFactoryBase* ret = CreateFactory();
    EXPECT_EQ(static_cast<CommonJobFactory*>(ret) -> m_commonJobMap.size(), 12);
}

/*
* 用例名称：CheckApplication
* 前置条件：无
* check点：检查请求，函数体为空
*/
TEST_F(FilePluginTest, CheckApplication)
{

    ActionResult returnValue;
    ApplicationEnvironment appEnv;
    Application application;
    EXPECT_NO_THROW(CheckApplication(returnValue, appEnv, application));
}


/*
* 用例名称：ListApplicationResource
* 前置条件：无
* check点：列出请求资源，函数体为空
*/
TEST_F(FilePluginTest, ListApplicationResource)
{

    vector<ApplicationResource> returnValue;
    ApplicationEnvironment appEnv;
    Application application;
    ApplicationResource parentResource;
    EXPECT_NO_THROW(ListApplicationResource(returnValue, appEnv, application, parentResource));
}

/*
* 用例名称：ListApplicationResourceV2
* 前置条件：无
* check点：列出请求资源，函数体为空
*/
TEST_F(FilePluginTest, ListApplicationResourceV2)
{

    ResourceResultByPage returnValue;
    ListResourceRequest request;
    ListApplicationResourceV2(returnValue, request);

    EXPECT_EQ(request.applications.empty(), true);
}

/*
* 用例名称：CheckBackupJobType
* 前置条件：无
* check点：检查恢复工作类型，函数体为空
*/
TEST_F(FilePluginTest, CheckBackupJobType)
{

    ActionResult returnValue;
    AppProtect::BackupJob job;
    EXPECT_NO_THROW(CheckBackupJobType(returnValue, job));
}

/*
* 用例名称：AllowBackupInLocalNode
* 前置条件：无
* check点：允许在本地节点恢复，函数体为空
*/
TEST_F(FilePluginTest, AllowBackupInLocalNode)
{

    ActionResult returnValue;
    AppProtect::BackupJob job;
    AppProtect::BackupLimit::type limit;
    EXPECT_NO_THROW(AllowBackupInLocalNode(returnValue, job, limit));
}

/*
* 用例名称：AllowBackupSubJobInLocalNode
* 前置条件：无
* check点：函数体为空
*/
TEST_F(FilePluginTest, AllowBackupSubJobInLocalNode)
{

    ActionResult returnValue;
    AppProtect::BackupJob job;
    const AppProtect::SubJob subJob;

    EXPECT_NO_THROW(AllowBackupSubJobInLocalNode(returnValue, job, subJob));
}

/*
* 用例名称：AllowRestoreInLocalNode
* 前置条件：无
* check点：函数体为空
*/
TEST_F(FilePluginTest, AllowRestoreInLocalNode)
{

    ActionResult returnValue;
    const AppProtect::RestoreJob job;

    EXPECT_NO_THROW(AllowRestoreInLocalNode(returnValue, job));
}

/*
* 用例名称：AllowRestoreSubJobInLocalNode
* 前置条件：无
* check点：函数体为空
*/
TEST_F(FilePluginTest, AllowRestoreSubJobInLocalNode)
{

    ActionResult returnValue;
    const AppProtect::RestoreJob job;
    const AppProtect::SubJob subJob;

    EXPECT_NO_THROW(AllowRestoreSubJobInLocalNode(returnValue, job, subJob));
}
