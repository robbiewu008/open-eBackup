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
#include <list>
#include <cstdio>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"

#include <securec.h>
#include <job/JobCommonInfo.h>
#include "ClientInvoke.h"
#include <log/Log.h>

#include <common/Macros.h>
#include <common/Structs.h>

#include "job_controller/factory/VirtualizationJobFactory.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::A;

using namespace VirtPlugin;

namespace {
    const std::string MODULE_NAME = "VirtualizationJobFactoryTest";
}

namespace HDT_TEST {
class VirtualizationJobFactoryTest : public testing::Test {
public:
    void SetUp() {
        InitLogger();
    }
    void TearDown() {}
    void InitLogger()
    {
        std::string logFileName = "virt_plugin_test.log";
        std::string logFilePath = "/tmp/log/";
        int logLevel = DEBUG;
        int logFileCount = 10;
        int logFileSize = 30;
        Module::CLogger::GetInstance().Init(logFileName.c_str(), logFilePath, logLevel, logFileCount, logFileSize);
    }
};

/*
 * 测试用例：创建备份任务实例
 * 前置条件：JobType正确
 * CHECK点：创建备份任务实例成功
 */
TEST_F(VirtualizationJobFactoryTest, CreateVirt_BackupJob_Success)
{
    INFOLOG("Case: CreateVirt_BackupJob_Success");
    AppProtect::BackupJob backupJobInfo;
    backupJobInfo.jobParam.backupType = AppProtect::BackupJobType::FULL_BACKUP;
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(backupJobInfo);
    jobInfo->SetJobInfo(data);

    JobType jobTpye = JobType::BACKUP;
    std::shared_ptr<BasicJob> backupJob = VirtualizationJobFactory::GetInstance()->CreateJob(jobInfo, jobTpye);
    EXPECT_NE(nullptr, backupJob);
    INFOLOG("Case: CreateVirt_BackupJob_Success END");
}

/*
 * 测试用例：创建备份任务实例
 * 前置条件：JobType错误
 * CHECK点：创建备份任务实例失败
 */
TEST_F(VirtualizationJobFactoryTest, CreateVirt_BackupJob_Failed)
{
    INFOLOG("Case: CreateVirt_BackupJob_Failed");
    AppProtect::BackupJob backupJobInfo;
    backupJobInfo.jobParam.backupType = AppProtect::BackupJobType::FULL_BACKUP;
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(backupJobInfo);
    jobInfo->SetJobInfo(data);

    JobType jobTpye = JobType::UNDEFINED_JOB_TYPE;
    std::shared_ptr<BasicJob> backupJob = VirtualizationJobFactory::GetInstance()->CreateJob(jobInfo, jobTpye);
    EXPECT_EQ(nullptr, backupJob);
    INFOLOG("Case: CreateVirt_BackupJob_Failed END");
}

/*
 * 测试用例：创建恢复任务实例
 * 前置条件：JobType正确
 * CHECK点：创建恢复任务实例成功
 */
/*
TEST_F(VirtualizationJobFactoryTest, CreateVirt_RestoreJob_Success)
{
    INFOLOG("Case: CreateVirt_RestoreJob_Success");
    AppProtect::BackupJob backupJobInfo;
    backupJobInfo.jobParam.backupType = AppProtect::BackupJobType::FULL_BACKUP;
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(backupJobInfo);
    jobInfo->SetJobInfo(data);

    JobType jobTpye = JobType::RESTORE;
    std::shared_ptr<BasicJob> backupJob = VirtualizationJobFactory::GetInstance()->CreateJob(jobInfo, jobTpye);
    EXPECT_NE(nullptr, backupJob);
    INFOLOG("Case: CreateVirt_RestoreJob_Success END");
}
*/
/*
 * 测试用例：创建恢复任务实例
 * 前置条件：JobType错误
 * CHECK点：创建恢复任务实例失败
 */
TEST_F(VirtualizationJobFactoryTest, CreateVirt_RestoreJob_Failed)
{
    INFOLOG("Case: CreateVirt_RestoreJob_Failed");
    AppProtect::BackupJob backupJobInfo;
    backupJobInfo.jobParam.backupType = AppProtect::BackupJobType::FULL_BACKUP;
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(backupJobInfo);
    jobInfo->SetJobInfo(data);

    JobType jobTpye = JobType::UNDEFINED_JOB_TYPE;
    std::shared_ptr<BasicJob> backupJob = VirtualizationJobFactory::GetInstance()->CreateJob(jobInfo, jobTpye);
    EXPECT_EQ(nullptr, backupJob);
    INFOLOG("Case: CreateVirt_RestoreJob_Failed END");
}

static void StubJobServiceReportJobDetails(void *obj, ActionResult& result, const SubJobDetails& subJobDetails)
{
    return;
}

/*
 * 测试用例：上报日志到Agent
 * 前置条件：ParentID、SubJobID不为空
 * CHECK点：上报日志到Agent成功
 */
TEST_F(VirtualizationJobFactoryTest, ReportLog2AgentSuccess)
{
    Stub stub;
    std::string parentId = "123-456-789";
    std::string subJobId = "111-222-333";

    stub.set(ADDR(JobService, ReportJobDetails), StubJobServiceReportJobDetails);
    VirtualizationJobFactory::GetInstance()->ReportLog2Agent(parentId, subJobId);
}

/*
 * 测试用例：上报日志到Agent
 * 前置条件：SubJobID为空
 * CHECK点：上报日志到Agent成功
 */
TEST_F(VirtualizationJobFactoryTest, ReportLog2AgentSuccessSubJobIdNull)
{
    Stub stub;
    std::string parentId = "123-456-789";
    std::string subJobId = "";

    stub.set(ADDR(JobService, ReportJobDetails), StubJobServiceReportJobDetails);
    VirtualizationJobFactory::GetInstance()->ReportLog2Agent(parentId, subJobId);
}
}