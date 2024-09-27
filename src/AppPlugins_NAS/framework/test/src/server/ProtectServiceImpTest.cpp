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
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "log/Log.h"
#include "ProtectServiceImp.h"
#include "BasicJob.h"
#include "OpenLibMgr.h"
#include "JobFactoryBase.h"

using namespace AppProtect;

class ProtectServiceImpTest : public testing::Test {
public:
    void SetUp() {
        m_servicePtr = std::make_unique<ProtectServiceImp>();
    };
    void TearDown() {
        GlobalMockObject::verify(); // 校验mock规范并清除mock规范
    };

private:
    std::unique_ptr<ProtectServiceImp> m_servicePtr {nullptr};
};

/*
 * 用例名称: 测试thrift同步接口通过dlopen加载动态库函数
 * 前置条件：未加载动态库
 * check点：测试打开动态库失败后的返回以及抛出异常
 */
TEST_F(ProtectServiceImpTest, AsyncAbortJob)
{
    ActionResult returnValue;
    std::string jobId;
    std::string subJobId;
    std::string appType;

    m_servicePtr->AsyncAbortJob(returnValue, jobId, subJobId, appType);
    EXPECT_EQ(returnValue.code, 0);
}

TEST_F(ProtectServiceImpTest, PauseJob)
{
    ActionResult returnValue;
    std::string jobId;
    std::string subJobId;
    std::string appType;

    m_servicePtr->PauseJob(returnValue, jobId, subJobId, appType);
    EXPECT_EQ(returnValue.code, 0);
}

void Stub_CheckBackupJob (ActionResult& actionResult, const BackupJob& job) 
{
    actionResult.__set_code(0);
};

TEST_F(ProtectServiceImpTest, CheckBackupJobType)
{
    ActionResult actionResult;
    BackupJob job;

    m_servicePtr->CheckBackupJobType(actionResult, job);
    EXPECT_EQ(actionResult.code, INNER_ERROR);

    // 成功获取到动态库函数
    MOCKER(Module::DlibDlsym)
        .stubs()
        .will(returnValue((void*)Stub_CheckBackupJob));
    m_servicePtr->CheckBackupJobType(actionResult, job);
    EXPECT_EQ(actionResult.code, 0);

    // test for param_check type COMMON_VALUE_UINT32
    BackupJobParam jobParam;
    Qos qos;
    qos.bandwidth = -1;
    jobParam.qos = qos;
    job.jobParam = jobParam;
    MOCKER(Module::DlibDlsym)
    .stubs()
    .will(returnValue((void*)Stub_CheckBackupJob));
    m_servicePtr->CheckBackupJobType(actionResult, job);
    EXPECT_EQ(actionResult.code, 1073948954);

    job.jobParam.qos.bandwidth = 0;
    MOCKER(Module::DlibDlsym)
    .stubs()
    .will(returnValue((void*)Stub_CheckBackupJob));
    m_servicePtr->CheckBackupJobType(actionResult, job);
    EXPECT_EQ(actionResult.code, 0);

    // test for param_check type COPY_NAME
    Copy copy;
    copy.name = "名字";
    job.copy = copy;
    MOCKER(Module::DlibDlsym)
    .stubs()
    .will(returnValue((void*)Stub_CheckBackupJob));
    m_servicePtr->CheckBackupJobType(actionResult, job);
    EXPECT_EQ(actionResult.code, 1073948954);

    job.copy.name = "abc";
    MOCKER(Module::DlibDlsym)
    .stubs()
    .will(returnValue((void*)Stub_CheckBackupJob));
    m_servicePtr->CheckBackupJobType(actionResult, job);
    EXPECT_EQ(actionResult.code, 0);

    // test for param_check type TIMESTAMP
    job.copy.timestamp = -1;
    MOCKER(Module::DlibDlsym)
    .stubs()
    .will(returnValue((void*)Stub_CheckBackupJob));
    m_servicePtr->CheckBackupJobType(actionResult, job);
    EXPECT_EQ(actionResult.code, 1073948954);

    job.copy.timestamp = 0;
    MOCKER(Module::DlibDlsym)
    .stubs()
    .will(returnValue((void*)Stub_CheckBackupJob));
    m_servicePtr->CheckBackupJobType(actionResult, job);
    EXPECT_EQ(actionResult.code, 0);

    // test for param_check type SNAPSHOT_ID
    std::vector<Snapshot> snapshots;
    Snapshot snapshot;
    snapshot.id = "中文";
    snapshots.push_back(snapshot);
    job.copy.snapshots = snapshots;
    MOCKER(Module::DlibDlsym)
    .stubs()
    .will(returnValue((void*)Stub_CheckBackupJob));
    m_servicePtr->CheckBackupJobType(actionResult, job);
    EXPECT_EQ(actionResult.code, 1073948954);

    job.copy.snapshots[0].id = "123";
    MOCKER(Module::DlibDlsym)
    .stubs()
    .will(returnValue((void*)Stub_CheckBackupJob));
    m_servicePtr->CheckBackupJobType(actionResult, job);
    EXPECT_EQ(actionResult.code, 0);

    // test for param_check type SNAPSHOT_PARENT_NAME
    job.copy.snapshots[0].parentName = "中文";
    MOCKER(Module::DlibDlsym)
    .stubs()
    .will(returnValue((void*)Stub_CheckBackupJob));
    m_servicePtr->CheckBackupJobType(actionResult, job);
    EXPECT_EQ(actionResult.code, 1073948954);
    job.copy.snapshots[0].parentName = "abc";
    MOCKER(Module::DlibDlsym)
    .stubs()
    .will(returnValue((void*)Stub_CheckBackupJob));
    m_servicePtr->CheckBackupJobType(actionResult, job);
    EXPECT_EQ(actionResult.code, 0);
}

