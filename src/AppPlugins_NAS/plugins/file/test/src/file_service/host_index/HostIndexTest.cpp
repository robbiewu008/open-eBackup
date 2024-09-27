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
#include "client/ClientInvoke.h"
#include "PluginUtilities.h"

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

class HostIndexTest : public testing::Test {
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

void HostIndexTest::SetUpTestCase()
{}

void HostIndexTest::TearDownTestCase()
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

void HostIndexTest::SetJobInfo()
{
    AppProtect::BuildIndexJob indexJob = BuildIndexJobSetup();
    auto jobInfoIndex = make_shared<JobCommonInfo>(make_shared<BuildIndexJob>(indexJob));
    m_indexObj.SetJobInfo(jobInfoIndex);
}

void HostIndexTest::SetIncJobInfo()
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

static void Stub_PrepareForGenerateRfi(HostIndex* obj, string s1, string s2)
{
    obj->m_isPreparing = false;
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

static void returnVoidStub(void* obj)
{
    return;
}

void HostIndexTest::SetUp()
{
    stub.set(sleep, returnVoidStub);
    stub.set(ADDR(JobService, ReportJobDetails), StubFunction_VOID);
}

void HostIndexTest::TearDown()
{
    // stub.set(ADDR(JobService, ReportJobDetails), StubFunction_VOID);
}

TEST_F(HostIndexTest, testPrerequisiteJobInner)
{
    SetJobInfo();
    EXPECT_EQ(m_indexObj.PrerequisiteJob(), Module::SUCCESS);
}

TEST_F(HostIndexTest, testPrerequisiteJobInnerNullptr)
{
    SetJobInfo();
    stub.set(ADDR(HostIndex, GetJobInfoBody), GetJobInfoBody_nullptr);
    EXPECT_EQ(m_indexObj.PrerequisiteJob(), Module::FAILED);
}

TEST_F(HostIndexTest, testGenerateSubJob)
{
    SetJobInfo();
    stub.set(ADDR(JobService, AddNewJob), Stub_AddReturnValue_SUCCESS);

    EXPECT_EQ(m_indexObj.GenerateSubJob(), Module::SUCCESS);
}

TEST_F(HostIndexTest, testGenerateSubJobNullptr)
{
    SetJobInfo();
    stub.set(ADDR(JobService, AddNewJob), Stub_AddReturnValue_SUCCESS);
    stub.set(ADDR(HostIndex, GetJobInfoBody), GetJobInfoBody_nullptr);
    EXPECT_EQ(m_indexObj.GenerateSubJob(), Module::FAILED);
}

TEST_F(HostIndexTest, testExecuteSubJob)
{
    SetJobInfo();
    // prepare 函数打桩
    stub.set(ADDR(HostIndex, PrepareForGenrateRfi), Stub_PrepareForGenerateRfi);
    stub.set(ADDR(HostIndex, CheckDcacheExist), Stub_CheckDcacheExist);
    // scanner start 函数打桩， getStatus函数打桩
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInst_Stub);

    AppProtect::SubJob subJob = GetSubJob();
    m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_indexObj.ExecuteSubJob(), Module::SUCCESS);
}


TEST_F(HostIndexTest, testExecuteSubJobDcacheNotExist)
{
    SetJobInfo();
    // prepare 函数打桩
    stub.set(ADDR(HostIndex, PrepareForGenrateRfi), Stub_PrepareForGenerateRfi);
    // scanner start 函数打桩， getStatus函数打桩
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInst_Stub);

    AppProtect::SubJob subJob = GetSubJob();
    m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_indexObj.ExecuteSubJob(), Module::FAILED);
}

