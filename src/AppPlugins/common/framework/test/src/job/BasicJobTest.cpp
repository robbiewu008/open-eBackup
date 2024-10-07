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

#include "BasicJob.h"
#include "JobCommonInfo.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

using namespace AppProtect;


class BasicJobTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BasicJob job;
};

void BasicJobTest::SetUp() {}
void BasicJobTest::TearDown() {}
void BasicJobTest::SetUpTestCase() {}
void BasicJobTest::TearDownTestCase() {}
namespace {
class DemoBasicJob : public BasicJob {
public:
    DemoBasicJob() {}
    ~DemoBasicJob() {}
    int PrerequisiteJob() override { return 0; }
    int GenerateSubJob() override { return 0; }
    int ExecuteSubJob() override { return 0; }
    int PostJob() override { return 0; }
};
}
TEST_F(BasicJobTest, ConstructDemoBasicJob)
{
    DemoBasicJob demo;
    // info
    BackupJob backupJob;
    RestoreJob restoreJob;
    BuildIndexJob buildIndexJob;

    auto jobInfoBackup = std::make_shared<JobCommonInfo>(std::make_shared<BackupJob>(backupJob));
    auto jobInfoRestore = std::make_shared<JobCommonInfo>(std::make_shared<RestoreJob>(restoreJob));
    auto jobInfoIndex = std::make_shared<JobCommonInfo>(std::make_shared<BuildIndexJob>(buildIndexJob));

    auto bj = jobInfoBackup->GetJobInfo();
    auto rj = jobInfoRestore->GetJobInfo();
    auto ij = jobInfoIndex->GetJobInfo();
    demo.SetJobInfo(jobInfoBackup);
    auto jobinfo = demo.GetJobInfo();
    // jobId
    std::string jobId = "new_job";
    std::string parJobId = "new_par_job";
    demo.SetJobId(jobId);
    auto retJobId = demo.GetJobId();
    demo.SetParentJobId(parJobId);
    auto retParJobId = demo.GetParentJobId();
    auto subjob = std::make_shared<SubJob>();
    demo.SetSubJob(subjob);
}

TEST_F(BasicJobTest, DetachJobThread)
{
    std::shared_ptr<std::thread> th1 = std::make_shared<std::thread>([](){});
    job.SetJobThread(th1);
    int ret = job.DetachJobThread();
    EXPECT_EQ(ret, Module::SUCCESS);

    std::shared_ptr<std::thread> th2 = std::make_shared<std::thread>([](){});
    job.SetJobThread(th2);
    job.GetJobThread()->detach();
    ret = job.DetachJobThread();
    EXPECT_EQ(ret, Module::SUCCESS);
}

TEST_F(BasicJobTest, EndJob)
{
    job.EndJob(AppProtect::SubJobStatus::COMPLETED);
    job.EndJob(AppProtect::SubJobStatus::INITIALIZING);
}

TEST_F(BasicJobTest, ExecuteJob)
{
    EXPECT_EQ(job.PrerequisiteJob(), Module::SUCCESS);
    EXPECT_EQ(job.GenerateSubJob(), Module::SUCCESS);
    EXPECT_EQ(job.ExecuteSubJob(), Module::SUCCESS);
    EXPECT_EQ(job.PostJob(), Module::SUCCESS);
}

TEST_F(BasicJobTest, GetJobInfo)
{
    EXPECT_EQ(job.GetSubJobId(), "");
    AppProtect::SubJob subJob;
    subJob.jobId = "jobId123";
    subJob.subJobId = "subJob123";
    job.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(job.GetSubJobId(), "subJob123");
    job.SetJobAborted();
    EXPECT_EQ(job.IsAbortJob(), true);
    job.SetJobToPause();
    EXPECT_EQ(job.IsJobPause(), true);
    job.SetProgress(100);
    job.SetJobToFinish();
    EXPECT_EQ(job.IsJobFinish(), true);
    job.SetPostJobResultType(AppProtect::JobResult::SUCCESS);
    EXPECT_EQ(job.m_jobResult, AppProtect::JobResult::SUCCESS);
}

TEST_F(BasicJobTest, RunStateMachine)
{
    job.m_nextState = 1;
    job.m_stateHandles[2] = [](){return Module::FAILED;};
    EXPECT_EQ(job.RunStateMachine(), Module::FAILED);
    job.m_nextState = 2;
    EXPECT_EQ(job.RunStateMachine(), Module::FAILED);
    EXPECT_EQ(job.AbortJob(), Module::SUCCESS);
}