void Stub_QueryJobPermission(JobPermission& returnJobPermission, 
    const ApplicationEnvironment& appEnv, 
    const Application& application)
{
    returnJobPermission.user = std::string("usr1");
}

TEST_F(ProtectServiceImpTest, QueryJobPermission)
{
    JobPermission returnJobPermission;
    ApplicationEnvironment appEnv;
    Application application;
    m_servicePtr->QueryJobPermission(returnJobPermission, appEnv, application);
    EXPECT_EQ(returnJobPermission.user, "");

    MOCKER(Module::DlibDlsym)
        .stubs()
        .will(returnValue((void*)Stub_QueryJobPermission));
    m_servicePtr->QueryJobPermission(returnJobPermission, appEnv, application);
    EXPECT_EQ(returnJobPermission.user, "usr1");
}

void Stub_AllowBackupInLocalNode(ActionResult& actionResult, const BackupJob& job, const BackupLimit::type limit) 
{
    actionResult.__set_code(0);
}

TEST_F(ProtectServiceImpTest, AllowBackupInLocalNode)
{
    ActionResult actionResult;
    BackupJob job;
    BackupLimit::type limit;

    m_servicePtr->AllowBackupInLocalNode(actionResult, job, limit);
    EXPECT_EQ(actionResult.code, INNER_ERROR);

    MOCKER(Module::DlibDlsym)
        .stubs()
        .will(returnValue((void*)Stub_AllowBackupInLocalNode));
    m_servicePtr->AllowBackupInLocalNode(actionResult, job, limit);
    EXPECT_EQ(actionResult.code, 0);
}

void Stub_AllowBackupSubJobInLocalNode(ActionResult& actionResult, const BackupJob& job, const SubJob& subJob) 
{
    actionResult.__set_code(0);
}

TEST_F(ProtectServiceImpTest, AllowBackupSubJobInLocalNode)
{
    ActionResult actionResult;
    BackupJob job;
    SubJob subJob;

    m_servicePtr->AllowBackupSubJobInLocalNode(actionResult, job, subJob);
    EXPECT_EQ(actionResult.code, INNER_ERROR);

    MOCKER(Module::DlibDlsym)
        .stubs()
        .will(returnValue((void*)Stub_AllowBackupSubJobInLocalNode));
    m_servicePtr->AllowBackupSubJobInLocalNode(actionResult, job, subJob);
    EXPECT_EQ(actionResult.code, 0);
}

void Stub_AllowRestoreInLocalNode(ActionResult& actionResult, const RestoreJob& job)
{
    actionResult.__set_code(0);
}

TEST_F(ProtectServiceImpTest, AllowRestoreInLocalNode)
{
    ActionResult actionResult;
    RestoreJob job;

    m_servicePtr->AllowRestoreInLocalNode(actionResult, job);
    EXPECT_EQ(actionResult.code, INNER_ERROR);

    MOCKER(Module::DlibDlsym)
        .stubs()
        .will(returnValue((void*)Stub_AllowRestoreInLocalNode));
    m_servicePtr->AllowRestoreInLocalNode(actionResult, job);
    EXPECT_EQ(actionResult.code, 0);
}

void Stub_AllowRestoreSubJobInLocalNode(ActionResult& actionResult, const RestoreJob& job, const SubJob& subJob)
{
    actionResult.__set_code(0);
}

