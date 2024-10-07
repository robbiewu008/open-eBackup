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
#include "gmock/gmock.h"
#include "mockcpp/mockcpp.hpp"
#include "log/Log.h"
#include "named_stub.h"
#include "ArchiveDownloadFile.h"
#include "host_archive/HostArchiveRestore.h"
#include "host_restore/HostRestore.h"
#include "Backup.h"
#include "ScanMgr.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using namespace FilePlugin;
using namespace std;
using namespace FS_Backup;

namespace {
const int SUCCESS = 0;
const int FAILED = -1;

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

struct IpPort {
    std::string ip;
    int port { -1 };
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(ip, ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(port, port)
    END_SERIAL_MEMEBER
};

struct ExtendArchiveInfo {
    std::vector<IpPort> serviceInfo;
    bool enableSSL {false};
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(enableSSL, enable_ssl)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(serviceInfo, service_info)
    END_SERIAL_MEMEBER
};

AppProtect::RestoreJob RestoreJobInfoSetUp()
{
    AppProtect::RestoreJob restoreJob;
    restoreJob.requestId = "123456789";
    restoreJob.jobId = "111111";
    restoreJob.targetEnv.id = "123";
    restoreJob.targetEnv.name = "123";
    restoreJob.targetEnv.port = 8088;
    restoreJob.targetEnv.auth.authkey = "admin";
    restoreJob.targetEnv.auth.authPwd = "Huawei@123456789";
    restoreJob.targetEnv.subType = "1";
    restoreJob.targetObject.id = "123";
    restoreJob.targetObject.name = "123";
    restoreJob.targetObject.parentId =" 4564654645564";
    restoreJob.targetObject.parentName = "test_filesystem";
    restoreJob.targetObject.subType ="5";
    restoreJob.targetEnv.endpoint = "127.0.0.1";
    restoreJob.targetObject.auth.authkey = "admin";
    restoreJob.targetObject.auth.authPwd = "Huawei@123456789";

    FileSetInfo fileSetInfo;
    fileSetInfo.filters = "";
    fileSetInfo.paths = "[{\"name\":\"/l30015744_restore\"}]";
    fileSetInfo.templateId = "";
    fileSetInfo.templateName = "";

    std::string filesetInfoStr;
    Module::JsonHelper::StructToJsonString(fileSetInfo, filesetInfoStr);
    restoreJob.targetEnv.extendInfo = filesetInfoStr;
    restoreJob.targetObject.extendInfo = filesetInfoStr;

    std::string path = "/MetaFS/cache";
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
    dataRepo.protocol = RepositoryProtocolType::type::S3;

    ExtendArchiveInfo archiveInfo;

    IpPort port;
    port.ip = "192.168.34.52";
    port.port = 57577;

    archiveInfo.serviceInfo.push_back(port);
    archiveInfo.enableSSL = false;

    std::string archiveInfoStr;
    Module::JsonHelper::StructToJsonString(archiveInfo, archiveInfoStr);

    dataRepo.extendInfo = archiveInfoStr;
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

    Copy copy;
    copy.repositories.push_back(cacheRepo);
    copy.repositories.push_back(dataRepo);
    copy.repositories.push_back(metaRepo);
    copy.id = "copyId";
    copy.protectObject.id = "CopyProtectObjectId";
    restoreJob.copies.push_back(copy);
    restoreJob.extendInfo = "{\"failed_script\":\"\",\"post_script\":\"\",\"pre_script\":\"\","
                           "\"restoreOption\":\"SKIP\"}";
    return restoreJob;
}
} // namespace


namespace Archive_Test {
    class BackupTest : public Backup {
    public:
        explicit BackupTest(const BackupParams& backupParams);
        BackupTest(std::string source, std::string destination);
        ~BackupTest() override;

