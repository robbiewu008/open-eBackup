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

#include "JobMgr.h"
#include "JobCommonInfo.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

using namespace common::jobmanager;

class JobMgrTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
private:
    std::string jobId;
    std::shared_ptr<BasicJob> jobPtr;
};

void JobMgrTest::SetUp() {
    jobId = "job_1";
    jobPtr = std::make_shared<BasicJob>();
    JobMgr::GetInstance().m_jobIdMap.clear();
}
void JobMgrTest::TearDown() {}
void JobMgrTest::SetUpTestCase() {}
void JobMgrTest::TearDownTestCase() {}

namespace {
    constexpr auto MODULE = "JobMgr";
    const int JOB_MGR_MONITOR_INTERVAL = 1;
    const std::string PREREQUISITE = "PRE";
    const std::string GENERATE = "GEN";
    const std::string POST = "PST";
    const std::vector<std::string> JOBPOSTFIX{PREREQUISITE, GENERATE, POST};
}

TEST_F(JobMgrTest, InsertJobTest)
{
    int ret;

    ret = JobMgr::GetInstance().InsertJob(jobId, nullptr);
    EXPECT_EQ(ret, Module::FAILED); // 任务为空，插入失败

    JobMgr::GetInstance().m_jobIdMap.emplace(jobId, jobPtr);
    ret = JobMgr::GetInstance().InsertJob(jobId, jobPtr); 
    EXPECT_EQ(ret, Module::FAILED); // 存在相同任务，插入失败

    JobMgr::GetInstance().m_jobIdMap.clear();
    ret = JobMgr::GetInstance().InsertJob(jobId, jobPtr);
    EXPECT_EQ(ret, Module::SUCCESS); // 插入任务成功
}

TEST_F(JobMgrTest, AsyncAbortJobTest)
{
    int ret;

    JobMgr::GetInstance().m_jobIdMap.emplace(jobId, jobPtr);
    ret = JobMgr::GetInstance().AsyncAbortJob("", jobId); 
    EXPECT_EQ(ret, Module::SUCCESS); // 找到对应任务并终止成功

    JobMgr::GetInstance().m_jobIdMap.clear();
    ret = JobMgr::GetInstance().AsyncAbortJob("", jobId); 
    EXPECT_EQ(ret, Module::FAILED); // 没有任务可以删除，返回失败

    std::string mgrJobId("_");
    mgrJobId.append(JOBPOSTFIX[0]);
    JobMgr::GetInstance().m_jobIdMap.emplace(mgrJobId, jobPtr);
    ret = JobMgr::GetInstance().AsyncAbortJob("", jobId); 
    EXPECT_EQ(ret, Module::SUCCESS); // 找到子任务并终止成功
}

TEST_F(JobMgrTest, PauseJobTest)
{
    int ret;

    JobMgr::GetInstance().m_jobIdMap.emplace(jobId, jobPtr);
    ret = JobMgr::GetInstance().PauseJob("", jobId); 
    EXPECT_EQ(ret, Module::SUCCESS); // 找到对应任务并暂停

    JobMgr::GetInstance().m_jobIdMap.clear();
    ret = JobMgr::GetInstance().PauseJob("", jobId); 
    EXPECT_EQ(ret, Module::FAILED); // 没有任务可以暂停，返回失败

    std::string mgrJobId("_");
    mgrJobId.append(JOBPOSTFIX[0]);
    JobMgr::GetInstance().m_jobIdMap.emplace(mgrJobId, jobPtr);
    ret = JobMgr::GetInstance().PauseJob("", jobId); 
    EXPECT_EQ(ret, Module::SUCCESS); // 找到子任务并暂停成功
}

TEST_F(JobMgrTest, EraseFinishJobTest)
{
    jobPtr->SetJobToFinish();
    JobMgr::GetInstance().m_jobIdMap.emplace(jobId, jobPtr);
    JobMgr::GetInstance().EraseFinishJob(); // 遍历删除已经结束的节点
}

// TEST_F(JobMgrTest, StartAndEndJobMonitorTest)
// {
//     JobMgr::GetInstance().m_jobIdMap.emplace(jobId, jobPtr);
//     JobMgr::GetInstance().SetMonitorInterval(10); // 设置监控间隔
//     int ret = JobMgr::GetInstance().StartMonitorJob(); 
//     EXPECT_EQ(ret, Module::SUCCESS); // 创建jobMgr线程成功
//     // JobMgr::GetInstance().EndJobMonitor(); // 结束jobMgr线程
// }

/*
 * 用例名称: jobMgr停止所有任务
 * 前置条件：
 * check点：任务状态已终止
 */
TEST_F(JobMgrTest, PauseAllJobTest)
{
    JobMgr::GetInstance().m_jobIdMap.emplace(jobId, jobPtr);
    EXPECT_EQ(JobMgr::GetInstance().m_jobIdMap.begin()->second->m_isPause, false);
    JobMgr::GetInstance().PauseAllJob();
    EXPECT_EQ(JobMgr::GetInstance().m_jobIdMap.begin()->second->m_isPause, true);
}