TEST_F(ProtectServiceImpTest, AllowRestoreSubJobInLocalNode)
{
    ActionResult actionResult;
    RestoreJob job;
    SubJob subJob;

    m_servicePtr->AllowRestoreSubJobInLocalNode(actionResult, job, subJob);
    EXPECT_EQ(actionResult.code, INNER_ERROR);

    MOCKER(Module::DlibDlsym)
        .stubs()
        .will(returnValue((void*)Stub_AllowRestoreSubJobInLocalNode));
    m_servicePtr->AllowRestoreSubJobInLocalNode(actionResult, job, subJob);
    EXPECT_EQ(actionResult.code, 0);
}

void Stub_DeliverTaskStatus(ActionResult& actionResult,
    const std::string& status, const std::string& jobId, const std::string& script)
{
    actionResult.__set_code(0);
}

TEST_F(ProtectServiceImpTest, DeliverTaskStatus)
{
    ActionResult actionResult;
    std::string status;
    std::string jobId;
    std::string script;

    m_servicePtr->DeliverTaskStatus(actionResult, status, jobId, script);
    EXPECT_EQ(actionResult.code, INNER_ERROR);

    MOCKER(Module::DlibDlsym)
        .stubs()
        .will(returnValue((void*)Stub_DeliverTaskStatus));
    m_servicePtr->DeliverTaskStatus(actionResult, status, jobId, script);
    EXPECT_EQ(actionResult.code, 0);
}

class Stub_JobFactory: public JobFactoryBase {
public:
    std::shared_ptr<BasicJob> CreateJob(const std::shared_ptr<JobCommonInfo>& jobInfo, JobType jobType){
        return std::make_shared<BasicJob>();
    }
};

JobFactoryBase* Stub_JobFactory_Fun() {
    return new Stub_JobFactory();
};

JobFactoryBase* Stub_JobFactory_Fun_Return_Nullptr() {
    return nullptr;
};

class Stub_JobFactory_Return_Nullptr: public JobFactoryBase {
public:
    std::shared_ptr<BasicJob> CreateJob(const std::shared_ptr<JobCommonInfo>& jobInfo, JobType jobType){
        return nullptr;
    }
};

JobFactoryBase* Stub_JobFactory_Class_Return_Nullptr() {
    return new Stub_JobFactory_Return_Nullptr();
};

class ProtectServiceImpAsyncTest : public testing::Test {
public:
    void SetUp() {
        m_servicePtr = std::make_unique<ProtectServiceImp>();

        MOCKER(Module::DlibDlsym)
            .stubs()
            .will(returnValue((void*)Stub_JobFactory_Fun))
            .then(returnValue((void*)Stub_JobFactory_Fun_Return_Nullptr))
            .then(returnValue((void*)Stub_JobFactory_Class_Return_Nullptr));
    };
    void TearDown() {
        GlobalMockObject::verify(); // 校验mock规范并清除mock规范
    };

private:
    std::unique_ptr<ProtectServiceImp> m_servicePtr {nullptr};
};

/*
 * 用例名称: 测试thrift异步接口
 * 前置条件：未正常启动线程
 * check点：rpc返回失败
 */
