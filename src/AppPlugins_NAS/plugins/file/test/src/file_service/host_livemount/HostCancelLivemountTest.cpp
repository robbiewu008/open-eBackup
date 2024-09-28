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
#include "host_livemount/HostCancelLivemount.h"
 
using namespace std;
using namespace AppProtect;
using namespace FilePlugin;
 
class HostCancelLivemountTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    HostCancelLivemount m_hostCancelLeavemountObj;
    void SetJobInfo();
    void SetJsonTransFailJobInfo();
    Stub stub;
};
 
void HostCancelLivemountTest::TearDown()
{}
 
void HostCancelLivemountTest::SetUpTestCase()
{}
 
void HostCancelLivemountTest::TearDownTestCase()
{}
 
static AppProtect::CancelLivemountJob CancelLivemountJobSetup()
{
    AppProtect::CancelLivemountJob cancelLivemountJob;
    cancelLivemountJob.requestId = "reqId123";
    cancelLivemountJob.jobId = "jId123";
 
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
 
    cancelLivemountJob.copy.repositories.push_back(cacheRepo);
    cancelLivemountJob.copy.repositories.push_back(dataRepo);
    cancelLivemountJob.copy.repositories.push_back(metaRepo);
 
    cancelLivemountJob.extendInfo = "{\n\t\"dstPath\" : \"/tmp\",\n\t\"fileSystemShareInfo\" : \n\t[\n\t\t{\n\t\t\t\"accessPermission\" : 1,\n\t\t\t\"advanceParams\" : \n\t\t\t{\n\t\t\t\t\"clientName\" : \"*\",\n\t\t\t\t\"clientType\" : 0,\n\t\t\t\t\"portSecure\" : 1,\n\t\t\t\t\"rootSquash\" : 1,\n\t\t\t\t\"squash\" : 1\n\t\t\t},\n\t\t\t\"fileSystemName\" : \"fileset_mount_202207251733\",\n\t\t\t\"type\" : 1\n\t\t}\n\t],\n\t\"performance\" : {}\n}\n";
    return cancelLivemountJob;
}

static AppProtect::CancelLivemountJob CancelLivemountJobSetupJsonTransFail()
{
    AppProtect::CancelLivemountJob cancelLivemountJob;
    cancelLivemountJob.requestId = "reqId123";
    cancelLivemountJob.jobId = "jId123";
 
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
 
    cancelLivemountJob.copy.repositories.push_back(cacheRepo);
    cancelLivemountJob.copy.repositories.push_back(dataRepo);
    cancelLivemountJob.copy.repositories.push_back(metaRepo);
 
    return cancelLivemountJob;
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

static AppProtect::CancelLivemountJob GetJobInfo_Null_Rep()
{
    AppProtect::CancelLivemountJob cancelLivemountJob = CancelLivemountJobSetup();
    cancelLivemountJob.copy.repositories.clear();
    return cancelLivemountJob;
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

static std::shared_ptr<CancelLivemountJob> GetJobInfoBody_nullptr()
{
    return nullptr;
}

void HostCancelLivemountTest::SetUp()
{
    stub.set(ADDR(JobService, ReportJobDetails), StubFunction_VOID);
}

void HostCancelLivemountTest::SetJobInfo()
{
    AppProtect::CancelLivemountJob cancelLivemountJob = CancelLivemountJobSetup();
    auto jobInfoLivemount = make_shared<JobCommonInfo>(make_shared<CancelLivemountJob>(cancelLivemountJob));
    m_hostCancelLeavemountObj.SetJobInfo(jobInfoLivemount);
}

void HostCancelLivemountTest::SetJsonTransFailJobInfo()
{
    AppProtect::CancelLivemountJob cancelLivemountJob = CancelLivemountJobSetupJsonTransFail();
    auto jobInfoLivemount = make_shared<JobCommonInfo>(make_shared<CancelLivemountJob>(cancelLivemountJob));
    m_hostCancelLeavemountObj.SetJobInfo(jobInfoLivemount);
}
 
TEST_F(HostCancelLivemountTest, testPrerequisiteJobInner)
{
    AppProtect::CancelLivemountJob cancelLivemountJob = CancelLivemountJobSetup();
    auto jobInfoLivemount = make_shared<JobCommonInfo>(make_shared<CancelLivemountJob>(cancelLivemountJob));
    m_hostCancelLeavemountObj.SetJobInfo(jobInfoLivemount);
 
    // Stub stubTemp;
    // typedef int (*fptr)(JobService*);
    // fptr JobService_ReportJobDetails = (fptr)(&JobService::ReportJobDetails);
    // stubTemp.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
 
    EXPECT_EQ(m_hostCancelLeavemountObj.PrerequisiteJob(), Module::SUCCESS);
}

// TEST_F(HostCancelLivemountTest, testPrerequisiteJob_Null_Info)
// {
//     stub.set(ADDR(HostCancelLivemount, GetJobInfoBody), GetJobInfoBody_nullptr);

//     // Stub stubTemp;
//     // typedef int (*fptr)(JobService*);
//     // fptr JobService_ReportJobDetails = (fptr)(&JobService::ReportJobDetails);
//     // stubTemp.set(JobService_ReportJobDetails, ReportJobDetailsSuc);

//     EXPECT_EQ(m_hostCancelLeavemountObj.PrerequisiteJob(), Module::FAILED);
// }

TEST_F(HostCancelLivemountTest, GenerateSubJob)
{
    SetJobInfo();
    stub.set(ADDR(JobService, AddNewJob), Stub_AddReturnValue_SUCCESS);

    EXPECT_EQ(m_hostCancelLeavemountObj.GenerateSubJob(), Module::SUCCESS);
}

// TEST_F(HostCancelLivemountTest, GenerateSubJob_Null_Info)
// {
//     stub.set(ADDR(HostCancelLivemount, GetJobInfoBody), GetJobInfoBody_nullptr);
//     EXPECT_EQ(m_hostCancelLeavemountObj.GenerateSubJob(), Module::FAILED);
// }

TEST_F(HostCancelLivemountTest, ExecuteSubJob)
{
    SetJobInfo();
    stub.set(ADDR(JobService, MountRepositoryByPlugin), Stub_AddReturnValue_SUCCESS);
    stub.set(ADDR(JobService, UnMountRepositoryByPlugin), Stub_AddReturnValue_SUCCESS);
    AppProtect::SubJob subJob = GetSubJob();
    m_hostCancelLeavemountObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostCancelLeavemountObj.ExecuteSubJob(), Module::SUCCESS);
}

TEST_F(HostCancelLivemountTest, ExecuteSubJobJsonTranFail)
{
    SetJsonTransFailJobInfo();
    stub.set(ADDR(JobService, MountRepositoryByPlugin), Stub_AddReturnValue_SUCCESS);
    stub.set(ADDR(JobService, UnMountRepositoryByPlugin), Stub_AddReturnValue_SUCCESS);
    AppProtect::SubJob subJob = GetSubJob();
    m_hostCancelLeavemountObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostCancelLeavemountObj.ExecuteSubJob(), Module::FAILED);
}

