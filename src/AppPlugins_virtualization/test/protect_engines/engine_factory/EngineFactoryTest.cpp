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
#include "stub.h"
#include "protect_engines/ProtectEngine.h"
#include "protect_engines/engine_factory/EngineFactory.h"
using namespace VirtPlugin;

namespace HDT_TEST {
class EngineFactoryTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void EngineFactoryTest::SetUp() {}
void EngineFactoryTest::TearDown() {}
void EngineFactoryTest::SetUpTestCase() {}
void EngineFactoryTest::TearDownTestCase() {}

/*
 * 测试用例： 创建Kubernetes保护对象
 * 前置条件： 应用名称错误
 * CHECK点： 创建Kubernetes保护对象失败
 */
TEST_F(EngineFactoryTest, CreateProtectEngine_Falied)
{
    std::shared_ptr<ProtectEngine> protectEnginePtr = nullptr;
    std::string engineName = "KubernetesH";
    protectEnginePtr = EngineFactory::CreateProtectEngineWithoutTask(engineName);
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, true);
}

/*
 * 测试用例： 创建Kubernetes保护对象
 * 前置条件： 应用名称正确
 * CHECK点： 创建Kubernetes保护对象成功
 */
TEST_F(EngineFactoryTest, CreateProtectEngine_Success)
{
    std::shared_ptr<ProtectEngine> protectEnginePtr = nullptr;
    std::string engineName = "Kubernetes";
    protectEnginePtr = EngineFactory::CreateProtectEngineWithoutTask(engineName);
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, false);
}

/*
 * 测试用例： 创建Kubernetes保护对象-备份任务
 * 前置条件： 应用名称正确
 * CHECK点： 创建Kubernetes保护对象成功
 */
TEST_F(EngineFactoryTest, Create_K8S_BackupProtectEngine_Success)
{
    JobType jobType = JobType::BACKUP;
    AppProtect::BackupJob backupJobParam;
    backupJobParam.protectEnv.subType = "Kubernetes";
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(backupJobParam);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<ProtectEngine> protectEnginePtr = EngineFactory::CreateProtectEngine(jobType, jobInfo, "1234", "");
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, false);
}

/*
 * 测试用例： 创建Kubernetes保护对象-恢复任务
 * 前置条件： 应用名称正确
 * CHECK点： 创建Kubernetes保护对象成功
 */
TEST_F(EngineFactoryTest, Create_K8S_RestoreProtectEngine_Success)
{
    JobType jobType = JobType::RESTORE;
    AppProtect::RestoreJob restoreJobParam;
    restoreJobParam.targetEnv.subType = "Kubernetes";
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(restoreJobParam);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<ProtectEngine> protectEnginePtr = EngineFactory::CreateProtectEngine(jobType, jobInfo, "123", "");
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, false);
}

/*
 * 测试用例： 创建Kubernetes保护对象-备份任务
 * 前置条件： 应用名称错误
 * CHECK点： 创建Kubernetes保护对象失败
 */
TEST_F(EngineFactoryTest, Create_K8S_BackupProtectEngine_Failed)
{
    JobType jobType = JobType::BACKUP;
    AppProtect::BackupJob backupJobParam;
    backupJobParam.protectEnv.subType = "KubernetesH";
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(backupJobParam);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<ProtectEngine> protectEnginePtr = EngineFactory::CreateProtectEngine(jobType, jobInfo, "123", "");
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, true);
}

/*
 * 测试用例： 创建Kubernetes保护对象-恢复任务
 * 前置条件： 应用名称错误
 * CHECK点： 创建Kubernetes保护对象失败
 */
TEST_F(EngineFactoryTest, Create_K8S_RestoreProtectEngine_Failed)
{
    JobType jobType = JobType::RESTORE;
    AppProtect::RestoreJob restoreJobParam;
    restoreJobParam.targetEnv.subType = "KubernetesH";
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(restoreJobParam);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<ProtectEngine> protectEnginePtr = EngineFactory::CreateProtectEngine(jobType, jobInfo, "123", "");
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, true);
}

/*
 * 测试用例： 创建 HCS 保护对象-备份任务
 * 前置条件： 应用名称正确
 * CHECK点： 创建 HCS 保护对象成功
 */
TEST_F(EngineFactoryTest, Create_HCS_BackupProtectEngine_Success)
{
    JobType jobType = JobType::BACKUP;
    AppProtect::BackupJob backupJobParam;
    backupJobParam.protectEnv.subType = "HCSContainer";
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(backupJobParam);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<ProtectEngine> protectEnginePtr = EngineFactory::CreateProtectEngine(jobType, jobInfo, "123", "");
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, false);
}

/*
 * 测试用例： 创建HCS保护对象-恢复任务
 * 前置条件： 应用名称正确
 * CHECK点： 创建HCS保护对象成功
 */
TEST_F(EngineFactoryTest, Create_HCS_RestoreProtectEngine_Success)
{
    JobType jobType = JobType::RESTORE;
    AppProtect::RestoreJob restoreJobParam;
    restoreJobParam.targetEnv.subType = "HCSContainer";
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(restoreJobParam);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<ProtectEngine> protectEnginePtr = EngineFactory::CreateProtectEngine(jobType, jobInfo, "123", "");
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, false);
}