static void Stub_Remove()
{}
static int Stub_Return_0()
{}
TEST_F(HostIndexTest, PrepareForGenrateRfi)
{
    SetJobInfo();

    stub.set(PluginUtils::Remove, Stub_Remove);
    stub.set(PluginUtils::CreateDirectory, Stub_Remove);
    stub.set(PluginUtils::Remove, Stub_Remove);
    // stub.set(Module::runShellCmdWithOutput, Stub_Return_0);
    m_indexObj.m_cacheRepo = std::make_shared<AppProtect::StorageRepository>();
    m_indexObj.m_metaRepo = std::make_shared<AppProtect::StorageRepository>();
    m_indexObj.m_preMetaRepo = std::make_shared<AppProtect::StorageRepository>();
    std::vector<std::string> tpath = {"111"};
    m_indexObj.m_cacheRepo->path = tpath;
    m_indexObj.m_metaRepo->path = tpath;
    m_indexObj.m_preMetaRepo->path = tpath;

    string preMetaFilePath = "";
    string curMetafilepath = "222";
    AppProtect::SubJob subJob = GetSubJob();
    m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_NO_THROW(m_indexObj.PrepareForGenrateRfi(preMetaFilePath, curMetafilepath));

    preMetaFilePath = "111";
    EXPECT_NO_THROW(m_indexObj.PrepareForGenrateRfi(preMetaFilePath, curMetafilepath));

    // stub.reset(Module::runShellCmdWithOutput);
}

static bool GetDirListInDirectory_stub(std::string dir, std::vector<string>& dirList)
{
    std::string dirPath = "/home/tmp";
    dirList.emplace_back(dirPath);
    return true;
}

TEST_F(HostIndexTest, testExecuteSubJobScannerAbort)
{
    SetJobInfo();
    // prepare 函数打桩
    stub.set(ADDR(HostIndex, PrepareForGenrateRfi), Stub_PrepareForGenerateRfi);
    stub.set(ADDR(HostIndex, CheckDcacheExist), Stub_CheckDcacheExist);
    stub.set(PluginUtils::GetDirListInDirectory, GetDirListInDirectory_stub);
    // scanner start 函数打桩， getStatus函数打桩
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInstAbort_Stub);

    AppProtect::SubJob subJob = GetSubJob();
    m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_indexObj.ExecuteSubJob(), Module::FAILED);
}

TEST_F(HostIndexTest, testExecuteSubJobScannerFailed)
{
    SetJobInfo();
    // prepare 函数打桩
    stub.set(ADDR(HostIndex, PrepareForGenrateRfi), Stub_PrepareForGenerateRfi);
    stub.set(ADDR(HostIndex, CheckDcacheExist), Stub_CheckDcacheExist);
    stub.set(PluginUtils::GetDirListInDirectory, GetDirListInDirectory_stub);
    // scanner start 函数打桩， getStatus函数打桩
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInstFailed_Stub);

    AppProtect::SubJob subJob = GetSubJob();
    m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_indexObj.ExecuteSubJob(), Module::FAILED);
}

TEST_F(HostIndexTest, testExecuteSubJobScannerNullptr)
{
    SetJobInfo();
    // prepare 函数打桩
    stub.set(ADDR(HostIndex, PrepareForGenrateRfi), Stub_PrepareForGenerateRfi);
    stub.set(ADDR(HostIndex, CheckDcacheExist), Stub_CheckDcacheExist);
    stub.set(PluginUtils::GetDirListInDirectory, GetDirListInDirectory_stub);
    // scanner start 函数打桩， getStatus函数打桩
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInstNullptr_Stub);

    AppProtect::SubJob subJob = GetSubJob();
    m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_indexObj.ExecuteSubJob(), Module::FAILED);
}

TEST_F(HostIndexTest, testExecuteSubJobInc)
{
    SetIncJobInfo();
    // prepare 函数打桩
    stub.set(ADDR(HostIndex, PrepareForGenrateRfi), Stub_PrepareForGenerateRfi);
    stub.set(ADDR(HostIndex, CheckDcacheExist), Stub_CheckDcacheExist);
    // scanner start 函数打桩， getStatus函数打桩
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInst_Stub);

    AppProtect::SubJob subJob = GetSubJob();
    m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_indexObj.ExecuteSubJob(), Module::SUCCESS);
}

