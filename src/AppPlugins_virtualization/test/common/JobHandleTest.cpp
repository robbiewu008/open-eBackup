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
#include "json/json.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "common/Structs.h"

using namespace VirtPlugin;
namespace HDT_TEST {
class JobHandleTest : public testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

public:
    AppProtect::BackupJob m_backupJobInfo;
    AppProtect::RestoreJob m_restoreJobInfo;
};

/* ----------- Test GetAppEnv ----------- */
/*
 * 用例名称：获取应用环境信息失败
 * 前置条件：任务类型错误
 * check点：环境信息获取失败
 */
TEST_F(JobHandleTest, GetAppEnvUndefinedType)
{
    JobType jobType = JobType::UNDEFINED_JOB_TYPE;
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = nullptr;
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    AppProtect::ApplicationEnvironment appEnv = jobHandle.GetAppEnv();
    EXPECT_EQ(jobHandle.GetJobType(), JobType::UNDEFINED_JOB_TYPE);
    EXPECT_EQ(appEnv.id, "");
}

/*
 * 用例名称：获取备份任务环境信息成功
 * 前置条件：任务类型正确，备份参数正确
 * check点：环境信息ID正确
 */
TEST_F(JobHandleTest, GetAppEnvBACKUPSuccess)
{
    JobType jobType = JobType::BACKUP;
    m_backupJobInfo.protectEnv.id = "123";
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupJobInfo);
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    AppProtect::ApplicationEnvironment appEnv = jobHandle.GetAppEnv();
    EXPECT_EQ(appEnv.id, "123");
}

/*
 * 用例名称：获取备份任务环境信息失败
 * 前置条件：任务类型正确，备份参数为nullptr
 * check点：环境信息ID为空
 */
TEST_F(JobHandleTest, GetAppEnvBACKUPFailed)
{
    JobType jobType = JobType::BACKUP;
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = nullptr;
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    AppProtect::ApplicationEnvironment appEnv = jobHandle.GetAppEnv();
    EXPECT_EQ(appEnv.id, "");
}

/*
 * 用例名称：获取恢复任务环境信息成功
 * 前置条件：任务类型正确，备份参数正确
 * check点：ID正确
 */
TEST_F(JobHandleTest, GetAppEnvRESTORESuccess)
{
    JobType jobType = JobType::RESTORE;
    m_restoreJobInfo.targetEnv.id = "123";
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreJobInfo);
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    AppProtect::ApplicationEnvironment appEnv = jobHandle.GetAppEnv();
    EXPECT_EQ(appEnv.id, "123");
}

/*
 * 用例名称：获取恢复任务环境信息失败
 * 前置条件：任务类型正确，备份参数为nullptr
 * check点：环境信息ID为空
 */
TEST_F(JobHandleTest, GetAppEnvRESTOREFailed)
{
    JobType jobType = JobType::RESTORE;
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = nullptr;
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    AppProtect::ApplicationEnvironment appEnv = jobHandle.GetAppEnv();
    EXPECT_EQ(appEnv.id, "");
}

/* ----------- Test GetApp ----------- */
/*
 * 用例名称：获取应用信息失败
 * 前置条件：任务类型设置错误
 * check点：ID为空
 */
TEST_F(JobHandleTest, GetAppUndefinedType)
{
    JobType jobType = JobType::UNDEFINED_JOB_TYPE;
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = nullptr;
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    AppProtect::Application app = jobHandle.GetApp();
    EXPECT_EQ(jobHandle.GetJobType(), JobType::UNDEFINED_JOB_TYPE);
    EXPECT_EQ(app.id, "");
}

/*
 * 用例名称：获取备份任务应用信息成功
 * 前置条件：任务类型正确、应用信息正确
 * check点：ID符合预期
 */
TEST_F(JobHandleTest, GetAppBACKUPSuccess)
{
    JobType jobType = JobType::BACKUP;
    m_backupJobInfo.protectObject.id = "123";
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupJobInfo);
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    AppProtect::Application app = jobHandle.GetApp();
    EXPECT_EQ(app.id, "123");
}