// TEST_F(HostCancelLivemountTest, ExecuteSubJob_Null_Info)
// {
//     stub.set(ADDR(HostCancelLivemount, GetJobInfoBody), GetJobInfoBody_nullptr);
//     EXPECT_EQ(m_hostCancelLeavemountObj.ExecuteSubJob(), Module::FAILED);
// }

TEST_F(HostCancelLivemountTest, ExecuteSubJob_null_Rep)
{
    AppProtect::CancelLivemountJob cancelLivemountJob = GetJobInfo_Null_Rep();
    auto jobInfoLivemount = make_shared<JobCommonInfo>(make_shared<CancelLivemountJob>(cancelLivemountJob));
    m_hostCancelLeavemountObj.SetJobInfo(jobInfoLivemount);
    stub.set(ADDR(JobService, MountRepositoryByPlugin), Stub_AddReturnValue_SUCCESS);
    stub.set(ADDR(JobService, UnMountRepositoryByPlugin), Stub_AddReturnValue_SUCCESS);
    AppProtect::SubJob subJob = GetSubJob();
    m_hostCancelLeavemountObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostCancelLeavemountObj.ExecuteSubJob(), Module::FAILED);
}

TEST_F(HostCancelLivemountTest, PostJob)
{
    SetJobInfo();
    EXPECT_EQ(m_hostCancelLeavemountObj.PostJob(), Module::SUCCESS);
}

// TEST_F(HostCancelLivemountTest, PostJob_Null_Info)
// {
//     stub.set(ADDR(HostCancelLivemount, GetJobInfoBody), GetJobInfoBody_nullptr);
//     EXPECT_EQ(m_hostCancelLeavemountObj.PostJob(), Module::FAILED);
// }

TEST_F(HostCancelLivemountTest, testCheckBlackList)
{
    EXPECT_EQ(m_hostCancelLeavemountObj.CheckBlackList("/bin"), false);
    EXPECT_EQ(m_hostCancelLeavemountObj.CheckBlackList("/boot/sys"), false);
    EXPECT_EQ(m_hostCancelLeavemountObj.CheckBlackList("/tmp"), true);
}