        BackupRetCode Start() override;
        BackupRetCode Abort() override;
        BackupRetCode Destroy() override;
        BackupRetCode Enqueue(std::string contrlFile) override;
        BackupPhaseStatus GetStatus() override;
        BackupStats GetStats() override;
        std::unordered_set<FailedRecordItem, FailedRecordItemHash> GetFailedDetails() override;
    };


    BackupTest::BackupTest(const BackupParams& backupParams) :  Backup(backupParams) {};

    BackupTest::~BackupTest() {};

    BackupRetCode BackupTest::Start() {
        return BackupRetCode::SUCCESS;
    }
    BackupRetCode BackupTest::Abort() {
        return BackupRetCode::SUCCESS;
    }
    BackupRetCode BackupTest::Destroy() {
        return BackupRetCode::SUCCESS;
    }
    BackupRetCode BackupTest::Enqueue(std::string contrlFile) {
        return BackupRetCode::SUCCESS;
    }
    BackupPhaseStatus BackupTest::GetStatus() {
        return BackupPhaseStatus::COMPLETED;
    }
    BackupStats BackupTest::GetStats() {
        BackupStats backupStats;
        return backupStats;
    }
    std::unordered_set<FailedRecordItem, FailedRecordItemHash> BackupTest::GetFailedDetails() {
        std::unordered_set<FailedRecordItem, FailedRecordItemHash> ret;
        return ret;
    }

    class ScanTest : public Scanner {
    public:
        explicit ScanTest(const ScanConfig& scanConfig);
        ~ScanTest() override;

        SCANNER_STATUS Start() override;
        SCANNER_STATUS Abort() override;
        SCANNER_STATUS Destroy() override;
        SCANNER_STATUS Enqueue(const std::string& directory, const std::string& prefix = "", uint8_t filterFlag = 0) override;
        SCANNER_STATUS EnqueueV2(const std::string& directory) override;
        SCANNER_STATUS GetStatus() override;
        ScanStatistics GetStatistics() override;
    };

    ScanTest::ScanTest(const ScanConfig& scanConfig) : Scanner(scanConfig) {};

    ScanTest::~ScanTest() {};

    SCANNER_STATUS ScanTest::Start() {
        return SCANNER_STATUS::SUCCESS;
    }
    SCANNER_STATUS ScanTest::Abort() {
        return SCANNER_STATUS::SUCCESS;
    }
    SCANNER_STATUS ScanTest::Destroy() {
        return SCANNER_STATUS::SUCCESS;
    }
    SCANNER_STATUS ScanTest::Enqueue(const std::string& directory, const std::string& prefix, uint8_t filterFlag) {
        return SCANNER_STATUS::SUCCESS;
    }
    SCANNER_STATUS ScanTest::EnqueueV2(const std::string& directory) {
        return SCANNER_STATUS::SUCCESS;
    }
    SCANNER_STATUS ScanTest::GetStatus() {
        return SCANNER_STATUS::COMPLETED;
    }
    ScanStatistics ScanTest::GetStatistics() {
        ScanStatistics scanStats {};
        return scanStats;
    }
}// namespace archive_test

static unique_ptr<FS_Backup::Backup> CreateBackupInstStub_Succ(void* obj, BackupParams backupParams) {
    BackupParams emptyInfo;
    return std::make_unique<Archive_Test::BackupTest>(emptyInfo);
}

static unique_ptr<Scanner> CreateScanInstStub_Succ(void* obj, const ScanConfig& scanConfig) {
    ScanConfig emptInfo;
    unique_ptr<Scanner> scanInst = std::make_unique<Archive_Test::ScanTest>(emptInfo);
    return scanInst;
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

static void StubFunction_VOID(void *ob)
{}

static bool StubFunction_TRUE(void *ob)
{
    return true;
}

class HostArchiveRestoreTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::unique_ptr<HostArchiveRestore> m_jobPtr = std::make_unique<HostArchiveRestore>();
    void SetUpResourceManagerFunc();
    LLTSTUB::Stub stub;
};

void HostArchiveRestoreTest::SetUpResourceManagerFunc()
{
    stub.set(ADDR(ShareResourceManager, SetResourcePath), StubFunction_TRUE);

    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager, InitResource), StubFunction_TRUE);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        HostScanStatistics&))ADDR(ShareResourceManager, InitResource), StubFunction_TRUE);

    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager, QueryResource), StubFunction_TRUE);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        HostScanStatistics&))ADDR(ShareResourceManager, QueryResource), StubFunction_TRUE);

    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager, UpdateResource), StubFunction_TRUE);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        HostScanStatistics&))ADDR(ShareResourceManager, UpdateResource), StubFunction_TRUE);

    stub.set(ADDR(ShareResourceManager, DeleteResource), StubFunction_TRUE);
    stub.set(ADDR(ShareResourceManager, Wait), StubFunction_VOID);
    stub.set(ADDR(ShareResourceManager, Signal), StubFunction_VOID);
}

