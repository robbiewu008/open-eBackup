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
#include "statistics/ShareResourceManager.h"
#include "Module/src/common/JsonHelper.h"
#include <memory>

using ::testing::_;
using ::testing::Invoke;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ::testing::Mock;

using namespace FilePlugin;
namespace {
    const uint64_t COMMANDJOBTYPE_TEST = 999;
    struct TestResourceInfo {
        int64_t lastLogReportTime {0};
        uint64_t dirNum {0}; 
        uint64_t fileNum {0}; 
        uint64_t dirFailedNum {0}; 
        uint64_t fileFailedNum {0}; 
        uint64_t dataSize {0}; 

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(lastLogReportTime, lastLogReportTime)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(dirNum, dirNum)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(fileNum, fileNum)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(dirFailedNum, dirFailedNum)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(fileFailedNum, fileFailedNum)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(dataSize, dataSize)
        END_SERIAL_MEMEBER
    };
}

class ShareResourceManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

static void returnVoidStub(void* obj)
{
    return;
}

void ShareResourceManagerTest::SetUp()
{
    stub.set(sleep, returnVoidStub);
}

void ShareResourceManagerTest::TearDown()
{
}

void ShareResourceManagerTest::SetUpTestCase()
{
    ShareResourceManager::GetInstance().SetResourcePath("/home/statistics/", "jobId");
}

void ShareResourceManagerTest::TearDownTestCase()
{

}

/*
* 用例名称：初始化备份统计信息
* 前置条件：设置了统计信息存放目录
* check点：初始化备份统计信息成功
*/
TEST_F(ShareResourceManagerTest, Init_Backup_Static_SUCCESS)
{
    std::string jobId = "12345";
    TestResourceInfo resourceInfo;
    bool bResult = ShareResourceManager::GetInstance().InitResource(ShareResourceType::BACKUP, jobId, resourceInfo);
    EXPECT_EQ(bResult, true);
   
}

/*
* 用例名称：初始化扫描统计信息
* 前置条件：设置了统计信息存放目录
* check点：初始化扫描统计信息成功
*/
TEST_F(ShareResourceManagerTest, Init_Scan_Static_SUCCESS)
{
    std::string jobId = "12345";
    TestResourceInfo resourceInfo;
    bool bResult = ShareResourceManager::GetInstance().InitResource(ShareResourceType::SCAN, jobId, resourceInfo);
    EXPECT_EQ(bResult, true);
   
}

/*
* 用例名称：更新备份统计信息
* 前置条件：设置了统计信息存放目录
* check点：更新统计信息成功
*/
TEST_F(ShareResourceManagerTest, Update_Backup_Static_SUCCESS)
{
    std::string jobId = "12345";
    TestResourceInfo resourceInfo;
    resourceInfo.dirNum = 100;
    resourceInfo.fileNum = 300;
    resourceInfo.dirFailedNum = 5;
    bool bResult = ShareResourceManager::GetInstance().UpdateResource(ShareResourceType::BACKUP, jobId, resourceInfo);
    EXPECT_EQ(bResult, true);
}

static bool STUB_IsFileExist_FALSE(void* obj, std::string fileName)
{
    fileName.c_str();
    return false;
}

/*
* 用例名称：查询备份统计信息
* 前置条件：设置了统计信息存放目录
* check点：更新统计信息成功
*/
TEST_F(ShareResourceManagerTest, Query_Backup_Static_SUCCESS)
{
    std::string jobId = "12345";
    TestResourceInfo resourceInfo;
    resourceInfo.dirNum = 1000;
    resourceInfo.fileNum = 3000;
    resourceInfo.dirFailedNum = 50;
    bool bResult = ShareResourceManager::GetInstance().UpdateResource(ShareResourceType::BACKUP, jobId, resourceInfo);
    TestResourceInfo queryResourceResult;
    bool queryResult = ShareResourceManager::GetInstance().QueryResource(ShareResourceType::BACKUP, jobId, queryResourceResult);
    bool result = bResult && queryResult && queryResourceResult.dirNum == resourceInfo.dirNum;
    stub.set(ADDR(PluginUtils, IsFileExist), STUB_IsFileExist_FALSE);
    EXPECT_EQ(result, true);
}

/*
* 用例名称：删除备份统计信息
* 前置条件：设置了统计信息存放目录
* check点：删除统计信息成功
*/
TEST_F(ShareResourceManagerTest, Add_Backup_Static_SUCCESS)
{
    std::string jobId = "12345";
    TestResourceInfo resourceInfo;
    resourceInfo.dirNum = 2000;
    resourceInfo.fileNum = 5000;
    resourceInfo.dirFailedNum = 60;
    ShareResourceManager::GetInstance().Wait(ShareResourceType::BACKUP,jobId);
    bool bResult = ShareResourceManager::GetInstance().UpdateResource(ShareResourceType::BACKUP, jobId, resourceInfo);
    TestResourceInfo queryResourceResult;
    bool queryResult = ShareResourceManager::GetInstance().QueryResource(ShareResourceType::BACKUP, jobId, queryResourceResult);
    resourceInfo.dirNum += queryResourceResult.dirNum;
    resourceInfo.fileNum += queryResourceResult.fileNum;
    resourceInfo.dirFailedNum += queryResourceResult.dirFailedNum;
    bool updateAddRet = ShareResourceManager::GetInstance().UpdateResource(ShareResourceType::BACKUP, jobId, resourceInfo);
    bool queryAddResult = ShareResourceManager::GetInstance().QueryResource(ShareResourceType::BACKUP, jobId, queryResourceResult);
    ShareResourceManager::GetInstance().Signal(ShareResourceType::BACKUP,jobId);
    bool lastResult = updateAddRet && queryAddResult && queryResourceResult.dirNum == resourceInfo.dirNum;
    EXPECT_EQ(lastResult, true);
}

