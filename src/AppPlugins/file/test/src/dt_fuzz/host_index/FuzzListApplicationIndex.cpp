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
#include "HostIndex.h"
#include "secodeFuzz.h"
#include "client/ClientInvoke.h"

using namespace std;
using namespace AppProtect;
using namespace FilePlugin;

class Scanner_test_Index : public Scanner {
public:
    explicit Scanner_test_Index(const ScanConfig& scanConfig);
    ~Scanner_test_Index() {};

    SCANNER_STATUS Start() override;
    SCANNER_STATUS Abort()override;
    SCANNER_STATUS Destroy() override;
    SCANNER_STATUS Enqueue(const std::string& directory, const std::string& prefix = "", uint8_t filterFlag = 0) override;
    SCANNER_STATUS EnqueueV2(const std::string& directory) override;
    SCANNER_STATUS GetStatus() override;
    ScanStatistics GetStatistics() override;

protected:
    ScanConfig m_config;
};

class Scanner_test_Index_Abort : public Scanner {
public:
    explicit Scanner_test_Index_Abort(const ScanConfig& scanConfig);
    ~Scanner_test_Index_Abort() {};

    SCANNER_STATUS Start() override;
    SCANNER_STATUS Abort()override;
    SCANNER_STATUS Destroy() override;
    SCANNER_STATUS Enqueue(const std::string& directory, const std::string& prefix = "", uint8_t filterFlag = 0) override;
    SCANNER_STATUS EnqueueV2(const std::string& directory) override;
    SCANNER_STATUS GetStatus() override;
    ScanStatistics GetStatistics() override;

protected:
    ScanConfig m_config;
};

class Scanner_test_Index_Failed : public Scanner {
public:
    explicit Scanner_test_Index_Failed(const ScanConfig& scanConfig);
    ~Scanner_test_Index_Failed() {};

    SCANNER_STATUS Start() override;
    SCANNER_STATUS Abort()override;
    SCANNER_STATUS Destroy() override;
    SCANNER_STATUS Enqueue(const std::string& directory, const std::string& prefix = "", uint8_t filterFlag = 0) override;
    SCANNER_STATUS EnqueueV2(const std::string& directory) override;
    SCANNER_STATUS GetStatus() override;
    ScanStatistics GetStatistics() override;

protected:
    ScanConfig m_config;
};