static void Stub_AddNewJob_SUCCESS(ActionResult& returnValue, void *obj)
{
    returnValue.code = Module::SUCCESS;
}

static int stubInitClient(void* obj, const std::vector<std::string>& ipList, int port, bool enableSSL)
{
    return SUCCESS;
}

static int StubClientDisconnect(void* obj)
{
    return SUCCESS;
}

static int StubClientDisconnectFailed(void* obj)
{
    return FAILED;
}

static int ArchiveStreamService_Init_SUCCESS(void* obj)
{
    return SUCCESS;
}

static int ArchiveStreamService_Connect_SUCCESS(void* obj)
{
    return SUCCESS;
}

static int ArchiveStreamService_PrepareRecovery_SUCCESS(void* obj)
{
    return SUCCESS;
}

static int ArchiveStreamService_Disconnect_SUCCESS(void* obj)
{
    return SUCCESS;
}

static bool stub_IsDirExist(const string& pathName)
{
    return true;
}

static bool stub_IsDirExist_Failed(const string& pathName)
{
    return false;
}

void HostArchiveRestoreTest::SetUp()
{
    AppProtect::RestoreJob restoreJob = RestoreJobInfoSetUp();
    auto jobInfoRestore = make_shared<JobCommonInfo>(make_shared<RestoreJob>(restoreJob));
    m_jobPtr->SetJobInfo(jobInfoRestore);
    stub.set(ADDR(JobService, ReportJobDetails), StubFunction_VOID);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_SUCCESS);
    MOCKER_CPP(PluginUtils::CreateDirectory)
            .stubs()
            .will(returnValue(true));
    stub.set(ADDR(ArchiveStreamService, Init), ArchiveStreamService_Init_SUCCESS);
    stub.set(ADDR(ArchiveStreamService, Connect), ArchiveStreamService_Connect_SUCCESS);
    stub.set(ADDR(ArchiveStreamService, PrepareRecovery), ArchiveStreamService_PrepareRecovery_SUCCESS);
    // stub.set(ADDR(ArchiveStreamService, Disconnect), ArchiveStreamService_Disconnect_SUCCESS);
    SetUpResourceManagerFunc();
    AppProtect::SubJob subJob = GetSubJob();
    m_jobPtr->SetSubJob(std::make_shared<SubJob>(subJob));
}

void HostArchiveRestoreTest::TearDown()
{
    GlobalMockObject::verify();
}

void HostArchiveRestoreTest::SetUpTestCase()
{}

void HostArchiveRestoreTest::TearDownTestCase()
{}

int TestReturnSuccess()
{
    return SUCCESS;
}

int TestReturnFailed()
{
    return FAILED;
}

static int Stub_PostJobInner_TRUE(void*)
{
    return SUCCESS;
}

static int Stub_EndRecover_TRUE(void*)
{
    return SUCCESS;
}

static int Stub_Disconnect_TRUE(void*)
{
    return SUCCESS;
}

static bool Stub_ReturnValueBoolNoParam_SUCCESS(void* obj)
{
    return true;
}