// backcup
TEST_F(ProtectServiceImpAsyncTest, AsyncBackupPrerequisite)
{
    ActionResult actionResult;
    BackupJob job;

    // 执行正常
    m_servicePtr->AsyncBackupPrerequisite(actionResult, job);
    EXPECT_EQ(actionResult.code, 0);

    m_servicePtr->AsyncBackupPrerequisite(actionResult, job);
    EXPECT_EQ(actionResult.code, INNER_ERROR);

    m_servicePtr->AsyncBackupPrerequisite(actionResult, job);
    EXPECT_EQ(actionResult.code, INNER_ERROR);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncBackupGenerateSubJob)
{
    ActionResult returnValue;
    BackupJob job;
    int nodeNum;

    m_servicePtr->AsyncBackupGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncBackupGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncBackupGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncExecuteBackupSubJob)
{
    ActionResult returnValue;
    BackupJob job;
    SubJob subJob;

    m_servicePtr->AsyncExecuteBackupSubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncExecuteBackupSubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncExecuteBackupSubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncBackupPostJob)
{
    ActionResult returnValue;
    BackupJob job;
    SubJob subJob;
    JobResult::type jobResult;

    m_servicePtr->AsyncBackupPostJob(returnValue, job, subJob, jobResult);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncBackupPostJob(returnValue, job, subJob, jobResult);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncBackupPostJob(returnValue, job, subJob, jobResult);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

// restore
TEST_F(ProtectServiceImpAsyncTest, AsyncRestorePrerequisite)
{
    ActionResult returnValue;
    RestoreJob job;

    m_servicePtr->AsyncRestorePrerequisite(returnValue, job);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncRestorePrerequisite(returnValue, job);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncRestorePrerequisite(returnValue, job);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncRestoreGenerateSubJob)
{
    ActionResult returnValue;
    RestoreJob job;
    int nodeNum;

    m_servicePtr->AsyncRestoreGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncRestoreGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncRestoreGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncExecuteRestoreSubJob)
{
    ActionResult returnValue;
    RestoreJob job;
    SubJob subJob;

    m_servicePtr->AsyncExecuteRestoreSubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncExecuteRestoreSubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncExecuteRestoreSubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncRestorePostJob)
{
    ActionResult returnValue;
    RestoreJob job;
    SubJob subJob;
    JobResult::type jobResult;

    m_servicePtr->AsyncRestorePostJob(returnValue, job, subJob, jobResult);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncRestorePostJob(returnValue, job, subJob, jobResult);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncRestorePostJob(returnValue, job, subJob, jobResult);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

// livemount
TEST_F(ProtectServiceImpAsyncTest, AsyncLivemountGenerateSubJob)
{
    ActionResult returnValue;
    LivemountJob job;
    int nodeNum;

    m_servicePtr->AsyncLivemountGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncLivemountGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncLivemountGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncExecuteLivemountSubJob)
{
    ActionResult returnValue;
    LivemountJob job;
    SubJob subJob;

    m_servicePtr->AsyncExecuteLivemountSubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncExecuteLivemountSubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncExecuteLivemountSubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncCancelLivemountGenerateSubJob)
{
    ActionResult returnValue;
    CancelLivemountJob job;
    int nodeNum;

    m_servicePtr->AsyncCancelLivemountGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncCancelLivemountGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncCancelLivemountGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncExecuteCancelLivemountSubJob)
{
    ActionResult returnValue;
    CancelLivemountJob job;
    SubJob subJob;

    m_servicePtr->AsyncExecuteCancelLivemountSubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncExecuteCancelLivemountSubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncExecuteCancelLivemountSubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

// InstantRestore
TEST_F(ProtectServiceImpAsyncTest, AsyncInstantRestorePrerequisite)
{
    ActionResult returnValue;
    RestoreJob job;

    m_servicePtr->AsyncInstantRestorePrerequisite(returnValue, job);
    EXPECT_EQ(returnValue.code, 0);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncInstantRestoreGenerateSubJob)
{
    ActionResult returnValue;
    RestoreJob job;
    int nodeNum;

    m_servicePtr->AsyncInstantRestoreGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, 0);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncExecuteInstantRestoreSubJob)
{
    ActionResult returnValue;
    RestoreJob job;
    SubJob subJob;

    EXPECT_EQ(returnValue.code, 0);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncInstantRestorePostJob)
{
    ActionResult returnValue;
    RestoreJob job;
    SubJob subJob;
    JobResult::type jobResult;

    m_servicePtr->AsyncInstantRestorePostJob(returnValue, job, subJob, jobResult);
    EXPECT_EQ(returnValue.code, 0);
}

//build index
TEST_F(ProtectServiceImpAsyncTest, AsyncBuildIndexGenerateSubJob)
{
    ActionResult returnValue;
    BuildIndexJob job;
    int nodeNum;

    m_servicePtr->AsyncBuildIndexGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncBuildIndexGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncBuildIndexGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncBuildIndexSubJob)
{
    ActionResult returnValue;
    BuildIndexJob job;
    SubJob subJob;

    m_servicePtr->AsyncBuildIndexSubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncBuildIndexSubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncBuildIndexSubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

// copy
TEST_F(ProtectServiceImpAsyncTest, AsyncDelCopyGenerateSubJob)
{
    ActionResult returnValue;
    DelCopyJob job;
    int nodeNum;

    m_servicePtr->AsyncDelCopyGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncDelCopyGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncDelCopyGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncDelCopySubJob)
{
    ActionResult returnValue;
    DelCopyJob job;
    SubJob subJob;

    m_servicePtr->AsyncDelCopySubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncDelCopySubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncDelCopySubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncCheckCopyGenerateSubJob)
{
    ActionResult returnValue;
    CheckCopyJob job;
    int nodeNum;

    m_servicePtr->AsyncCheckCopyGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncCheckCopyGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncCheckCopyGenerateSubJob(returnValue, job, nodeNum);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

TEST_F(ProtectServiceImpAsyncTest, AsyncCheckCopySubJob)
{
    ActionResult returnValue;
    CheckCopyJob job;
    SubJob subJob;

    m_servicePtr->AsyncCheckCopySubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, 0);

    m_servicePtr->AsyncCheckCopySubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);

    m_servicePtr->AsyncCheckCopySubJob(returnValue, job, subJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}