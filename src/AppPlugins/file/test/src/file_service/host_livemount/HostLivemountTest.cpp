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
#include "host_livemount/HostLivemount.h"
 
using namespace std;
using namespace AppProtect;
using namespace FilePlugin;
 
class HostLivemountTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    HostLivemount m_hostLivemountObj;
    void SetJobInfo();
    void SetJobInfoTransJsonFail();
    Stub stub;
};
 
void HostLivemountTest::TearDown()
{}
 
void HostLivemountTest::SetUpTestCase()
{}
 
void HostLivemountTest::TearDownTestCase()
{}
 
static AppProtect::LivemountJob LivemountJobSetup()
{
    AppProtect::LivemountJob livemountJob;
    livemountJob.requestId = "reqId123";
    livemountJob.jobId = "jId123";
 
    string path = "/MetaFS/cache";
    HostAddress hostadd;
    hostadd.ip="10.28.12.144";
    StorageRepository cacheRepo;
    cacheRepo.repositoryType = RepositoryDataType::CACHE_REPOSITORY;
    cacheRepo.path.push_back(path);
    cacheRepo.remotePath = "/MetaFS/cache";
    cacheRepo.remoteHost.push_back(hostadd);
    cacheRepo.auth.authkey = "admin";
    cacheRepo.auth.authPwd = "Admin@123";
    cacheRepo.endpoint.ip = "10.28.12.144";
    cacheRepo.endpoint.port = 8088;
 
    StorageRepository dataRepo;
    dataRepo.repositoryType = RepositoryDataType::DATA_REPOSITORY;
    dataRepo.remotePath = "/BackupFS";
    dataRepo.path.push_back(dataRepo.remotePath);
    dataRepo.remoteHost.push_back(hostadd);
    dataRepo.auth.authkey = "admin";
    dataRepo.auth.authPwd = "Admin@123";
    dataRepo.endpoint.ip = "10.28.12.144";
    dataRepo.endpoint.port = 8088;
 
    StorageRepository metaRepo;
    metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
    path = "/MetaFS/meta";
    metaRepo.path.push_back(path);
    metaRepo.remotePath = "/MetaFS/meta";
    metaRepo.remoteHost.push_back(hostadd);
    metaRepo.auth.authkey = "admin";
    metaRepo.auth.authPwd = "Admin@123";
    metaRepo.endpoint.ip = "10.28.12.144";
    metaRepo.endpoint.port = 8088;
 
    livemountJob.copy.repositories.push_back(cacheRepo);
    livemountJob.copy.repositories.push_back(dataRepo);
    livemountJob.copy.repositories.push_back(metaRepo);
 
    livemountJob.extendInfo = "{\n\t\"dstPath\" : \"/opt\",\n\t\"fileSystemShareInfo\" : \n\t[\n\t\t{\n\t\t\t\"accessPermission\" : 1,\n\t\t\t\"advanceParams\" : \n\t\t\t{\n\t\t\t\t\"clientName\" : \"*\",\n\t\t\t\t\"clientType\" : 0,\n\t\t\t\t\"portSecure\" : 1,\n\t\t\t\t\"rootSquash\" : 1,\n\t\t\t\t\"squash\" : 1\n\t\t\t},\n\t\t\t\"fileSystemName\" : \"fileset_mount_202207251733\",\n\t\t\t\"type\" : 1\n\t\t}\n\t],\n\t\"performance\" : {}\n}\n";
    return livemountJob;
}