/*
 * 用例名称：获取备份任务应用信息失败
 * 前置条件：任务类型正确、应用信息为nullptr
 * check点：ID为空
 */
TEST_F(JobHandleTest, GetAppBACKUPFailed)
{
    JobType jobType = JobType::BACKUP;
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = nullptr;
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    AppProtect::Application app = jobHandle.GetApp();
    EXPECT_EQ(app.id, "");
}

/*
 * 用例名称：获取恢复任务应用信息成功
 * 前置条件：任务类型正确、应用信息正确
 * check点：ID符合预期
 */
TEST_F(JobHandleTest, GetAppRESTORESuccess)
{
    JobType jobType = JobType::RESTORE;
    m_restoreJobInfo.targetObject.id = "123";
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreJobInfo);
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    AppProtect::Application app = jobHandle.GetApp();
    EXPECT_EQ(app.id, "123");
}

/*
 * 用例名称：获取备份任务应用信息失败
 * 前置条件：任务类型正确、应用信息为nullptr
 * check点：ID为空
 */
TEST_F(JobHandleTest, GetAppRESTOREFailed)
{
    JobType jobType = JobType::RESTORE;
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = nullptr;
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    AppProtect::Application app = jobHandle.GetApp();
    EXPECT_EQ(app.id, "");
}

/* ----------- Test GetVolumes ----------- */
/*
 * 用例名称：获取任务保护子对象信息失败
 * 前置条件：任务类型错误
 * check点：任务类型为UNDEFINED_JOB_TYPE
 */
TEST_F(JobHandleTest, GetVolumesUndefinedType)
{
    JobType jobType = JobType::UNDEFINED_JOB_TYPE;
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = nullptr;
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    std::vector<AppProtect::ApplicationResource> volList = jobHandle.GetVolumes();
    EXPECT_EQ(jobHandle.GetJobType(), JobType::UNDEFINED_JOB_TYPE);
}

/*
 * 用例名称：获取任务保护子对象信息成功
 * 前置条件：任务类型正确、任务参数正确
 * check点：子对象资源id正确
 */
TEST_F(JobHandleTest, GetVolumesBACKUPSuccess)
{
    JobType jobType = JobType::BACKUP;
    ApplicationResource res;
    res.id = "resourceId";
    m_backupJobInfo.protectSubObject.push_back(res);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupJobInfo);
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    std::vector<AppProtect::ApplicationResource> volList = jobHandle.GetVolumes();
    EXPECT_EQ(volList.size(), 1);
    EXPECT_EQ(volList[0].id, res.id);
}

/*
 * 用例名称：获取任务保护子对象信息失败
 * 前置条件：任务类型正确、任务参数为pullptr
 * check点：子对象size为0
 */
TEST_F(JobHandleTest, GetVolumesBACKUPFailed)
{
    JobType jobType = JobType::BACKUP;
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = nullptr;
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    std::vector<AppProtect::ApplicationResource> volList = jobHandle.GetVolumes();
    EXPECT_EQ(volList.size(), 0);
}

/*
 * 用例名称：获取任务子对象信息成功
 * 前置条件：任务类型正确、任务参数正确
 * check点：子对象资源id正确
 */
TEST_F(JobHandleTest, GetVolumesRESTORESuccess)
{
    JobType jobType = JobType::RESTORE;
    ApplicationResource res;
    res.id = "resourceId";
    m_restoreJobInfo.restoreSubObjects.push_back(res);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreJobInfo);
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    std::vector<AppProtect::ApplicationResource> volList = jobHandle.GetVolumes();
    EXPECT_EQ(volList.size(), 1);
    EXPECT_EQ(volList[0].id, res.id);
}

/*
 * 用例名称：获取任务子对象信息失败
 * 前置条件：任务类型正确、任务参数为pullptr
 * check点：子对象size为0
 */
TEST_F(JobHandleTest, GetVolumesRESTOREFailed)
{
    JobType jobType = JobType::RESTORE;
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = nullptr;
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    std::vector<AppProtect::ApplicationResource> volList = jobHandle.GetVolumes();
    EXPECT_EQ(volList.size(), 0);
}