Scanner_test_Index::Scanner_test_Index(const ScanConfig& scanConfig) : Scanner(scanConfig) {}
SCANNER_STATUS Scanner_test_Index::Start() {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test_Index::Abort() {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test_Index::Destroy() {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test_Index::Enqueue(const std::string& directory, const std::string& prefix, uint8_t filterFlag) {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test_Index::EnqueueV2(const std::string& directory) {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test_Index::GetStatus() {
    return SCANNER_STATUS::COMPLETED;
}
ScanStatistics Scanner_test_Index::GetStatistics() {
    ScanStatistics scanStatistics;
    return scanStatistics;
}

Scanner_test_Index_Abort::Scanner_test_Index_Abort(const ScanConfig& scanConfig) : Scanner(scanConfig) {}
SCANNER_STATUS Scanner_test_Index_Abort::Start() {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test_Index_Abort::Abort() {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test_Index_Abort::Destroy() {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test_Index_Abort::Enqueue(const std::string& directory, const std::string& prefix, uint8_t filterFlag) {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test_Index_Abort::EnqueueV2(const std::string& directory) {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test_Index_Abort::GetStatus() {
    return SCANNER_STATUS::ABORTED;
}
ScanStatistics Scanner_test_Index_Abort::GetStatistics() {
    ScanStatistics scanStatistics;
    return scanStatistics;
}


Scanner_test_Index_Failed::Scanner_test_Index_Failed(const ScanConfig& scanConfig) : Scanner(scanConfig) {}
SCANNER_STATUS Scanner_test_Index_Failed::Start() {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test_Index_Failed::Abort() {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test_Index_Failed::Destroy() {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test_Index_Failed::Enqueue(const std::string& directory, const std::string& prefix, uint8_t filterFlag) {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test_Index_Failed::EnqueueV2(const std::string& directory) {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test_Index_Failed::GetStatus() {
    return SCANNER_STATUS::PROTECTED_SERVER_NOT_REACHABLE;
}
ScanStatistics Scanner_test_Index_Failed::GetStatistics() {
    ScanStatistics scanStatistics;
    return scanStatistics;
}

class FuzzListApplicationIndex : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetJobInfo();
    void SetIncJobInfo();
    Stub stub;
    HostIndex m_indexObj;
};

void FuzzListApplicationIndex::SetUpTestCase()
{}
 
void FuzzListApplicationIndex::TearDownTestCase()
{}

static BuildIndexJob BuildIndexJobSetup()
{
    BuildIndexJob buildIndexJob;
    buildIndexJob.requestId = "reqId123";
    buildIndexJob.jobId = "jId123";
 
    string path = "/MetaFS/cache";
    HostAddress hostadd;
    hostadd.ip="10.28.12.144";
    StorageRepository cacheRepo;
    cacheRepo.repositoryType = RepositoryDataType::type::CACHE_REPOSITORY;
    cacheRepo.path.push_back(path);
    cacheRepo.remotePath = DT_SetGetString(&g_Element[0], 11, 100, (char *)"path//test");
    cacheRepo.remoteHost.push_back(hostadd);
    cacheRepo.auth.authkey = "admin";
    cacheRepo.auth.authPwd = "Admin@123";
    cacheRepo.endpoint.ip = "10.28.12.144";
    cacheRepo.endpoint.port = 8088;
 
    StorageRepository dataRepo;
    dataRepo.repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
    dataRepo.remotePath = "/BackupFS";
    dataRepo.path.push_back(dataRepo.remotePath);
    dataRepo.remoteHost.push_back(hostadd);
    dataRepo.auth.authkey = "admin";
    dataRepo.auth.authPwd = "Admin@123";
    dataRepo.endpoint.ip = "10.28.12.144";
    dataRepo.endpoint.port = 8088;
    dataRepo.extendInfo = R"({"isCurrentCopyRepo": true, "timestamp": 123, "copyId": "123", "fsId":"123"})";
 
    StorageRepository metaRepo;
    metaRepo.repositoryType = RepositoryDataType::type::META_REPOSITORY;
    path = "/MetaFS/meta";
    metaRepo.path.push_back(path);
    metaRepo.remotePath = "/MetaFS/meta";
    metaRepo.remoteHost.push_back(hostadd);
    metaRepo.auth.authkey = "admin";
    metaRepo.auth.authPwd = "Admin@123";
    metaRepo.endpoint.ip = "10.28.12.144";
    metaRepo.endpoint.port = 8088;
    metaRepo.extendInfo = R"({"isCurrentCopyRepo": true, "timestamp": 123, "copyId": "123", "fsId":"123"})";

    StorageRepository indexRepo;
    indexRepo.repositoryType = RepositoryDataType::type::INDEX_REPOSITORY;
    path = "/MetaFS/meta";
    indexRepo.path.push_back(path);
    indexRepo.remotePath = "/MetaFS/meta";
    indexRepo.remoteHost.push_back(hostadd);
    indexRepo.auth.authkey = "admin";
    indexRepo.auth.authPwd = "Admin@123";
    indexRepo.endpoint.ip = "10.28.12.144";
    indexRepo.endpoint.port = 8088;

    Copy copy;
    copy.repositories.push_back(cacheRepo); 
    copy.repositories.push_back(dataRepo);
    copy.repositories.push_back(metaRepo);
 
    buildIndexJob.copies.push_back(copy);
    buildIndexJob.repositories.push_back(indexRepo);
 
    buildIndexJob.extendInfo = "{\n\t\"dstPath\" : \"/opt\",\n\t\"fileSystemShareInfo\" : \n\t[\n\t\t{\n\t\t\t\"accessPermission\" : 1,\n\t\t\t\"advanceParams\" : \n\t\t\t{\n\t\t\t\t\"clientName\" : \"*\",\n\t\t\t\t\"clientType\" : 0,\n\t\t\t\t\"portSecure\" : 1,\n\t\t\t\t\"rootSquash\" : 1,\n\t\t\t\t\"squash\" : 1\n\t\t\t},\n\t\t\t\"fileSystemName\" : \"fileset_mount_202207251733\",\n\t\t\t\"type\" : 1\n\t\t}\n\t],\n\t\"performance\" : {}\n}\n";
    return buildIndexJob;
}

static BuildIndexJob BuildIndexJobSetupInc()
{
    BuildIndexJob buildIndexJob;
    buildIndexJob.requestId = "reqId123";
    buildIndexJob.jobId = "jId123";
 
    string path = "/MetaFS/cache";
    HostAddress hostadd;
    hostadd.ip="10.28.12.144";
    StorageRepository cacheRepo;
    cacheRepo.repositoryType = RepositoryDataType::type::CACHE_REPOSITORY;
    cacheRepo.path.push_back(path);
    cacheRepo.remotePath = "/MetaFS/cache";
    cacheRepo.remoteHost.push_back(hostadd);
    cacheRepo.auth.authkey = "admin";
    cacheRepo.auth.authPwd = "Admin@123";
    cacheRepo.endpoint.ip = "10.28.12.144";
    cacheRepo.endpoint.port = 8088;
 
    StorageRepository dataRepo;
    dataRepo.repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
    dataRepo.remotePath = "/BackupFS";
    dataRepo.path.push_back(dataRepo.remotePath);
    dataRepo.remoteHost.push_back(hostadd);
    dataRepo.auth.authkey = "admin";
    dataRepo.auth.authPwd = "Admin@123";
    dataRepo.endpoint.ip = "10.28.12.144";
    dataRepo.endpoint.port = 8088;
    dataRepo.extendInfo = R"({"isCurrentCopyRepo": true, "timestamp": 123, "copyId": "123", "fsId":"123"})";

    StorageRepository dataRepoPre;
    dataRepoPre.repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
    dataRepoPre.remotePath = "/BackupFS";
    dataRepoPre.path.push_back(dataRepo.remotePath);
    dataRepoPre.remoteHost.push_back(hostadd);
    dataRepoPre.auth.authkey = "admin";
    dataRepoPre.auth.authPwd = "Admin@123";
    dataRepoPre.endpoint.ip = "10.28.12.144";
    dataRepoPre.endpoint.port = 8088;
    dataRepoPre.extendInfo = R"({"isCurrentCopyRepo": false, "timestamp": 123, "copyId": "123", "fsId":"123"})";
 
    StorageRepository metaRepo;
    metaRepo.repositoryType = RepositoryDataType::type::META_REPOSITORY;
    path = "/MetaFS/meta";
    metaRepo.path.push_back(path);
    metaRepo.remotePath = "/MetaFS/meta";
    metaRepo.remoteHost.push_back(hostadd);
    metaRepo.auth.authkey = "admin";
    metaRepo.auth.authPwd = "Admin@123";
    metaRepo.endpoint.ip = "10.28.12.144";
    metaRepo.endpoint.port = 8088;
    metaRepo.extendInfo = R"({"isCurrentCopyRepo": true, "timestamp": 123, "copyId": "123", "fsId":"123"})";

    StorageRepository metaRepoPre;
    metaRepoPre.repositoryType = RepositoryDataType::type::META_REPOSITORY;
    path = "/MetaFS/meta";
    metaRepoPre.path.push_back(path);
    metaRepoPre.remotePath = "/MetaFS/meta";
    metaRepoPre.remoteHost.push_back(hostadd);
    metaRepoPre.auth.authkey = "admin";
    metaRepoPre.auth.authPwd = "Admin@123";
    metaRepoPre.endpoint.ip = "10.28.12.144";
    metaRepoPre.endpoint.port = 8088;
    metaRepoPre.extendInfo = R"({"isCurrentCopyRepo": false, "timestamp": 123, "copyId": "123", "fsId":"123"})";

    StorageRepository indexRepo;
    indexRepo.repositoryType = RepositoryDataType::type::INDEX_REPOSITORY;
    path = "/MetaFS/meta";
    indexRepo.path.push_back(path);
    indexRepo.remotePath = "/MetaFS/meta";
    indexRepo.remoteHost.push_back(hostadd);
    indexRepo.auth.authkey = "admin";
    indexRepo.auth.authPwd = "Admin@123";
    indexRepo.endpoint.ip = "10.28.12.144";
    indexRepo.endpoint.port = 8088;

    Copy copy;
    copy.repositories.push_back(cacheRepo); 
    copy.repositories.push_back(dataRepo);
    copy.repositories.push_back(metaRepo);
    copy.repositories.push_back(metaRepoPre);
    copy.repositories.push_back(dataRepoPre);
 
    buildIndexJob.copies.push_back(copy);
    buildIndexJob.repositories.push_back(indexRepo);
 
    buildIndexJob.extendInfo = "{\n\t\"dstPath\" : \"/opt\",\n\t\"fileSystemShareInfo\" : \n\t[\n\t\t{\n\t\t\t\"accessPermission\" : 1,\n\t\t\t\"advanceParams\" : \n\t\t\t{\n\t\t\t\t\"clientName\" : \"*\",\n\t\t\t\t\"clientType\" : 0,\n\t\t\t\t\"portSecure\" : 1,\n\t\t\t\t\"rootSquash\" : 1,\n\t\t\t\t\"squash\" : 1\n\t\t\t},\n\t\t\t\"fileSystemName\" : \"fileset_mount_202207251733\",\n\t\t\t\"type\" : 1\n\t\t}\n\t],\n\t\"performance\" : {}\n}\n";
    return buildIndexJob;
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

void FuzzListApplicationIndex::SetJobInfo()
{
    AppProtect::BuildIndexJob indexJob = BuildIndexJobSetup();
    auto jobInfoIndex = make_shared<JobCommonInfo>(make_shared<BuildIndexJob>(indexJob));
    m_indexObj.SetJobInfo(jobInfoIndex);
}

void FuzzListApplicationIndex::SetIncJobInfo()
{
    AppProtect::BuildIndexJob indexJob = BuildIndexJobSetupInc();
    auto jobInfoIndex = make_shared<JobCommonInfo>(make_shared<BuildIndexJob>(indexJob));
    m_indexObj.SetJobInfo(jobInfoIndex);
}

static void Stub_AddReturnValue_SUCCESS(ActionResult& returnValue, void *obj)
{
    returnValue.code = Module::SUCCESS;
}

static void Stub_AddReturnValue_FAILED(ActionResult& returnValue, void *obj)
{
    returnValue.code = Module::FAILED;
}

static void Stub_PrepareForGenerateRfi(void* obj, string s1, string s2)
{}

static int Stub_ReturnSUCCESS(void* obj)
{
    return Module::SUCCESS;
}

static int Stub_ReturnFAILED(void* obj)
{
    return Module::FAILED;
}

static bool Stub_CheckDcacheExist(string s1)
{
    return true;
}

static void StubFunction_VOID(void *ob)
{}

static std::shared_ptr<BuildIndexJob> GetJobInfoBody_nullptr()
{
    return nullptr;
}

static SCANNER_STATUS Stub_ScanStat_SUCCESS(void *ob)
{
    return SCANNER_STATUS::SUCCESS;
}

static SCANNER_STATUS Stub_ScanStat_COMPLETED(void *ob)
{
    return SCANNER_STATUS::COMPLETED;
}

static unique_ptr<Scanner> CreateScanInst_Stub (void* obj, ScanConfig scanConfig) {
    ScanConfig xx;
    return make_unique<Scanner_test_Index>(xx);
}

static unique_ptr<Scanner> CreateScanInstAbort_Stub (void* obj, ScanConfig scanConfig) {
    ScanConfig xx;
    return make_unique<Scanner_test_Index_Abort>(xx);
}

static unique_ptr<Scanner> CreateScanInstFailed_Stub (void* obj, ScanConfig scanConfig) {
    ScanConfig xx;
    return make_unique<Scanner_test_Index_Failed>(xx);
}

static unique_ptr<Scanner> CreateScanInstNullptr_Stub (void* obj, ScanConfig scanConfig) {
    return nullptr;
}

static int Stub_PrepareForGenrateRfi(void* obj, string path1, string path2)
{
    reinterpret_cast<HostIndex*>(obj)->m_isPreparing = false;
}

static int Stub_UnzipMetafileToCurPathAndRemoveAsync(void* obj, const string& path1, promise<int>& promiseObj)
{
    reinterpret_cast<HostIndex*>(obj)->m_isPreparing = false;
    promiseObj.set_value(Module::SUCCESS);
}

static int Stub_PrepareForGenerateIncRfiInPath(void* obj, const string& path1, const string& path2, promise<int>& promiseObj)
{
    reinterpret_cast<HostIndex*>(obj)->m_isPreparing = false;
    promiseObj.set_value(Module::SUCCESS);
}

void FuzzListApplicationIndex::SetUp()
{
    stub.set(ADDR(JobService, ReportJobDetails), StubFunction_VOID);
    // stub.set(ADDR(HostIndex, ProcessFullHostIndex), Stub_ReturnSUCCESS);
    // stub.set(ADDR(HostIndex, GenerateFullRfiInPath), Stub_ReturnSUCCESS);
    // stub.set(ADDR(HostIndex, GenerateIncRfiInPath), Stub_ReturnSUCCESS);
    // stub.set(ADDR(HostIndex, ProcessIncHostIndex), Stub_ReturnSUCCESS);
    stub.set(ADDR(HostIndex, PrepareForGenrateRfi), Stub_PrepareForGenrateRfi);
    stub.set(ADDR(HostIndex, UnzipMetafileToCurPathAndRemoveAsync), Stub_UnzipMetafileToCurPathAndRemoveAsync);
    stub.set(ADDR(HostIndex, PrepareForGenerateIncRfiInPath), Stub_PrepareForGenerateIncRfiInPath);
}

void FuzzListApplicationIndex::TearDown()
{
    // stub.set(ADDR(JobService, ReportJobDetails), StubFunction_VOID);
}

TEST_F(FuzzListApplicationIndex, testPrerequisiteJobInner)
{
    DT_FUZZ_START(0, 10,(char*)"FuzzHostIndexPreJobInner" ,0){
        SetJobInfo();
        m_indexObj.PrerequisiteJob();
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzListApplicationIndex, testPrerequisiteJobInnerNullptr)
{
    stub.set(ADDR(HostIndex, GetJobInfoBody), GetJobInfoBody_nullptr); 
    DT_FUZZ_START(0, 10,(char*)"FuzzHostIndexPreJobInnerNullptr" ,0){
        SetJobInfo(); 
        m_indexObj.PrerequisiteJob();
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzListApplicationIndex, testGenerateSubJob)
{
    stub.set(ADDR(JobService, AddNewJob), Stub_AddReturnValue_SUCCESS);
    DT_FUZZ_START(0, 10,(char*)"FuzzHostIndexGenSubJob" ,0){
        SetJobInfo();
        m_indexObj.GenerateSubJob();
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzListApplicationIndex, testGenerateSubJobNullptr)
{
    stub.set(ADDR(JobService, AddNewJob), Stub_AddReturnValue_SUCCESS);
    stub.set(ADDR(HostIndex, GetJobInfoBody), GetJobInfoBody_nullptr);
    DT_FUZZ_START(0, 10,(char*)"FuzzHostIndexGenSubJobNullptr" ,0){
        SetJobInfo();
        m_indexObj.GenerateSubJob();
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzListApplicationIndex, testExecuteSubJob)
{
    // prepare 函数打桩
    stub.set(ADDR(HostIndex, CheckDcacheExist), Stub_CheckDcacheExist);
    // scanner start 函数打桩， getStatus函数打桩
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInst_Stub);
    DT_FUZZ_START(0, 10,(char*)"FuzzHostIndexExeSubJob" ,0){
        SetJobInfo();


        AppProtect::SubJob subJob = GetSubJob();
        m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
        m_indexObj.ExecuteSubJob();
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzListApplicationIndex, testExecuteSubJobScannerAbort)
{
    SetJobInfo();
    // prepare 函数打桩
    stub.set(ADDR(HostIndex, CheckDcacheExist), Stub_CheckDcacheExist);
    // scanner start 函数打桩， getStatus函数打桩
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInstAbort_Stub);
    DT_FUZZ_START(0, 10,(char*)"FuzzHostIndexExeSubJobNullptr" ,0){

        AppProtect::SubJob subJob = GetSubJob();
        m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
        m_indexObj.ExecuteSubJob();
    }
    DT_FUZZ_END()
    return;   
}

// TEST_F(FuzzListApplicationIndex, testExecuteSubJobScannerFailed)
// {
//     SetJobInfo();
//     // prepare 函数打桩
//     stub.set(ADDR(HostIndex, CheckDcacheExist), Stub_CheckDcacheExist);
//     // scanner start 函数打桩， getStatus函数打桩
//     stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInstFailed_Stub);
//     DT_FUZZ_START(0, 10,(char*)"FuzzHostIndexExeSubJobNullptr" ,0){

//         AppProtect::SubJob subJob = GetSubJob();
//         m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
//         m_indexObj.ExecuteSubJob();
//     }
//     DT_FUZZ_END()
//     return;   
// }

// TEST_F(FuzzListApplicationIndex, testExecuteSubJobScannerNullptr)
// {
//     SetJobInfo();
//     // prepare 函数打桩
//     stub.set(ADDR(HostIndex, CheckDcacheExist), Stub_CheckDcacheExist);
//     // scanner start 函数打桩， getStatus函数打桩
//     stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInstNullptr_Stub);
//     DT_FUZZ_START(0, 10,(char*)"FuzzHostIndexExeSubJobNullptr" ,0){

//         AppProtect::SubJob subJob = GetSubJob();
//         m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
//         m_indexObj.ExecuteSubJob();
//     }
//     DT_FUZZ_END()
//     return;
// }

TEST_F(FuzzListApplicationIndex, testExecuteSubJobInc)
{
    SetIncJobInfo();
    // prepare 函数打桩
    stub.set(ADDR(HostIndex, CheckDcacheExist), Stub_CheckDcacheExist);
    // scanner start 函数打桩， getStatus函数打桩
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInst_Stub);
    DT_FUZZ_START(0, 10,(char*)"FuzzHostIndexExeSubJobNullptr" ,0){

        AppProtect::SubJob subJob = GetSubJob();
        m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
        m_indexObj.ExecuteSubJob();
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzListApplicationIndex, testExecuteSubJobIncDcacheNotExist)
{
    SetIncJobInfo();
    // prepare 函数打桩
    // scanner start 函数打桩， getStatus函数打桩
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInst_Stub);
    DT_FUZZ_START(0, 10,(char*)"FuzzHostIndexExeSubJobNullptr" ,0){
        AppProtect::SubJob subJob = GetSubJob();
        m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
        m_indexObj.ExecuteSubJob();
    }
    DT_FUZZ_END()
    return;
}

// TEST_F(FuzzListApplicationIndex, testExecuteSubJobNullptr)
// {
//     stub.set(ADDR(HostIndex, GetJobInfoBody), GetJobInfoBody_nullptr);
//     // prepare 函数打桩
//     stub.set(ADDR(HostIndex, CheckDcacheExist), Stub_CheckDcacheExist);
//     // scanner start 函数打桩， getStatus函数打桩
//     stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInst_Stub);
//     DT_FUZZ_START(0, 10,(char*)"FuzzHostIndexExeSubJobNullptr" ,0){
//         SetJobInfo();
//         AppProtect::SubJob subJob = GetSubJob();
//         m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
//         m_indexObj.ExecuteSubJob();
//     }
//     DT_FUZZ_END()
//     return;
// }

// TEST_F(FuzzListApplicationIndex, testPostJob)
// {
//     DT_FUZZ_START(0, 10, (char*)"FuzzHostIndexPostJob" ,0){
//         SetJobInfo();
//         m_indexObj.PostJob();
//     }
//     DT_FUZZ_END()
//     return;
// }