static AppProtect::LivemountJob LivemountJobSetupTransJsonFail()
{
    AppProtect::LivemountJob livemountJob;
    livemountJob.requestId = "reqId123";
    livemountJob.jobId = "jId123";
 
    string path = "/MetaFS/cache";
    HostAddress hostadd;
    hostadd.ip="10.28.12.144";
    StorageRepository cacheRepo;
    cacheRepo.repositoryType = RepositoryDataType::CACHE_REPOSITORY;
    cacheRepo.path.push_back(path);
    cacheRepo.remotePath = "/MetaFS/cache";
    cacheRepo.remoteHost.push_back(hostadd);
    cacheRepo.auth.authkey = "admin";
    cacheRepo.auth.authPwd = "Admin@123";
    cacheRepo.endpoint.ip = "10.28.12.144";
    cacheRepo.endpoint.port = 8088;
 
    StorageRepository dataRepo;
    dataRepo.repositoryType = RepositoryDataType::DATA_REPOSITORY;
    dataRepo.remotePath = "/BackupFS";
    dataRepo.path.push_back(dataRepo.remotePath);
    dataRepo.remoteHost.push_back(hostadd);
    dataRepo.auth.authkey = "admin";
    dataRepo.auth.authPwd = "Admin@123";
    dataRepo.endpoint.ip = "10.28.12.144";
    dataRepo.endpoint.port = 8088;
 
    StorageRepository metaRepo;
    metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
    path = "/MetaFS/meta";
    metaRepo.path.push_back(path);
    metaRepo.remotePath = "/MetaFS/meta";
    metaRepo.remoteHost.push_back(hostadd);
    metaRepo.auth.authkey = "admin";
    metaRepo.auth.authPwd = "Admin@123";
    metaRepo.endpoint.ip = "10.28.12.144";
    metaRepo.endpoint.port = 8088;
 
    livemountJob.copy.repositories.push_back(cacheRepo);
    livemountJob.copy.repositories.push_back(dataRepo);
    livemountJob.copy.repositories.push_back(metaRepo);

    return livemountJob;
}

static AppProtect::SubJob GetSubJob()
{
    AppProtect::SubJob subJob;
    subJob.jobId = "jobId123";
    subJob.subJobId = "subJob123";
    subJob.jobName = "subJob";
    subJob.jobPriority = 1;
    subJob.ignoreFailed = true;
    subJob.execNodeId = "abcde";
    subJob.jobInfo = "{\"ControlFile\":\"/cache/livemount-job/scan/ctrl\","
                    "\"SubTaskType\":1,"
                    "\"Ext\":\"abc\"}";
    return subJob;
}
 
static void ReportJobDetailsSuc(void *obj, ActionResult& returnValue, const SubJobDetails& jobInfo)
{
    return;
}

static int StubFunction_MountDriveInfo(void *obj, ActionResult& returnValue)
{
    returnValue.code = Module::SUCCESS;
    return Module::SUCCESS;
}

static void StubFunction_VOID(void *ob)
{}

static void Stub_AddReturnValue_SUCCESS(ActionResult& returnValue, void *obj)
{
    returnValue.code = Module::SUCCESS;
}

static void Stub_AddReturnValue_FAILED(ActionResult& returnValue, void *obj)
{
    returnValue.code = Module::FAILED;
}

static std::shared_ptr<LivemountJob> GetJobInfoBody_nullptr()
{
    return nullptr;
}

static AppProtect::LivemountJob GetJobInfo_Null_Rep()
{
    AppProtect::LivemountJob livemountJob = LivemountJobSetup();
    livemountJob.copy.repositories.clear();
    return livemountJob;
}

void HostLivemountTest::SetUp()
{
    stub.set(ADDR(JobService, ReportJobDetails), StubFunction_VOID);
}

void HostLivemountTest::SetJobInfo()
{
    AppProtect::LivemountJob livemountJob = LivemountJobSetup();
    auto jobInfoLivemount = make_shared<JobCommonInfo>(make_shared<LivemountJob>(livemountJob));
    m_hostLivemountObj.SetJobInfo(jobInfoLivemount);
}

void HostLivemountTest::SetJobInfoTransJsonFail()
{
    AppProtect::LivemountJob livemountJob = LivemountJobSetupTransJsonFail();
    auto jobInfoLivemount = make_shared<JobCommonInfo>(make_shared<LivemountJob>(livemountJob));
    m_hostLivemountObj.SetJobInfo(jobInfoLivemount);
}

TEST_F(HostLivemountTest, testPrerequisiteJobInner)
{
    SetJobInfo();
    EXPECT_EQ(m_hostLivemountObj.PrerequisiteJob(), Module::SUCCESS);
}