/*
 * 用例名称: 获取备份副本信息GetBackupCopyInfo
 * 前置条件：填充扩展信息
 * check点：扩展信息解析成功
 */
TEST_F(HostArchiveRestoreTest, GetBackupCopyInfoTest)
{
    std::string ext = "{ \"extendInfo\" :  { \"metaPathSuffix\" : \"eecc6061-142e-4236-a8bc-2b3eb420b950\", \"dataPathSuffix\" : \"eecc6061-142e-4236-a8bc-2b3eb420b950\"}}";

    bool ret = m_jobPtr->GetBackupCopyInfo(ext);
    INFOLOG("metapath: %s", m_jobPtr->m_aggCopyInfo.metaPathSuffix.c_str());
    EXPECT_EQ(ret, true);

    stub.set(Module::JsonHelper::JsonStringToJsonValue, TestReturnSuccess);
    stub.set(ADDR(HostArchiveRestore, IsFineRestore), TestReturnFailed);
    //stub.set(Module::JsonHelper::JsonValueToStruct, TestReturnSuccess);
    ret = m_jobPtr->GetBackupCopyInfo(ext);
    stub.reset(Module::JsonHelper::JsonStringToJsonValue);
    stub.reset(ADDR(HostArchiveRestore, IsFineRestore));
    //stub.reset(Module::JsonHelper::JsonValueToStruct);
}

/*
 * 用例名称: 归档恢复前置任务
 * 前置条件：
 * check点：前置任务成功
 */
TEST_F(HostArchiveRestoreTest, PrerequisiteJobTest)
{
    INFOLOG("enter HostArchiveRestoreTest.PrerequisiteJobTest");
    // 打桩 isDirExist
    stub.set(PluginUtils::IsDirExist, stub_IsDirExist);
    int ret = m_jobPtr->PrerequisiteJob();
    EXPECT_EQ(ret, SUCCESS);

    stub.set(PluginUtils::IsDirExist, stub_IsDirExist_Failed);
    ret = m_jobPtr->PrerequisiteJob();
    EXPECT_EQ(ret, FAILED);
}

/*
 * 用例名称: 归档恢复分解子任务
 * 前置条件：
 * check点：分解子任务成功/失败
 */
TEST_F(HostArchiveRestoreTest, GenerateSubJobTest)
{
    INFOLOG("enter HostArchiveRestoreTest.PrerequisiteJobTest");
    stub.set(PluginUtils::IsDirExist, stub_IsDirExist);
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInstStub_Succ);
    stub.set(ADDR(HostArchiveRestore, DownloadMetaFile), Stub_ReturnValueBoolNoParam_SUCCESS);
    stub.set(ADDR(HostArchiveRestore, UnzipMetafileZip), Stub_ReturnValueBoolNoParam_SUCCESS);
    m_jobPtr->m_isPrepareMetaFinish = true;
    int ret = m_jobPtr->GenerateSubJob();
    EXPECT_EQ(ret, SUCCESS);

    stub.set(PluginUtils::IsDirExist, stub_IsDirExist_Failed);
    ret = m_jobPtr->GenerateSubJob();
    EXPECT_EQ(ret, 0);
}

TEST_F(HostArchiveRestoreTest, ExecuteSubJobTest)
{
    INFOLOG("Enter HostArchiveRestoreTest.ExecuteSubJobTest");
    // 打桩 isDirExist
    stub.set(PluginUtils::IsDirExist, stub_IsDirExist);
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);
    int ret = m_jobPtr->ExecuteSubJob();
    EXPECT_EQ(ret, SUCCESS);

    stub.set(PluginUtils::IsDirExist, stub_IsDirExist_Failed);
    ret = m_jobPtr->ExecuteSubJob();
    EXPECT_EQ(ret, FAILED);
}

/*
 * 用例名称: 归档恢复后置任务
 * 前置条件：
 * check点：后置任务成功/失败
 */
