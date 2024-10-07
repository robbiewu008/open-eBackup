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
#include "CommonJobFactory.h"
#include "HostCommonService.h"
#include "HostBackup.h"
#include "HostRestore.h"
#include "HostLivemount.h"
#include "HostCancelLivemount.h"
#include "HostArchiveRestore.h"

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
using namespace std;

namespace {
    const uint64_t COMMANDJOBTYPE_TEST = 999;
}

class CommonJobFactoryTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};


void CommonJobFactoryTest::SetUp()
{
}

void CommonJobFactoryTest::TearDown()
{
}

void CommonJobFactoryTest::SetUpTestCase()
{
}

void CommonJobFactoryTest::TearDownTestCase()
{
}

struct FileSetInfo {
    std::string filters;
    std::string paths;
    std::string templateId;
    std::string templateName;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(filters, filters)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(paths, paths)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(templateId, templateId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(templateName, templateName)
    END_SERIAL_MEMEBER
};

AppProtect::BuildIndexJob BuildIndexJobSetUp()
{
    AppProtect::BuildIndexJob buildIndexJob;

    buildIndexJob.requestId = "123456789";
    buildIndexJob.jobId = "111111";
    buildIndexJob.indexProtectObject.id = "123";
    buildIndexJob.indexProtectObject.name = "123";
    buildIndexJob.indexProtectObject.subType = "Fileset";

    FileSetInfo fileSetInfo;
    fileSetInfo.filters = "";
    fileSetInfo.paths = "[{\"name\":\"/l30015744_restore\"}]";
    fileSetInfo.templateId = "";
    fileSetInfo.templateName = "";

    string filesetInfoStr;
    Module::JsonHelper::StructToJsonString(fileSetInfo, filesetInfoStr);

    string cachepath = "/StorageRepository/cache";
    string datapath = "/StorageRepository/data";
    string metapath = "/StorageRepository/meta";
    StorageRepository cacheRepo;
    cacheRepo.repositoryType = RepositoryDataType::CACHE_REPOSITORY;
    cacheRepo.path.push_back(cachepath);

    StorageRepository dataRepo;
    dataRepo.repositoryType = RepositoryDataType::DATA_REPOSITORY;
    dataRepo.path.push_back(datapath);

    StorageRepository metaRepo;
    metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
    metaRepo.path.push_back(metapath);

    Copy copy;
    copy.repositories.push_back(cacheRepo);

    copy.repositories.push_back(dataRepo);
    copy.repositories.push_back(metaRepo);
    copy.formatType = CopyFormatType::type::INNER_DIRECTORY;

    AggCopyExtendInfo aggCopyExtendInfo;
    aggCopyExtendInfo.dataPathSuffix = "test6666666";
    aggCopyExtendInfo.dataPathSuffix = "test8888888";
    aggCopyExtendInfo.maxSizeToAggregate = "222222222";
    aggCopyExtendInfo.maxSizeAfterAggregate = "22222222";
    string aggCopyExtendInfoStr;
    Module::JsonHelper::StructToJsonString(aggCopyExtendInfo, aggCopyExtendInfoStr);
    copy.extendInfo = aggCopyExtendInfoStr;

    buildIndexJob.copies.push_back(copy);
    buildIndexJob.extendInfo = "{\"failed_script\":\"\",\"post_script\":\"\",\"pre_script\":\"\","
                           "\"restoreOption\":\"SKIP\"}";
    return buildIndexJob;
}

AppProtect::CancelLivemountJob CancelLivemountJobSetUp()
{
    AppProtect::CancelLivemountJob cancelLivemountJob;

    cancelLivemountJob.requestId = "123456789";
    cancelLivemountJob.jobId = "111111";
    cancelLivemountJob.targetEnv.subType = "1";
    cancelLivemountJob.targetObject.id = "123";
    cancelLivemountJob.targetObject.name = "123";
    cancelLivemountJob.targetObject.subType = "Fileset";

    FileSetInfo fileSetInfo;
    fileSetInfo.filters = "";
    fileSetInfo.paths = "[{\"name\":\"/l30015744_restore\"}]";
    fileSetInfo.templateId = "";
    fileSetInfo.templateName = "";

    string filesetInfoStr;
    Module::JsonHelper::StructToJsonString(fileSetInfo, filesetInfoStr);
    cancelLivemountJob.targetEnv.extendInfo = filesetInfoStr;

    string cachepath = "/StorageRepository/cache";
    string datapath = "/StorageRepository/data";
    string metapath = "/StorageRepository/meta";
    StorageRepository cacheRepo;
    cacheRepo.repositoryType = RepositoryDataType::CACHE_REPOSITORY;
    cacheRepo.path.push_back(cachepath);

    StorageRepository dataRepo;
    dataRepo.repositoryType = RepositoryDataType::DATA_REPOSITORY;
    dataRepo.path.push_back(datapath);

    StorageRepository metaRepo;
    metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
    metaRepo.path.push_back(metapath);

    Copy copy;
    copy.repositories.push_back(cacheRepo);

    copy.repositories.push_back(dataRepo);
    copy.repositories.push_back(metaRepo);
    copy.formatType = CopyFormatType::type::INNER_DIRECTORY;

    AggCopyExtendInfo aggCopyExtendInfo;
    aggCopyExtendInfo.dataPathSuffix = "test6666666";
    aggCopyExtendInfo.dataPathSuffix = "test8888888";
    aggCopyExtendInfo.maxSizeToAggregate = "222222222";
    aggCopyExtendInfo.maxSizeAfterAggregate = "22222222";
    string aggCopyExtendInfoStr;
    Module::JsonHelper::StructToJsonString(aggCopyExtendInfo, aggCopyExtendInfoStr);
    copy.extendInfo = aggCopyExtendInfoStr;

    cancelLivemountJob.copy = copy;
    cancelLivemountJob.extendInfo = "{\"failed_script\":\"\",\"post_script\":\"\",\"pre_script\":\"\","
                           "\"restoreOption\":\"SKIP\"}";
    return cancelLivemountJob;
}

AppProtect::LivemountJob LivemountJobSetUp()
{
    AppProtect::LivemountJob livemountJob;
    // livemountJob.jobId = "mm";
    // return livemountJob;
    // AppProtect::RestoreJob restoreJob;
    livemountJob.requestId = "123456789";
    livemountJob.jobId = "111111";
    livemountJob.targetEnv.subType = "1";
    livemountJob.targetObject.id = "123";
    livemountJob.targetObject.name = "123";
    livemountJob.targetObject.subType = "Fileset";

    FileSetInfo fileSetInfo;
    fileSetInfo.filters = "";
    fileSetInfo.paths = "[{\"name\":\"/l30015744_restore\"}]";
    fileSetInfo.templateId = "";
    fileSetInfo.templateName = "";

    string filesetInfoStr;
    Module::JsonHelper::StructToJsonString(fileSetInfo, filesetInfoStr);
    livemountJob.targetEnv.extendInfo = filesetInfoStr;

    string cachepath = "/StorageRepository/cache";
    string datapath = "/StorageRepository/data";
    string metapath = "/StorageRepository/meta";
    StorageRepository cacheRepo;
    cacheRepo.repositoryType = RepositoryDataType::CACHE_REPOSITORY;
    cacheRepo.path.push_back(cachepath);

    StorageRepository dataRepo;
    dataRepo.repositoryType = RepositoryDataType::DATA_REPOSITORY;
    dataRepo.path.push_back(datapath);

    StorageRepository metaRepo;
    metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
    metaRepo.path.push_back(metapath);

    Copy copy;
    copy.repositories.push_back(cacheRepo);

    copy.repositories.push_back(dataRepo);
    copy.repositories.push_back(metaRepo);
    copy.formatType = CopyFormatType::type::INNER_DIRECTORY;

    AggCopyExtendInfo aggCopyExtendInfo;
    aggCopyExtendInfo.dataPathSuffix = "test6666666";
    aggCopyExtendInfo.dataPathSuffix = "test8888888";
    aggCopyExtendInfo.maxSizeToAggregate = "222222222";
    aggCopyExtendInfo.maxSizeAfterAggregate = "22222222";
    string aggCopyExtendInfoStr;
    Module::JsonHelper::StructToJsonString(aggCopyExtendInfo, aggCopyExtendInfoStr);
    copy.extendInfo = aggCopyExtendInfoStr;

    livemountJob.copy = copy;
    livemountJob.extendInfo = "{\"failed_script\":\"\",\"post_script\":\"\",\"pre_script\":\"\","
                           "\"restoreOption\":\"SKIP\"}";
    return livemountJob;
}

AppProtect::RestoreJob RestoreJobInfoForAggregateSetUp()
{
    AppProtect::RestoreJob restoreJob;
    restoreJob.requestId = "123456789";
    restoreJob.jobId = "111111";
    restoreJob.targetEnv.subType = "1";
    restoreJob.targetObject.id = "123";
    restoreJob.targetObject.name = "123";
    restoreJob.targetObject.subType = "5";

    FileSetInfo fileSetInfo;
    fileSetInfo.filters = "";
    fileSetInfo.paths = "[{\"name\":\"/l30015744_restore\"}]";
    fileSetInfo.templateId = "";
    fileSetInfo.templateName = "";

    string filesetInfoStr;
    Module::JsonHelper::StructToJsonString(fileSetInfo, filesetInfoStr);
    restoreJob.targetEnv.extendInfo = filesetInfoStr;

    string cachepath = "/StorageRepository/cache";
    string datapath = "/StorageRepository/data";
    string metapath = "/StorageRepository/meta";
    StorageRepository cacheRepo;
    cacheRepo.repositoryType = RepositoryDataType::CACHE_REPOSITORY;
    cacheRepo.path.push_back(cachepath);

    StorageRepository dataRepo;
    dataRepo.repositoryType = RepositoryDataType::DATA_REPOSITORY;
    dataRepo.path.push_back(datapath);

    StorageRepository metaRepo;
    metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
    metaRepo.path.push_back(metapath);

    Copy copy;
    copy.repositories.push_back(cacheRepo);

    copy.repositories.push_back(dataRepo);
    copy.repositories.push_back(metaRepo);
    copy.formatType = CopyFormatType::type::INNER_DIRECTORY;

    AggCopyExtendInfo aggCopyExtendInfo;
    aggCopyExtendInfo.dataPathSuffix = "test6666666";
    aggCopyExtendInfo.dataPathSuffix = "test8888888";
    aggCopyExtendInfo.maxSizeToAggregate = "222222222";
    aggCopyExtendInfo.maxSizeAfterAggregate = "22222222";
    string aggCopyExtendInfoStr;
    Module::JsonHelper::StructToJsonString(aggCopyExtendInfo, aggCopyExtendInfoStr);
    copy.extendInfo = aggCopyExtendInfoStr;

    restoreJob.copies.push_back(copy);
    restoreJob.copies.push_back(copy);
    restoreJob.copies.push_back(copy);
    restoreJob.extendInfo = "{\"failed_script\":\"\",\"post_script\":\"\",\"pre_script\":\"\","
                           "\"restoreOption\":\"SKIP\"}";
    return restoreJob;
}

/*
* 用例名称：通过任务工厂创建任务
* 前置条件：无
* check点：成功创建对应任务
*/
TEST_F(CommonJobFactoryTest, FactoryCreateBackupJob_Success)
{
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    JobType jobtype;
    EXPECT_EQ(CommonJobFactory::GetInstance()->CreateJob(jobInfo, jobtype), nullptr);
}

/*
* 用例名称：通过任务工厂创建任务
* 前置条件：无
* check点：创建对应任务失败，无对应任务类型
*/
TEST_F(CommonJobFactoryTest, FactoryCreateBackupJob_Failed)
{
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    JobType jobtype;
    EXPECT_EQ(CommonJobFactory::GetInstance()->CreateJob(jobInfo, jobtype), nullptr);
}

/*
* 用例名称：GetAppType
* 前置条件：无
* check点：获取备份的App类型
*/
TEST_F(CommonJobFactoryTest, GetAppType)
{
    std::shared_ptr<JobCommonInfo> jobCommonInfo = std::make_shared<JobCommonInfo>();
    CommonJobFactory commonJobFactory;

    EXPECT_EQ(commonJobFactory.GetAppType(jobCommonInfo, JobType::BACKUP), 0);
    EXPECT_EQ(commonJobFactory.GetAppType(jobCommonInfo, JobType::INDEX), 0);
    EXPECT_EQ(commonJobFactory.GetAppType(jobCommonInfo, JobType::RESTORE), 0);
    EXPECT_EQ(commonJobFactory.GetAppType(jobCommonInfo, JobType::LIVEMOUNT), 0);
    EXPECT_EQ(commonJobFactory.GetAppType(jobCommonInfo, JobType::CANCELLIVEMOUNT), 0);
    EXPECT_EQ(commonJobFactory.GetAppType(jobCommonInfo, JobType::UNDEFINED_JOB_TYPE), 0);
}

/*
* 用例名称：GetBackupAppType
* 前置条件：无
* check点：获取备份的App类型
*/
TEST_F(CommonJobFactoryTest, GetBackupAppType)
{

    std::shared_ptr<JobCommonInfo> jobCommonInfo = std::make_shared<JobCommonInfo>();

    CommonJobFactory commonJobFactory;
    EXPECT_EQ(commonJobFactory.GetBackupAppType(jobCommonInfo), 0);
}

std::shared_ptr<ThriftDataBase> GetJobInf_NO_Null()
{

    return std::dynamic_pointer_cast<ThriftDataBase>(std::make_shared<AppProtect::BackupJob>());;
}

TEST_F(CommonJobFactoryTest, GetBackupAppType_NoJob)
{
    Stub stub;
    stub.set(ADDR(JobCommonInfo, GetJobInfo), GetJobInf_NO_Null);
    std::shared_ptr<JobCommonInfo> jobCommonInfo = std::make_shared<JobCommonInfo>();

    CommonJobFactory commonJobFactory;
    EXPECT_EQ(commonJobFactory.GetBackupAppType(jobCommonInfo), 0);

    stub.reset(ADDR(JobCommonInfo, GetJobInfo));
}

/*
* 用例名称：GetRestoreAppType
* 前置条件：无
* check点：获取存储的App类型
*/
TEST_F(CommonJobFactoryTest, GetRestoreAppType)
{
    CommonJobFactory commonJobFactory;

    std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
    std::make_shared<AppProtect::RestoreJob>(RestoreJobInfoForAggregateSetUp());

    std::shared_ptr<JobCommonInfo> jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);

