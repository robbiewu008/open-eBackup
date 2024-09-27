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
#include "JobExecution.h"
#include "BasicJob.h"
#include "JobMgr.h"

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
using namespace common::jobmanager;

class DemoBasicJob : public BasicJob {
public:
    DemoBasicJob() {}
    ~DemoBasicJob() {}
    int PrerequisiteJob() override { return 0; }
    int GenerateSubJob() override { return 0; }
    int ExecuteSubJob() override { return 0; }
    int PostJob() override { return 0; }
};

class JobExecutionTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void JobExecutionTest::SetUp() 
{
    
}
void JobExecutionTest::TearDown() {}
void JobExecutionTest::SetUpTestCase() {}
void JobExecutionTest::TearDownTestCase() {}

/*
 * 用例名称: 执行前置任务、分发子任务、执行子任务、后置任务
 * 前置条件：jobMgr检测jobId已存在
 * check点：返回成功
 */
TEST_F(JobExecutionTest, ExecuteJob)
{
    JobExecution jobExecution;
    AppProtect::ActionResult returnValue;
    string jobId = "JobExecutionTestjobId123";
    std::shared_ptr<BasicJob> jobPtr = std::make_shared<DemoBasicJob>();
    jobPtr->SetJobId(jobId);
    JobMgr::GetInstance().InsertJob(jobId, jobPtr);
    vector<OperType> operType = {OperType::PRE, OperType::GENERATE, 
                                OperType::EXECUTE, OperType::POST};
    for (auto type : operType) {
        int ret = jobExecution.ExecuteJob(returnValue, jobPtr, jobId, type);
        EXPECT_EQ(ret, Module::SUCCESS);
    }
}

TEST_F(JobExecutionTest, ExecPreJob)
{
    JobExecution jobExecution;
    AppProtect::ActionResult returnValue;
    string jobId = "JobExecutionTestPreJobId123";
    std::shared_ptr<BasicJob> jobPtr = std::make_shared<DemoBasicJob>();
    jobPtr->SetJobId(jobId);
    int ret = jobExecution.ExecuteJob(returnValue, jobPtr, jobId, OperType::PRE);
    EXPECT_EQ(ret, Module::SUCCESS);
}

TEST_F(JobExecutionTest, ExecGenSubJob)
{
    JobExecution jobExecution;
    AppProtect::ActionResult returnValue;
    string jobId = "JobExecutionTestGenSubJobId123";
    std::shared_ptr<BasicJob> jobPtr = std::make_shared<DemoBasicJob>();
    jobPtr->SetJobId(jobId);
    int ret = jobExecution.ExecuteJob(returnValue, jobPtr, jobId, OperType::GENERATE);
    EXPECT_EQ(ret, Module::SUCCESS);
}

TEST_F(JobExecutionTest, ExecSubJob)
{
    JobExecution jobExecution;
    AppProtect::ActionResult returnValue;
    string jobId = "JobExecutionTestSubJobId123";
    std::shared_ptr<BasicJob> jobPtr = std::make_shared<DemoBasicJob>();
    jobPtr->SetJobId(jobId);
    int ret = jobExecution.ExecuteJob(returnValue, jobPtr, jobId, OperType::EXECUTE);
    EXPECT_EQ(ret, Module::SUCCESS);
}

TEST_F(JobExecutionTest, ExecPostJob)
{
    JobExecution jobExecution;
    AppProtect::ActionResult returnValue;
    string jobId = "JobExecutionTestPostJobId123";
    std::shared_ptr<BasicJob> jobPtr = std::make_shared<DemoBasicJob>();
    jobPtr->SetJobId(jobId);
    int ret = jobExecution.ExecuteJob(returnValue, jobPtr, jobId, OperType::POST);
    EXPECT_EQ(ret, Module::SUCCESS);
}

TEST_F(JobExecutionTest, InitAsyncThread)
{
    JobExecution jobExecution;
    std::shared_ptr<std::thread> thread = nullptr;
    std::shared_ptr<BasicJob> jobPtr = std::make_shared<DemoBasicJob>();
    EXPECT_EQ(jobExecution.InitAsyncThread(jobPtr, thread, "InitAsyncThreadjobId123"), Module::FAILED);
    thread = std::make_shared<std::thread>(&BasicJob::PostJob, jobPtr);
    std::shared_ptr<BasicJob> jobPtr2 = std::make_shared<DemoBasicJob>();
    EXPECT_EQ(jobExecution.InitAsyncThread(jobPtr2, thread, "InitAsyncThreadjobId1234"), Module::SUCCESS);
}