/*
 * 用例名称：获取恢复任务子对象ID成功
 * 前置条件：任务类型正确、任务参数为pullptr
 * check点：子对象size为0
 */
TEST_F(JobHandleTest, GetTaskId_restore_SUCCESS)
{
    JobType jobType = JobType::RESTORE;
    ApplicationResource res;
    res.id = "resourceId";
    m_restoreJobInfo.restoreSubObjects.push_back(res);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreJobInfo);
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    EXPECT_EQ(jobHandle.GetTaskId(), "");
}

/*
 * 用例名称：获取备份任务子对象ID成功
 * 前置条件：
 * check点：
 */
TEST_F(JobHandleTest, GetTaskId_backup_SUCCESS)
{
    JobType jobType = JobType::BACKUP;
    m_backupJobInfo.protectObject.id = "123";
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupJobInfo);
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    EXPECT_EQ(jobHandle.GetTaskId(), "");
}


/*
 * 用例名称：获取任务对象ID失败
 * 前置条件：任务类型正确、任务参数为pullptr
 * check点：子对象size为0
 */
TEST_F(JobHandleTest, GetTaskId_undefined)
{
    JobType jobType = JobType::UNDEFINED_JOB_TYPE;
    m_backupJobInfo.protectObject.id = "123";
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupJobInfo);
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    EXPECT_EQ(jobHandle.GetTaskId(), "");
}

/*
 * 用例名称：获取任务子对象信息失败
 * 前置条件：任务类型正确、任务参数为pullptr
 * check点：子对象size为0
 */
TEST_F(JobHandleTest, GetStorageRepos_backup)
{
    JobType jobType = JobType::BACKUP;
    m_backupJobInfo.protectObject.id = "123";
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupJobInfo);
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    std::vector<AppProtect::StorageRepository> eq;
    EXPECT_EQ(jobHandle.GetStorageRepos(), eq);
}

/*
 * 用例名称：获取任务子对象信息失败
 * 前置条件：任务类型正确、任务参数为pullptr
 * check点：子对象size为0
 */
TEST_F(JobHandleTest, GetStorageRepos_backup_Failed)
{
    JobType jobType = JobType::BACKUP;
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    JobHandle jobHandle(jobType, jobInfo);
    std::vector<AppProtect::StorageRepository> eq;
    EXPECT_EQ(jobHandle.GetStorageRepos(), eq);
}

/*
 * 用例名称：获取任务子对象信息失败
 * 前置条件：任务类型正确、任务参数为pullptr
 * check点：子对象size为0
 */
TEST_F(JobHandleTest, GetBackupType_restore_SUCCESS)
{
    JobType jobType = JobType::RESTORE;
    ApplicationResource res;
    res.id = "resourceId";
    m_restoreJobInfo.restoreSubObjects.push_back(res);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreJobInfo);
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    EXPECT_EQ(jobHandle.GetBackupType(), AppProtect::BackupJobType::FULL_BACKUP);
}

/*
 * 用例名称：获取任务子对象信息失败
 * 前置条件：任务类型正确、任务参数为pullptr
 * check点：子对象size为0
 */
TEST_F(JobHandleTest, GetBackupType_backup_Failed)
{
    JobType jobType = JobType::BACKUP;
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    JobHandle jobHandle(jobType, jobInfo);
    EXPECT_EQ(jobHandle.GetBackupType(), AppProtect::BackupJobType::FULL_BACKUP);
}

/*
 * 用例名称：获取任务子对象信息失败
 * 前置条件：任务类型正确、任务参数为pullptr
 * check点：子对象size为0
 */
TEST_F(JobHandleTest, GetBackupType_backup_SUCCESS)
{
    JobType jobType = JobType::BACKUP;
    m_backupJobInfo.protectObject.id = "123";
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupJobInfo);
    jobInfo->SetJobInfo(data);
    JobHandle jobHandle(jobType, jobInfo);
    EXPECT_EQ(jobHandle.GetBackupType(), 0);
}
}