/*
 * 测试用例： 创建HCS保护对象-备份任务
 * 前置条件： 应用名称错误
 * CHECK点： 创建HCS保护对象失败
 */
TEST_F(EngineFactoryTest, Create_HCS_BackupProtectEngine_Failed)
{
    JobType jobType = JobType::BACKUP;
    AppProtect::BackupJob backupJobParam;
    backupJobParam.protectEnv.subType = "HCSErrSubType";
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(backupJobParam);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<ProtectEngine> protectEnginePtr = EngineFactory::CreateProtectEngine(jobType, jobInfo, "123", "");
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, true);
}

/*
 * 测试用例： 创建HCS保护对象-恢复任务
 * 前置条件： 应用名称错误
 * CHECK点： 创建HCS保护对象失败
 */
TEST_F(EngineFactoryTest, Create_HCS_RestoreProtectEngine_Failed)
{
    JobType jobType = JobType::RESTORE;
    AppProtect::RestoreJob restoreJobParam;
    restoreJobParam.targetEnv.subType = "HCSErrSubType";
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(restoreJobParam);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<ProtectEngine> protectEnginePtr = EngineFactory::CreateProtectEngine(jobType, jobInfo, "123", "");
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, true);
}

/*
 * 测试用例： 创建OpenStack保护对象
 * 前置条件： 应用名称正确
 * CHECK点： 创建OpenStack保护对象成功
 */
TEST_F(EngineFactoryTest, CreateOpenStackProtectEngine_Success)
{
    std::shared_ptr<ProtectEngine> protectEnginePtr = nullptr;
    std::string engineName = "OpenStackContainer";
    protectEnginePtr = EngineFactory::CreateProtectEngineWithoutTask(engineName);
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, false);
}
 
/*
 * 测试用例： 创建OpenStack保护对象失败
 * 前置条件： 应用名称错误
 * CHECK点： 创建OpenStack保护对象失败
 */
TEST_F(EngineFactoryTest, CreateOpenStackProtectEngine_Falied)
{
    std::shared_ptr<ProtectEngine> protectEnginePtr = nullptr;
    std::string engineName = "OpenStackErrType";
    protectEnginePtr = EngineFactory::CreateProtectEngineWithoutTask(engineName);
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, true);
}
 
/*
 * 测试用例： 创建OpenStack保护对象-备份任务
 * 前置条件： 应用名称错误
 * CHECK点： 创建OpenStack保护对象成功
 */
TEST_F(EngineFactoryTest, Create_OpenStack_BackupProtectEngine_Success)
{
    JobType jobType = JobType::BACKUP;
    AppProtect::BackupJob backupJobParam;
    backupJobParam.protectEnv.subType = "OpenStackContainer";
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(backupJobParam);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<ProtectEngine> protectEnginePtr = EngineFactory::CreateProtectEngine(jobType, jobInfo, "123", "");
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, false);
}
 
/*
 * 测试用例： 创建OpenStack保护对象-备份任务
 * 前置条件： 应用名称错误
 * CHECK点： 创建OpenStack保护对象失败
 */
TEST_F(EngineFactoryTest, Create_OpenStack_BackupProtectEngine_Failed)
{
    JobType jobType = JobType::BACKUP;
    AppProtect::BackupJob backupJobParam;
    backupJobParam.protectEnv.subType = "OpenStackErrType";
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(backupJobParam);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<ProtectEngine> protectEnginePtr = EngineFactory::CreateProtectEngine(jobType, jobInfo, "123", "");
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, true);
}
 
/*
 * 测试用例： 创建OpenStack保护对象-恢复任务
 * 前置条件： 应用名称正确
 * CHECK点： 创建OpenStack保护对象成功
 */
TEST_F(EngineFactoryTest, Create_OpenStack_RestoreProtectEngine_Success)
{
    JobType jobType = JobType::RESTORE;
    AppProtect::RestoreJob restoreJobParam;
    restoreJobParam.targetEnv.subType = "OpenStackContainer";
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(restoreJobParam);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<ProtectEngine> protectEnginePtr = EngineFactory::CreateProtectEngine(jobType, jobInfo, "123", "");
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, false);
}
 
/*
 * 测试用例： 创建OpenStack保护对象-恢复任务
 * 前置条件： 应用名称错误
 * CHECK点： 创建OpenStack保护对象失败
 */
TEST_F(EngineFactoryTest, Create_OpenStack_RestoreProtectEngine_Failed)
{
    JobType jobType = JobType::RESTORE;
    AppProtect::RestoreJob restoreJobParam;
    restoreJobParam.targetEnv.subType = "OpenStackErrType";
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(restoreJobParam);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<ProtectEngine> protectEnginePtr = EngineFactory::CreateProtectEngine(jobType, jobInfo, "123", "");
    bool retValue = false;
    retValue = (protectEnginePtr.get() == nullptr);
    EXPECT_EQ(retValue, true);
}
}