TEST_F(HostArchiveRestoreTest, PostJobTest)
{
    stub.set(ADDR(ArchiveStreamService, EndRecover), StubClientDisconnect);
    // stub.set(ADDR(ArchiveStreamService, Disconnect), stubClientDisconnect);
    bool ret = m_jobPtr->PostJob();
    EXPECT_EQ(ret, SUCCESS);

    stub.set(ADDR(ArchiveStreamService, EndRecover), StubClientDisconnectFailed);
    ret = m_jobPtr->PostJob();
    EXPECT_EQ(ret, 0);

    m_jobPtr->m_backupStats.noOfDirFailed = 0;

    stub.set(ADDR(ArchiveStreamService, EndRecover), StubClientDisconnectFailed);
    ret = m_jobPtr->PostJob();
    EXPECT_EQ(ret, 0);
}

/*
 * 用例名称: ReportGenerateSubJob
 * 前置条件：
 * check点：生成子任务
 */
TEST_F(HostArchiveRestoreTest, ReportGenerateSubJob)
{
    m_jobPtr->m_scanStats.m_totFailedDirs = 1;

    m_jobPtr->m_jobState = {ArchiveJobState::ABORT};
    EXPECT_NO_THROW(m_jobPtr->ReportGenerateSubJob());

    m_jobPtr->m_jobState = {ArchiveJobState::EMPTY_COPY};
    EXPECT_NO_THROW(m_jobPtr->ReportGenerateSubJob());

    m_jobPtr->m_jobState = {ArchiveJobState::NONE};
    EXPECT_NO_THROW(m_jobPtr->ReportGenerateSubJob());
}

/*
 * 用例名称: ReportPostJob
 * 前置条件：
 * check点：
 */
TEST_F(HostArchiveRestoreTest, ReportPostJob)
{

    m_jobPtr->m_jobState = {ArchiveJobState::PARTIAL_SUCCESS};
    EXPECT_NO_THROW(m_jobPtr->ReportPostJob());

    m_jobPtr->m_jobState = {ArchiveJobState::NONE};
    EXPECT_NO_THROW(m_jobPtr->ReportPostJob());
}

bool Return_True()
{
    return true;
}

bool Return_False()
{
    return false;
}

/*
 * 用例名称: InitInfo
 * 前置条件：
 * check点：初始化
 */
// TEST_F(HostArchiveRestoreTest, InitInfo)
// {
//     stub.set(ADDR(HostArchiveRestore, InitJobInfo), Return_False);
//     EXPECT_FALSE(m_jobPtr->InitInfo());
//     stub.reset(ADDR(HostArchiveRestore, InitJobInfo));


//     stub.set(ADDR(HostArchiveRestore, InitJobInfo), Return_True);
//     stub.set(ADDR(HostArchiveRestore, InitRepoInfo), Return_False);

//     EXPECT_FALSE(m_jobPtr->InitInfo());
//     stub.reset(ADDR(HostArchiveRestore, InitJobInfo));
//     stub.reset(ADDR(HostArchiveRestore, InitRepoInfo));

//     stub.set(ADDR(HostArchiveRestore, InitJobInfo), Return_True);
//     stub.set(ADDR(HostArchiveRestore, InitRepoInfo), Return_True);
//     stub.set(ADDR(HostArchiveRestore, InitRestoreInfo), Return_False);
//     EXPECT_FALSE(m_jobPtr->InitInfo());
//     stub.reset(ADDR(HostArchiveRestore, InitJobInfo));
//     stub.reset(ADDR(HostArchiveRestore, InitRepoInfo));
//     stub.reset(ADDR(HostArchiveRestore, InitRestoreInfo));