    EXPECT_EQ(commonJobFactory.GetRestoreAppType(jobCommonInfo), 0);
}

TEST_F(CommonJobFactoryTest, GetRestoreAppType_NoJob)
{
    Stub stub;
    stub.set(ADDR(JobCommonInfo, GetJobInfo), GetJobInf_NO_Null);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = std::make_shared<JobCommonInfo>();
    CommonJobFactory commonJobFactory;

    EXPECT_EQ(commonJobFactory.GetRestoreAppType(jobCommonInfo), 0);

    stub.reset(ADDR(JobCommonInfo, GetJobInfo));
}


/*
* 用例名称：CreateFactoryJob
* 前置条件：无
* check点：创建工厂任务
*/
TEST_F(CommonJobFactoryTest, CreateFactoryJob)
{

    std::shared_ptr<JobCommonInfo> jobCommonInfo = std::make_shared<JobCommonInfo>();

    CommonJobFactory commonJobFactory;

    std::shared_ptr<AppProtect::BackupJob> backupJobInfo = std::make_shared<AppProtect::BackupJob>();

    std::vector<ApplicationEnvironment> AgentList;
    ApplicationEnvironment test1;
    test1.id = "111";
    ApplicationEnvironment test2;
    test2.id = "222";
    AgentList.push_back(test1);
    AgentList.push_back(test2);

    backupJobInfo->protectEnv.nodes = AgentList;
    backupJobInfo->extendInfo = "{\"slave_node_first\":\"1\"}";
    backupJobInfo->protectObject.subType = "Dameng";
    backupJobInfo->protectEnv.extendInfo = "{\"deployType\":\"1\"}";
    std::shared_ptr<ThriftDataBase> data = std::dynamic_pointer_cast<ThriftDataBase>(backupJobInfo);
    jobCommonInfo->SetJobInfo(data);

    EXPECT_NE(commonJobFactory.CreateFactoryJob<HostBackup>(jobCommonInfo), nullptr);
}