/*
* 用例名称：删除备份统计信息
* 前置条件：设置了统计信息存放目录
* check点：删除统计信息成功
*/
TEST_F(ShareResourceManagerTest, Delete_Static_SUCCESS)
{
    std::string jobId = "12345";
    bool bDelBackupResult = ShareResourceManager::GetInstance().DeleteResource(ShareResourceType::BACKUP, jobId);
    bool bDelScanResult = ShareResourceManager::GetInstance().DeleteResource(ShareResourceType::SCAN, jobId);
    EXPECT_EQ(bDelBackupResult&&bDelScanResult, true);
}

TEST_F(ShareResourceManagerTest, CanReportSpeedToPM)
{
    std::string jobId = "12345";
    bool ret = ShareResourceManager::GetInstance().CanReportSpeedToPM(jobId, 10);
    EXPECT_EQ(ret, true);
    ret = ShareResourceManager::GetInstance().CanReportSpeedToPM(jobId, 100);
    EXPECT_EQ(ret, false);
    sleep(1);
    ret = ShareResourceManager::GetInstance().CanReportSpeedToPM(jobId, 0);
    EXPECT_EQ(ret, true);
}
/*
* 用例名称：是否上报统计标签至PM
* 前置条件：无
* check点：可以上报/不可以上报
*/
TEST_F(ShareResourceManagerTest, CanReportStatToPM)
{
    std::string jobId = "123";
    bool canReport = ShareResourceManager::GetInstance().CanReportStatToPM(jobId);
    EXPECT_EQ(canReport, false);
    canReport = ShareResourceManager::GetInstance().CanReportStatToPM(jobId);
    EXPECT_EQ(canReport, false);
    ShareResourceManager::GetInstance().m_TaskReportMap[jobId] = 0;
    canReport = ShareResourceManager::GetInstance().CanReportStatToPM(jobId);
    EXPECT_EQ(canReport, true);
}

/*
* 用例名称：是否上打印Backup状态
* 前置条件：无
* check点：可以上报/不可以上报
*/
TEST_F(ShareResourceManagerTest, CanPrintBackupStats)
{
    std::string jobId = "1234";
    bool canReport = ShareResourceManager::GetInstance().CanPrintBackupStats(jobId);
    EXPECT_EQ(canReport, false);
    canReport = ShareResourceManager::GetInstance().CanPrintBackupStats(jobId);
    EXPECT_EQ(canReport, false);
    ShareResourceManager::GetInstance().m_taskPrintBackupStatsMap[jobId] = 0;
    canReport = ShareResourceManager::GetInstance().CanPrintBackupStats(jobId);
    EXPECT_EQ(canReport, true);
}

/*
* 用例名称：增加正在运行的子任务数
* 前置条件：无
* check点：子任务数
*/
TEST_F(ShareResourceManagerTest, IncreaseRunningSubTasks)
{
    std::string jobId = "12345";
    ShareResourceManager::GetInstance().IncreaseRunningSubTasks(jobId);
    EXPECT_EQ(ShareResourceManager::GetInstance().m_noOfRuningSubTasks[jobId], 1);
    ShareResourceManager::GetInstance().IncreaseRunningSubTasks(jobId);
    EXPECT_EQ(ShareResourceManager::GetInstance().m_noOfRuningSubTasks[jobId], 2);
}

/*
* 用例名称：减少正在运行的子任务数
* 前置条件：无
* check点：子任务数
*/
TEST_F(ShareResourceManagerTest, DecreaseRunningSubTasks)
{
    std::string jobId = "123456";
    ShareResourceManager::GetInstance().DecreaseRunningSubTasks(jobId);
    EXPECT_EQ(ShareResourceManager::GetInstance().m_noOfRuningSubTasks[jobId], 0);

    ShareResourceManager::GetInstance().m_noOfRuningSubTasks[jobId] = 0;
    ShareResourceManager::GetInstance().DecreaseRunningSubTasks(jobId);
    EXPECT_EQ(ShareResourceManager::GetInstance().m_noOfRuningSubTasks[jobId], 0);

    ShareResourceManager::GetInstance().m_noOfRuningSubTasks[jobId] = 5;
    ShareResourceManager::GetInstance().DecreaseRunningSubTasks(jobId);
    EXPECT_EQ(ShareResourceManager::GetInstance().m_noOfRuningSubTasks[jobId], 4);
}

/*
* 用例名称：获取正在运行的子任务数
* 前置条件：无
* check点：子任务数
*/
TEST_F(ShareResourceManagerTest, QueryRunningSubTasks)
{
    std::string jobId = "1234567";
    ShareResourceManager::GetInstance().QueryRunningSubTasks(jobId);
    EXPECT_EQ(ShareResourceManager::GetInstance().m_noOfRuningSubTasks[jobId], 0);

    ShareResourceManager::GetInstance().m_noOfRuningSubTasks[jobId] = 0;
    ShareResourceManager::GetInstance().QueryRunningSubTasks(jobId);
    EXPECT_EQ(ShareResourceManager::GetInstance().m_noOfRuningSubTasks[jobId], 0);

    ShareResourceManager::GetInstance().m_noOfRuningSubTasks[jobId] = 5;
    ShareResourceManager::GetInstance().QueryRunningSubTasks(jobId);
    EXPECT_EQ(ShareResourceManager::GetInstance().m_noOfRuningSubTasks[jobId], 5);
}