//     stub.set(ADDR(HostArchiveRestore, InitJobInfo), Return_True);
//     stub.set(ADDR(HostArchiveRestore, InitRepoInfo), Return_True);
//     stub.set(ADDR(HostArchiveRestore, InitRestoreInfo), Return_True);
//     stub.set(ADDR(HostArchiveRestore, InitArchiveInfo), Return_False);
//     EXPECT_FALSE(m_jobPtr->InitInfo());
//     stub.reset(ADDR(HostArchiveRestore, InitJobInfo));
//     stub.reset(ADDR(HostArchiveRestore, InitRepoInfo));
//     stub.reset(ADDR(HostArchiveRestore, InitRestoreInfo));
//     stub.reset(ADDR(HostArchiveRestore, InitArchiveInfo));
// }

/*
 * 用例名称: GenerateSubJobInner
 * 前置条件：
 * check点：生成子任务
 */
TEST_F(HostArchiveRestoreTest, GenerateSubJobInner)
{
    stub.set(ADDR(HostArchiveRestore, InitCacheRepoDir), Return_False);
    int ret = m_jobPtr->GenerateSubJobInner();
    EXPECT_EQ(ret, FAILED);
    stub.reset(ADDR(HostArchiveRestore, InitCacheRepoDir));

    stub.set(ADDR(HostArchiveRestore, DownloadMetaFile), Return_False);
    ret = m_jobPtr->GenerateSubJobInner();
    EXPECT_EQ(ret, FAILED);
    stub.reset(ADDR(HostArchiveRestore, DownloadMetaFile));

    // stub.set(ADDR(HostArchiveRestore, StartScanner), Return_False);
    // m_jobPtr->m_isPrepareMetaFinish = true;
    // ret = m_jobPtr->GenerateSubJobInner();
    // EXPECT_EQ(ret, FAILED);
    // stub.reset(ADDR(HostArchiveRestore, StartScanner));
}

/*
 * 用例名称: SendJobReportForAliveness
 * 前置条件：
 * check点：SendJob
 */
TEST_F(HostArchiveRestoreTest, SendJobReportForAliveness)
{
    EXPECT_TRUE(m_jobPtr->SendJobReportForAliveness());
}

/*
 * 用例名称: UnzipMetafileZip
 * 前置条件：
 * check点：Unzip
 */
TEST_F(HostArchiveRestoreTest, UnzipMetafileZip)
{
    std::string input;
    std::string output;
    EXPECT_FALSE(m_jobPtr->UnzipMetafileZip(input, output));

    stub.set(PluginUtils::Remove, Return_False);
    GlobalMockObject::verify();
    MOCKER_CPP(PluginUtils::CreateDirectory)
            .stubs()
            .will(returnValue(false));
    EXPECT_FALSE(m_jobPtr->UnzipMetafileZip(input, output));
    stub.reset(PluginUtils::Remove);

    stub.set(PluginUtils::Remove, Return_True);
    GlobalMockObject::verify();
    MOCKER_CPP(PluginUtils::CreateDirectory)
            .stubs()
            .will(returnValue(true));
    stub.set(PluginUtils::IsFileExist, Return_True);
    EXPECT_FALSE(m_jobPtr->UnzipMetafileZip(input, output));
    stub.reset(PluginUtils::Remove);
    stub.reset(PluginUtils::IsFileExist);

    string name  = "w/";
    EXPECT_TRUE(m_jobPtr->IsDir(name));
}

TEST_F(HostArchiveRestoreTest, InitRestoreInfo)
{
    stub.set(ADDR(HostArchiveRestore, IsFineRestore), Return_True);
    m_jobPtr->InitJobInfo();
    EXPECT_TRUE(m_jobPtr->InitRestoreInfo());
}

TEST_F(HostArchiveRestoreTest, IsAbort)
{
    // stub.set(ADDR(HostArchiveRestore, IsFineRestore), Return_True);
    m_jobPtr->InitJobInfo();
    EXPECT_FALSE(m_jobPtr->IsAbort());
}

