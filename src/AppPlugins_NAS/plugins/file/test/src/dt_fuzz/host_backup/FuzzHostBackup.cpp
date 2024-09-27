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
#include <stdio.h>
#include <iostream>
#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"
#include "secodeFuzz.h"
#include "HostBackup.h"
#include "Backup.h"
#include "ScanMgr.h"
#include "snapshot_provider/LvmSnapshotProvider.h"
using namespace std;
using namespace FilePlugin;
using namespace AppProtect;
class FuzzHostBackup : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUpResourceManagerFunc();
    void SetJobInfo();
    Stub stub;
    HostBackup m_hostBackupJobObj;
};
void FuzzHostBackup::TearDown() {}

void FuzzHostBackup::SetUpTestCase() {}

void FuzzHostBackup::TearDownTestCase() {}

static AppProtect::BackupJob BackupJobSetup()
{
    AppProtect::BackupJob backupJob;
    backupJob.requestId = "reqId123";
    backupJob.jobId = "jobId123";
    backupJob.jobParam.backupType = AppProtect::BackupJobType::FULL_BACKUP;
    backupJob.jobParam.dataLayout.extendInfo = "xxx";

    backupJob.protectObject.id = "protectObjectId123";
    backupJob.protectObject.type = "storage";
    backupJob.protectObject.subType = "Fileset";
    backupJob.protectObject.name = "Fileset_Test";
    backupJob.protectObject.extendInfo = "{}";

    AppProtect::ApplicationResource subObject1;
    subObject1.name = "/home";
    backupJob.protectSubObject.push_back(subObject1);
    AppProtect::ApplicationResource subObject2;
    subObject2.name = "/dev";
    backupJob.protectSubObject.push_back(subObject2);

    HostAddress hostadd;
    hostadd.ip="10.28.12.144";

    StorageRepository cacheRepo;
    cacheRepo.repositoryType = RepositoryDataType::CACHE_REPOSITORY;
    string path = "/tmp/MetaFS/local/cache";
    cacheRepo.path.push_back(path);
    cacheRepo.remotePath = "/tmp/MetaFS/remote/cache";
    cacheRepo.remoteHost.push_back(hostadd);
    cacheRepo.auth.authkey = "admin";
    cacheRepo.auth.authPwd = "Admin@123";
    cacheRepo.endpoint.ip = "10.28.12.144";
    cacheRepo.endpoint.port = 8088;

    StorageRepository dataRepo;
    dataRepo.repositoryType = RepositoryDataType::DATA_REPOSITORY;
    path = "/tmp/BackupFS/local";
    dataRepo.path.push_back(path);
    dataRepo.remotePath = "/tmp/BackupFS/remote";
    dataRepo.remoteHost.push_back(hostadd);
    dataRepo.auth.authkey = "admin";
    dataRepo.auth.authPwd = "Admin@123";
    dataRepo.endpoint.ip = "10.28.12.144";
    dataRepo.endpoint.port = 8088;

    StorageRepository metaRepo;
    metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
    path = "/tmp/MetaFS/local/meta";
    metaRepo.path.push_back(path);
    metaRepo.remotePath = "/tmp/MetaFS/remote/meta";
    metaRepo.remoteHost.push_back(hostadd);
    metaRepo.auth.authkey = "admin";
    metaRepo.auth.authPwd = "Admin@123";
    metaRepo.endpoint.ip = "10.28.12.144";
    metaRepo.endpoint.port = 8088;

    backupJob.repositories.push_back(cacheRepo);
    backupJob.repositories.push_back(dataRepo);
    backupJob.repositories.push_back(metaRepo);

    backupJob.extendInfo = "{\"consistent_backup\":\"false\","
                            "\"cross_file_system\":\"false\","
                            "\"backup_nfs\":\"false\","
                            "\"sparse_file_detection\":\"false\","
                            "\"ads_file_detection\":\"false\","
                            "\"backup_continue_with_files_backup_failed\":\"false\","
                            "\"small_file_aggregation\":\"false\","
                            "\"aggregation_file_size\":\"0\","
                            "\"aggregation_file_max_size\":\"0\"}";

    AppProtect::ResourceFilter fileFilter {};
    fileFilter.filterBy = "Name";
    fileFilter.type = "File";
    fileFilter.mode = "EXCLUDE";
    std::vector<string> fileFilterVal {"/home/xiaoding/Dir1/f1", "/home/xiaoding/Dir2/f2"};
    fileFilter.values = fileFilterVal;

    AppProtect::ResourceFilter dirFilter {};
    dirFilter.filterBy = "Name";
    dirFilter.type = "Dir";
    dirFilter.mode = "EXCLUDE";
    vector<string> dirFilterVal {"/home/xiaoding/Dir4", "/home/xiaoding/Dir5"};
    dirFilter.values = dirFilterVal;
    backupJob.jobParam.filters.push_back(fileFilter);
    backupJob.jobParam.filters.push_back(dirFilter);
    return backupJob;
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

static void Stub_AddNewJob_SUCCESS(ActionResult& returnValue, void *obj)
{
    returnValue.code = Module::SUCCESS;
}

static void StubFunction_VOID(void *ob)
{}
static bool StubFunction_TRUE(void *ob)
{
    return true;
}

static bool StubFunction_FALSE(void *ob)
{
    return false;
}

static unique_ptr<Scanner> CreateScanInstStub_Succ(void* obj, const ScanConfig& scanConfig) {
    ScanConfig emptInfo;
    unique_ptr<Scanner> scanInst = make_unique<ScanTest>(emptInfo);
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
    subJob.jobInfo = "{\"ControlFile\":\"/cache/backup-job/scan/ctrl\","
                    "\"SubTaskType\":1,"
                    "\"Ext\":\"abc\"}";
    return subJob;
}
namespace FS_Backup {
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
}

static unique_ptr<FS_Backup::Backup> CreateBackupInstStub_Succ(void* obj, BackupParams backupParams) {
    BackupParams emptyInfo;
    return make_unique<FS_Backup::BackupTest>(emptyInfo);
}
static int Stub_Config_getInt(void* obj, const string & sectionName, const string & keyName,bool logFlag)
{
    return 100;
}
static std::string Stub_Config_getString(void* obj) {
    return "";
}

static shared_ptr<JobCommonInfo> Stub_GetJobInfo_correct_value(void *obj)
{
    AppProtect::BackupJob backupJob = BackupJobSetup();
    auto backupJobInfoPtr = make_shared<JobCommonInfo>(make_shared<BackupJob>(backupJob));
    return backupJobInfoPtr;
}
static bool CheckFilePathAndGetSrcFileList_SUCCESS(void *obj, string srcDir, string dstDir, vector<string> &srcFileList)
{
    srcFileList.push_back("/tmp/MetaFS/local/cache/backup-job/scan/ctrl/control_test");
    return true;
}

static bool IsDirExist_SUCCESS(void *ob, const std::string& pathName)
{
    return true;
}
void FuzzHostBackup::SetJobInfo()
{
    AppProtect::BackupJob backupJob = BackupJobSetup();
    auto backupJobInfoPtr = make_shared<JobCommonInfo>(make_shared<BackupJob>(backupJob));
    m_hostBackupJobObj.SetJobInfo(backupJobInfoPtr);
}
void FuzzHostBackup::SetUpResourceManagerFunc()
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

void FuzzHostBackup::SetUp() {
    stub.set(ADDR(Module::ConfigReader, getInt), Stub_Config_getInt);
    stub.set(ADDR(Module::ConfigReader, getString), Stub_Config_getString);
}

TEST_F(FuzzHostBackup, PrerequisiteJob)
{
    SetJobInfo();
    m_hostBackupJobObj.GetBackupJobInfo();
    SetUpResourceManagerFunc();
    stub.set(ADDR(JobService, ReportJobDetails), StubFunction_VOID);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_SUCCESS);
    stub.set(PluginUtils::CreateDirectory, StubFunction_TRUE);
    
    DT_FUZZ_START(0, 10,(char*)"FuzzHostBackup_PrerequisiteJob" ,0)
    {
        char *remotePath = DT_SetGetString(&g_Element[0], 24, 1000 ,(char *)"/tmp/MetaFS/local/cache");
        m_hostBackupJobObj.m_backupJobPtr->repositories[0].remotePath = remotePath;
        EXPECT_EQ(m_hostBackupJobObj.PrerequisiteJob(), Module::SUCCESS);
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzHostBackup, GenerateSubJob)
{
    SetJobInfo();
    m_hostBackupJobObj.GetBackupJobInfo();
    stub.set(ADDR(HostBackup, IsBackupDevice), StubFunction_TRUE);
    stub.set(ADDR(HostBackup, CheckScanRedo), StubFunction_VOID);
    stub.set(ADDR(HostBackup, WriteScannSuccess), StubFunction_TRUE);
    stub.set(ADDR(JobService, ReportJobDetails), StubFunction_VOID);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_SUCCESS);
    stub.set(PluginUtils::CreateDirectory, StubFunction_TRUE);
    SetUpResourceManagerFunc();
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInstStub_Succ);
    stub.set(ADDR(HostBackup, CheckFilePathAndGetSrcFileList), CheckFilePathAndGetSrcFileList_SUCCESS);
    stub.set(PluginUtils::CopyFile, StubFunction_TRUE);
    DT_FUZZ_START(0, 10,(char*)"FuzzHostBackup_GenerateSubJob" ,0)
    {
        char *remotePath = DT_SetGetString(&g_Element[0], 24, 1000 ,(char *)"/tmp/MetaFS/local/cache");
        m_hostBackupJobObj.m_backupJobPtr->repositories[0].remotePath = remotePath;
        m_hostBackupJobObj.m_backupJobPtr->repositories[1].remotePath = remotePath;
        EXPECT_EQ(m_hostBackupJobObj.GenerateSubJob(), Module::SUCCESS);
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzHostBackup, ExecuteSubJob)
{
    SetJobInfo();
    m_hostBackupJobObj.GetBackupJobInfo();
    stub.set(ADDR(JobService, ReportJobDetails), StubFunction_VOID);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_SUCCESS);
    stub.set(PluginUtils::CreateDirectory, StubFunction_TRUE);
    SetUpResourceManagerFunc();
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);
    stub.set(JsonFileTool::WriteToFile<HostBackupCopy>, StubFunction_TRUE);
    AppProtect::SubJob subJob = GetSubJob();
    DT_FUZZ_START(0, 10,(char*)"FuzzHostBackup_ExecuteSubJob" ,0)
    {
        char *ext = DT_SetGetString(&g_Element[0], 4, 1000, (char *)"abc");
        std::string extendValue = ext;
        std::replace(extendValue.begin(), extendValue.end(), '\\', '/');
        subJob.jobInfo.append("{\"ControlFile\":\"/cache/backup-job/scan/ctrl\",")
              .append("\"SubTaskType\":2,").append("\"Ext\":\"").append(extendValue)
              .append("\"}");
        m_hostBackupJobObj.SetSubJob(std::make_shared<SubJob>(subJob));
        EXPECT_EQ(m_hostBackupJobObj.ExecuteSubJob(), Module::SUCCESS);
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzHostBackup, PostJob)
{
    SetJobInfo();
    m_hostBackupJobObj.GetBackupJobInfo();
    stub.set(ADDR(JobService, ReportJobDetails), StubFunction_VOID);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_SUCCESS);
    stub.set(PluginUtils::CreateDirectory, StubFunction_TRUE);
    SetUpResourceManagerFunc();
    stub.set(PluginUtils::IsDirExist, IsDirExist_SUCCESS);
    DT_FUZZ_START(0, 10,(char*)"FuzzHostBackup_PostJob" ,0)
    {
        char *remotePath = DT_SetGetString(&g_Element[0], 24, 1000 ,(char *)"/tmp/MetaFS/local/cache");
        m_hostBackupJobObj.m_backupJobPtr->repositories[0].remotePath = remotePath;
        m_hostBackupJobObj.m_backupJobPtr->repositories[1].remotePath = remotePath;
        EXPECT_EQ(m_hostBackupJobObj.PostJob(), Module::SUCCESS);
    }
    DT_FUZZ_END()
    return;
}