TEST_F(HostIndexTest, testExecuteSubJobIncDcacheNotExist)
{
    SetIncJobInfo();
    // prepare 函数打桩
    stub.set(ADDR(HostIndex, PrepareForGenrateRfi), Stub_PrepareForGenerateRfi);
    // scanner start 函数打桩， getStatus函数打桩
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInst_Stub);

    AppProtect::SubJob subJob = GetSubJob();
    m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_indexObj.ExecuteSubJob(), Module::FAILED);
}

TEST_F(HostIndexTest, testExecuteSubJobNullptr)
{
    SetJobInfo();
    stub.set(ADDR(HostIndex, GetJobInfoBody), GetJobInfoBody_nullptr);

    // prepare 函数打桩
    stub.set(ADDR(HostIndex, PrepareForGenrateRfi), Stub_PrepareForGenerateRfi);
    stub.set(ADDR(HostIndex, CheckDcacheExist), Stub_CheckDcacheExist);
    // scanner start 函数打桩， getStatus函数打桩
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInst_Stub);

    AppProtect::SubJob subJob = GetSubJob();
    m_indexObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_indexObj.ExecuteSubJob(), Module::FAILED);
}

TEST_F(HostIndexTest, testPostJob)
{
    SetJobInfo();
    EXPECT_EQ(m_indexObj.PostJob(), Module::SUCCESS);
}

static int Stub_IdentifyRepos()
{
    return Module::FAILED;
}
TEST_F(HostIndexTest, ExecuteSubJobInner)
{
    SetJobInfo();
    stub.set(ADDR(HostIndex, IdentifyRepos), Stub_IdentifyRepos);
    EXPECT_EQ(m_indexObj.ExecuteSubJobInner(), Module::FAILED);
}

// void HostIndex::GeneratedCopyCtrlFileCb(void *usrData, string ctrlFile)

TEST_F(HostIndexTest, GeneratedCopyCtrlFileCb)
{
    string str1 = "111";
    string str2 = "222";
    RfiCbStruct cbParam;

    m_indexObj.GeneratedCopyCtrlFileCb(&str1, str2);
    m_indexObj.GeneratedHardLinkCtrlFileCb(&str1, str2);
    EXPECT_NO_THROW(m_indexObj.GenerateRfiCtrlFileCb(&str1, cbParam));

    cbParam.isFailed = false;
    cbParam.isComplete = true;
    EXPECT_NO_THROW(m_indexObj.GenerateRfiCtrlFileCb(&str1, cbParam));

    cbParam.isComplete = false;
    EXPECT_NO_THROW(m_indexObj.GenerateRfiCtrlFileCb(&str1, cbParam));
}

static int UnzipMetafileToCurPathAndRemoveSuc_Stub(void *obj)
{
    return Module::SUCCESS;
}

static int UnzipMetafileToCurPathAndRemoveFail_Stub(void *obj)
{
    return Module::FAILED;
}

TEST_F(HostIndexTest, PrepareForGenerateIncRfiInPath_Suc)
{
    std::string metaFilePath  = "/test/src";
    std::string prevMetaFilePath = "/src/test";
    promise<int> promiseObj;
    SetJobInfo();
    stub.set(ADDR(FilePlugin::HostIndex, UnzipMetafileToCurPathAndRemove), UnzipMetafileToCurPathAndRemoveSuc_Stub);
    EXPECT_NO_THROW(m_indexObj.PrepareForGenerateIncRfiInPath(metaFilePath, prevMetaFilePath, promiseObj));

    stub.reset(ADDR(FilePlugin::HostIndex, UnzipMetafileToCurPathAndRemove));
}

TEST_F(HostIndexTest, PrepareForGenerateIncRfiInPath_Fail)
{
    std::string metaFilePath  = "/test/src";
    std::string prevMetaFilePath = "/src/test";
    promise<int> promiseObj;
    SetJobInfo();

    stub.set(ADDR(FilePlugin::HostIndex, UnzipMetafileToCurPathAndRemove), UnzipMetafileToCurPathAndRemoveFail_Stub);
    EXPECT_NO_THROW(m_indexObj.PrepareForGenerateIncRfiInPath(metaFilePath, prevMetaFilePath, promiseObj));

    stub.reset(ADDR(FilePlugin::HostIndex, UnzipMetafileToCurPathAndRemove));
}