TEST_F(HostArchiveRestoreTest, IsBackupStatusInprogress)
{
    auto obj_backup = std::make_unique<HostArchiveRestore>();
    SubJobStatus::type jobStatus = SubJobStatus::RUNNING;
    obj_backup->m_backupStatus = BackupPhaseStatus::FAILED;
    int ret = obj_backup->IsBackupStatusInprogress(jobStatus);
    EXPECT_EQ(ret, false);

    obj_backup->m_backupStatus = BackupPhaseStatus::ABORTED;
    ret = obj_backup->IsBackupStatusInprogress(jobStatus);
    EXPECT_EQ(ret, false);

    obj_backup->m_backupStatus = BackupPhaseStatus::ABORT_INPROGRESS;
    ret = obj_backup->IsBackupStatusInprogress(jobStatus);
    EXPECT_EQ(ret, true);

    obj_backup->m_backupStatus = BackupPhaseStatus::INPROGRESS;
    ret = obj_backup->IsBackupStatusInprogress(jobStatus);
    EXPECT_EQ(ret, true);
}

/*
 * 用例名称: InitInfo
 * 前置条件：
 * check点：初始化信息
 */
TEST_F(HostArchiveRestoreTest, InitInfo)
{
    MOCKER_CPP(&HostArchiveRestore::InitJobInfo)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&HostArchiveRestore::InitRepoInfo)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&HostArchiveRestore::InitRestoreInfo)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&HostArchiveRestore::InitArchiveInfo)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    EXPECT_EQ(m_jobPtr->InitInfo(), false);
    EXPECT_EQ(m_jobPtr->InitInfo(), false);
    EXPECT_EQ(m_jobPtr->InitInfo(), false);
    EXPECT_EQ(m_jobPtr->InitInfo(), false);
    EXPECT_EQ(m_jobPtr->InitInfo(), true);
}

/*
 * 用例名称: PrerequisiteJobInner
 * 前置条件：
 * check点：前置任务
 */
TEST_F(HostArchiveRestoreTest, PrerequisiteJobInner)
{
    MOCKER_CPP(&HostArchiveRestore::InitInfo)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&HostArchiveRestore::InitArchiveClient)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    EXPECT_EQ(m_jobPtr->PrerequisiteJobInner(), -1);
    EXPECT_EQ(m_jobPtr->PrerequisiteJobInner(), -1);
}

/*
 * 用例名称: InitSubDir
 * 前置条件：
 * check点：初始化子目录
 */
TEST_F(HostArchiveRestoreTest, InitSubDir)
{
    GlobalMockObject::verify();
    MOCKER_CPP(PluginUtils::CreateDirectory)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(true));
    string dir = "/tmp";
    EXPECT_EQ(m_jobPtr->InitSubDir(dir), false);
    EXPECT_EQ(m_jobPtr->InitSubDir(dir), false);
    EXPECT_EQ(m_jobPtr->InitSubDir(dir), true);
}

/*
 * 用例名称: InitCacheRepoDir
 * 前置条件：
 * check点：初始化目录
 */
TEST_F(HostArchiveRestoreTest, InitCacheRepoDir)
{
    GlobalMockObject::verify();
    MOCKER_CPP(PluginUtils::CreateDirectory)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&HostArchiveRestore::InitMainResources)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&HostArchiveRestore::QueryMainScanResources)
            .stubs()
            .will(returnValue(true));

    // 第一次执行 CreateDirectory 返回失败，后面的 CreateDirectory、InitMainResources、InitMainResources都没有执行
    EXPECT_EQ(m_jobPtr->InitCacheRepoDir(), false);
    /* 第二次执行 CreateDirectory 返回成功，第3、4次都会返回成功，MOCKER_CPP默认以最后一次then保持不变，
       第一次执行InitMainResources, 返回失败，后面的 QueryMainScanResources 依然没有执行 */
    EXPECT_EQ(m_jobPtr->InitCacheRepoDir(), false);
    // QueryMainScanResources 第一次执行，因此只需要一次返回值
    EXPECT_EQ(m_jobPtr->InitCacheRepoDir(), true);
}
