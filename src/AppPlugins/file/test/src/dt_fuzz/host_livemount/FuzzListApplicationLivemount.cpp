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
#include "secodeFuzz.h"
#include "host_livemount/HostLivemount.h"
 
using namespace std;
using namespace AppProtect;
using namespace FilePlugin;
 
class FuzzListApplicationLivemount : public testing::Test {
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

void FuzzListApplicationLivemount::TearDown()
{}
 
void FuzzListApplicationLivemount::SetUpTestCase()
{}
 
void FuzzListApplicationLivemount::TearDownTestCase()
{}
 
static void ReportJobDetailsSuc(void *obj, ActionResult& returnValue, const SubJobDetails& jobInfo)
{
    return;
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

void FuzzListApplicationLivemount::SetUp()
{
    stub.set(ADDR(JobService, ReportJobDetails), StubFunction_VOID);
}

void FuzzListApplicationLivemount::SetJobInfo(){
    AppProtect::LivemountJob livemountJob;
    livemountJob.requestId = "reqId123";
    livemountJob.jobId = "jId123";
    string path = "/MetaFS/cache";
    HostAddress hostadd;
    hostadd.ip = "10.28.12.144";
    StorageRepository cacheRepo;
    cacheRepo.repositoryType = RepositoryDataType::CACHE_REPOSITORY;
    cacheRepo.path.push_back(path);
    cacheRepo.remotePath = DT_SetGetString(&g_Element[0], 11, 100, (char *)"path//test");
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
    dataRepo.auth.authPwd = DT_SetGetString(&g_Element[1], 10, 100, (char *)"Admin@123");
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
    auto jobInfoLivemount = make_shared<JobCommonInfo>(make_shared<LivemountJob>(livemountJob));
    m_hostLivemountObj.SetJobInfo(jobInfoLivemount);
}

TEST_F(FuzzListApplicationLivemount, testPrerequisiteJobInner_OK)
{
    DT_FUZZ_START(0, 10, (char*)"FuzzLivemount_Prerequisite", 0){
        SetJobInfo();
        EXPECT_EQ(m_hostLivemountObj.PrerequisiteJob(), Module::SUCCESS);
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzListApplicationLivemount, testPrerequisiteJob_Null_Info)
{
    stub.set(ADDR(HostLivemount, GetJobInfoBody), GetJobInfoBody_nullptr);
    DT_FUZZ_START(0, 10, (char*)"FuzzLivemount_Prerequisite_Null_Info", 0){
        EXPECT_EQ(m_hostLivemountObj.PrerequisiteJob(), Module::FAILED);
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzListApplicationLivemount, GenerateSubJob_OK)
{
    stub.set(ADDR(JobService, AddNewJob), Stub_AddReturnValue_SUCCESS);
    DT_FUZZ_START(0, 10,(char*)"FuzzLivemount_generateSubJob" ,0){
        SetJobInfo();
        EXPECT_EQ(m_hostLivemountObj.GenerateSubJob(), Module::SUCCESS);
    }   
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzListApplicationLivemount, GenerateSubJob_Null_Info)
{
    stub.set(ADDR(HostLivemount, GetJobInfoBody), GetJobInfoBody_nullptr);
    DT_FUZZ_START(0, 10,(char*)"FuzzLivemount_generateSubJob_Null_Info" ,0){
        EXPECT_EQ(m_hostLivemountObj.GenerateSubJob(), Module::FAILED);
    }
    DT_FUZZ_END()
    return;
}


// TEST_F(FuzzListApplicationLivemount, ExecuteSubJob_OK)
// {
//     DT_FUZZ_START(0, 10,(char*)"FuzzLivemount_exeSubJob" ,0){
//         SetJobInfo();
//         stub.set(ADDR(JobService, MountRepositoryByPlugin), Stub_AddReturnValue_SUCCESS);
//         stub.set(ADDR(JobService, UnMountRepositoryByPlugin), Stub_AddReturnValue_SUCCESS);
//         AppProtect::SubJob subJob;
//         subJob.jobId = "jobId123";
//         subJob.subJobId = "subJob123";
//         subJob.jobName = "subJob";
//         subJob.jobPriority = 1;
//         subJob.ignoreFailed = true;
//         subJob.execNodeId = "abcde";
//         subJob.jobInfo = "{\"ControlFile\":\"/cache/livemount-job/scan/ctrl\","
//                         "\"SubTaskType\":1,"
//                         "\"Ext\":\"abc\"}";
//         m_hostLivemountObj.SetSubJob(std::make_shared<SubJob>(subJob));
//         EXPECT_EQ(m_hostLivemountObj.ExecuteSubJob(), Module::SUCCESS); 
           
//     }
//     DT_FUZZ_END()
//     return;
// }

TEST_F(FuzzListApplicationLivemount, ExecuteSubJob_Null_Info)
{
    stub.set(ADDR(HostLivemount, GetJobInfoBody), GetJobInfoBody_nullptr);
    DT_FUZZ_START(0, 10, (char*)"FuzzLivemount_PostJob_Null_Info" ,0){
        EXPECT_EQ(m_hostLivemountObj.ExecuteSubJob(), Module::FAILED);
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzListApplicationLivemount, PostJob_OK)
{
    DT_FUZZ_START(0, 10, (char*)"FuzzLivemount_PostJob" ,0){
        SetJobInfo();
        EXPECT_EQ(m_hostLivemountObj.PostJob(), Module::SUCCESS);
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzListApplicationLivemount, PostJob_Null_Info)
{
    stub.set(ADDR(HostLivemount, GetJobInfoBody), GetJobInfoBody_nullptr);
    DT_FUZZ_START(0, 10, (char*)"FuzzLivemount_PostJob_Null_Info", 0){
        EXPECT_EQ(m_hostLivemountObj.PostJob(), Module::FAILED);
    }
    DT_FUZZ_END()
    return;
}
