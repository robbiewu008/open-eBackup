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
#include "log/Log.h"
#include "stub.h"
#include "ArchiveDownloadFile.h"
#include "host_archive/HostArchiveRestore.h"
#include "host_restore/HostRestore.h"
#include "secodeFuzz.h"
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
        return std::unordered_set<FailedRecordItem, FailedRecordItemHash> {};
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

class FuzzListApplicationArchiveRestore : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::unique_ptr<HostArchiveRestore> m_jobPtr = std::make_unique<HostArchiveRestore>();
    void SetUpResourceManagerFunc();
    Stub stub;
};

void FuzzListApplicationArchiveRestore::SetUpResourceManagerFunc()
{
    stub.set(ADDR(ShareResourceManager, SetResourcePath), StubFunction_TRUE);

    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,InitResource), StubFunction_TRUE);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        HostScanStatistics&))ADDR(ShareResourceManager,InitResource), StubFunction_TRUE);

    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,QueryResource), StubFunction_TRUE);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        HostScanStatistics&))ADDR(ShareResourceManager,QueryResource), StubFunction_TRUE);

    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,UpdateResource), StubFunction_TRUE);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        HostScanStatistics&))ADDR(ShareResourceManager,UpdateResource), StubFunction_TRUE);

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

static int stubClientDisconnect(void* obj)
{
    return SUCCESS;
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

static bool stub_InitArchiveClient()
{
    return true;
}

static int stub_PrerequisiteJobInner()
{
    return Module::SUCCESS;
}

static int stub_GenerateSubJobInner()
{
    return Module::SUCCESS;
}

static int stub_ExeSubJobInner()
{
    return Module::SUCCESS;
}
static int stub_PostJobInner()
{
    return Module::SUCCESS;
}

void FuzzListApplicationArchiveRestore::SetUp()
{
    AppProtect::RestoreJob restoreJob = RestoreJobInfoSetUp();
    auto jobInfoRestore = make_shared<JobCommonInfo>(make_shared<RestoreJob>(restoreJob));
    m_jobPtr->SetJobInfo(jobInfoRestore);
    stub.set(ADDR(JobService, ReportJobDetails), StubFunction_VOID);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_SUCCESS);
    stub.set(PluginUtils::CreateDirectory, StubFunction_TRUE);
    stub.set(ADDR(ArchiveStreamService, Init), ArchiveStreamService_Init_SUCCESS);
    stub.set(ADDR(ArchiveStreamService, Connect), ArchiveStreamService_Connect_SUCCESS);
    stub.set(ADDR(ArchiveStreamService, PrepareRecovery), ArchiveStreamService_PrepareRecovery_SUCCESS);
    // stub.set(ADDR(ArchiveStreamService, Disconnect), ArchiveStreamService_Disconnect_SUCCESS);
    SetUpResourceManagerFunc();
    AppProtect::SubJob subJob = GetSubJob();
    m_jobPtr->SetSubJob(std::make_shared<SubJob>(subJob));
}

void FuzzListApplicationArchiveRestore::TearDown()
{}

void FuzzListApplicationArchiveRestore::SetUpTestCase()
{}

void FuzzListApplicationArchiveRestore::TearDownTestCase()
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

TEST_F(FuzzListApplicationArchiveRestore, GetBackupCopyInfoTest)
{
    DT_FUZZ_START(0, 10, (char*)"FuzzArchiveRestoreGetBackupCopyInfoTest" ,0){
        std::string ext = "{ \"extendInfo\" :  { \"metaPathSuffix\" : \"eecc6061-142e-4236-a8bc-2b3eb420b950\", \"dataPathSuffix\" : \"eecc6061-142e-4236-a8bc-2b3eb420b950\"}}";

        bool ret = m_jobPtr->GetBackupCopyInfo(ext);
        INFOLOG("metapath: %s", m_jobPtr->m_aggCopyInfo.metaPathSuffix.c_str());
        EXPECT_EQ(ret, true);
    }
    DT_FUZZ_END()
    return;
}


TEST_F(FuzzListApplicationArchiveRestore, GenerateSubJobTest)
{
    stub.set(PluginUtils::IsDirExist, stub_IsDirExist);
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInstStub_Succ);
    stub.set(ADDR(HostArchiveRestore, DownloadMetaFile), Stub_ReturnValueBoolNoParam_SUCCESS);
    stub.set(ADDR(HostArchiveRestore, UnzipMetafileZip), Stub_ReturnValueBoolNoParam_SUCCESS);
    stub.set(ADDR(HostArchiveRestore, InitArchiveClient), stub_InitArchiveClient);
    stub.set(ADDR(HostArchiveRestore, GenerateSubJobInner), stub_GenerateSubJobInner);
    DT_FUZZ_START(0, 10, (char*)"FuzzArchiveRestore_PostJob_Null_Info" ,0){
        INFOLOG("enter HostArchiveRestoreTest.PrerequisiteJobTest");

        int ret = m_jobPtr->GenerateSubJob();
        EXPECT_EQ(ret, SUCCESS);
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzListApplicationArchiveRestore, ExecuteSubJobTest)
{
    // 打桩 isDirExist
    stub.set(PluginUtils::IsDirExist, stub_IsDirExist);
    stub.set(ADDR(HostArchiveRestore, ExecuteSubJobInner), stub_ExeSubJobInner);
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);
    DT_FUZZ_START(0, 10, (char*)"FuzzArchiveRestore_PostJob_Null_Info" ,0){
        INFOLOG("Enter HostArchiveRestoreTest.ExecuteSubJobTest");
        int ret = m_jobPtr->ExecuteSubJob();
        EXPECT_EQ(ret, SUCCESS);
    }
    DT_FUZZ_END()
    return;    
}

TEST_F(FuzzListApplicationArchiveRestore, PostJobTest)
{
    stub.set(ADDR(HostArchiveRestore, PostJobInner), stub_PostJobInner);
    stub.set(ADDR(ArchiveStreamService, EndRecover), stubClientDisconnect);
    DT_FUZZ_START(0, 10, (char*)"FuzzArchiveRestore_PostJob_Null_Info" ,0){
        // stub.set(ADDR(ArchiveStreamService, Disconnect), stubClientDisconnect);
        bool ret = m_jobPtr->PostJob();
        EXPECT_EQ(ret, SUCCESS);
    }
    DT_FUZZ_END()
    return;   
}