TEST_F(HostLivemountTest, testPrerequisiteJob_Null_Info)
{
    stub.set(ADDR(HostLivemount, GetJobInfoBody), GetJobInfoBody_nullptr);
    EXPECT_EQ(m_hostLivemountObj.PrerequisiteJob(), Module::FAILED);
}

TEST_F(HostLivemountTest, GenerateSubJob)
{
    SetJobInfo();
    stub.set(ADDR(JobService, AddNewJob), Stub_AddReturnValue_SUCCESS);

    EXPECT_EQ(m_hostLivemountObj.GenerateSubJob(), Module::SUCCESS);
}

TEST_F(HostLivemountTest, GenerateSubJob_Null_Info)
{
    stub.set(ADDR(HostLivemount, GetJobInfoBody), GetJobInfoBody_nullptr);
    EXPECT_EQ(m_hostLivemountObj.GenerateSubJob(), Module::FAILED);
}

TEST_F(HostLivemountTest, ExecuteSubJob)
{
    SetJobInfo();
    stub.set(ADDR(JobService, MountRepositoryByPlugin), Stub_AddReturnValue_SUCCESS);
    stub.set(ADDR(JobService, UnMountRepositoryByPlugin), Stub_AddReturnValue_SUCCESS);
    AppProtect::SubJob subJob = GetSubJob();
    stub.set(ADDR(HostLivemount, GetFileSetMountDriveInfo), StubFunction_MountDriveInfo);
    m_hostLivemountObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostLivemountObj.ExecuteSubJob(), Module::SUCCESS);
}

TEST_F(HostLivemountTest, ExecuteSubJobJsonTranFail)
{
    SetJobInfoTransJsonFail();
    stub.set(ADDR(JobService, MountRepositoryByPlugin), Stub_AddReturnValue_SUCCESS);
    stub.set(ADDR(JobService, UnMountRepositoryByPlugin), Stub_AddReturnValue_SUCCESS);
    AppProtect::SubJob subJob = GetSubJob();
    m_hostLivemountObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostLivemountObj.ExecuteSubJob(), Module::FAILED);
}

TEST_F(HostLivemountTest, ExecuteSubJob_Null_Info)
{
    stub.set(ADDR(HostLivemount, GetJobInfoBody), GetJobInfoBody_nullptr);
    EXPECT_EQ(m_hostLivemountObj.ExecuteSubJob(), Module::FAILED);
}

TEST_F(HostLivemountTest, ExecuteSubJob_null_Rep)
{
    AppProtect::LivemountJob livemountJob = GetJobInfo_Null_Rep();
    auto jobInfoLivemount = make_shared<JobCommonInfo>(make_shared<LivemountJob>(livemountJob));
    m_hostLivemountObj.SetJobInfo(jobInfoLivemount);
    stub.set(ADDR(JobService, MountRepositoryByPlugin), Stub_AddReturnValue_SUCCESS);
    stub.set(ADDR(JobService, UnMountRepositoryByPlugin), Stub_AddReturnValue_SUCCESS);
    AppProtect::SubJob subJob = GetSubJob();
    m_hostLivemountObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostLivemountObj.ExecuteSubJob(), Module::FAILED);
}

TEST_F(HostLivemountTest, ExecuteSubJob_Mount_Failed)
{
    SetJobInfo();
    stub.set(ADDR(JobService, MountRepositoryByPlugin), Stub_AddReturnValue_FAILED);
    stub.set(ADDR(JobService, UnMountRepositoryByPlugin), Stub_AddReturnValue_SUCCESS);
    AppProtect::SubJob subJob = GetSubJob();
    m_hostLivemountObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostLivemountObj.ExecuteSubJob(), Module::FAILED);
}

TEST_F(HostLivemountTest, PostJob)
{
    SetJobInfo();
    EXPECT_EQ(m_hostLivemountObj.PostJob(), Module::SUCCESS);
}

TEST_F(HostLivemountTest, PostJob_Null_Info)
{
    stub.set(ADDR(HostLivemount, GetJobInfoBody), GetJobInfoBody_nullptr);
    EXPECT_EQ(m_hostLivemountObj.PostJob(), Module::FAILED);
}