TEST_F(CommonJobFactoryTest, GetLivemountAppType)
{

    CommonJobFactory commonJobFactory;

    std::shared_ptr<ThriftDataBase> livemountJobInfoPtr =
    std::make_shared<AppProtect::LivemountJob>(LivemountJobSetUp());

    std::shared_ptr<JobCommonInfo> jobCommonInfo = std::make_shared<JobCommonInfo>(livemountJobInfoPtr);

    EXPECT_EQ(commonJobFactory.GetLivemountAppType(jobCommonInfo), 1);

    std::shared_ptr<AppProtect::BackupJob> backupJobInfo = std::make_shared<AppProtect::BackupJob>();

    std::vector<ApplicationEnvironment> AgentList;
    ApplicationEnvironment test1;
    test1.id = "111";
    ApplicationEnvironment test2;
    test2.id = "222";
    AgentList.push_back(test1);
    AgentList.push_back(test2);

    backupJobInfo->protectEnv.nodes = AgentList;
    backupJobInfo->extendInfo = "{\"slave_node_first\":\"1\"}";
    backupJobInfo->protectObject.subType = "Dameng";
    backupJobInfo->protectEnv.extendInfo = "{\"deployType\":\"1\"}";
    std::shared_ptr<ThriftDataBase> data = std::dynamic_pointer_cast<ThriftDataBase>(backupJobInfo);
    jobCommonInfo->SetJobInfo(data);
}

TEST_F(CommonJobFactoryTest, GetCancelLivemountAppType)
{
    CommonJobFactory commonJobFactory;

    std::shared_ptr<ThriftDataBase> cancelLivemountJobInfoPtr =
    std::make_shared<AppProtect::CancelLivemountJob>(CancelLivemountJobSetUp());

    std::shared_ptr<JobCommonInfo> jobCommonInfo = std::make_shared<JobCommonInfo>(cancelLivemountJobInfoPtr);

    EXPECT_EQ(commonJobFactory.GetCancelLivemountAppType(jobCommonInfo), 1);
}

TEST_F(CommonJobFactoryTest, GetIndexAppType)
{
    std::shared_ptr<ThriftDataBase> buildIndexJobInfoPtr =
    std::make_shared<AppProtect::BuildIndexJob>(BuildIndexJobSetUp());

    std::shared_ptr<JobCommonInfo> jobCommonInfo = std::make_shared<JobCommonInfo>(buildIndexJobInfoPtr);

    CommonJobFactory commonJobFactory;
    EXPECT_EQ(commonJobFactory.GetIndexAppType(jobCommonInfo), 1);
}

