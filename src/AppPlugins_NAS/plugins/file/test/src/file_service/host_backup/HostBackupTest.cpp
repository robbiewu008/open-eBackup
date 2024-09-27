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
#include "HostBackup.h"
#include "Backup.h"
#include "ScanMgr.h"
#include "snapshot_provider/LvmSnapshotProvider.h"
#include "snapshot_provider/LvmSnapshot.h"
#include "PluginUtilities.h"

using namespace std;
using ::testing::_;
using testing::AllOf;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using testing::ByMove;
using testing::DoAll;
using ::testing::Eq;
using ::testing::Field;
using ::testing::Ge;
using ::testing::Gt;
using testing::InitGoogleMock;
using ::testing::Invoke;
using ::testing::Ne;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::SetArgumentPointee;
using ::testing::Throw;
using namespace FilePlugin;

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
        std::unordered_set<FailedRecordItem, FailedRecordItemHash> ret;
        return ret;
    }
}
static unique_ptr<FS_Backup::Backup> CreateBackupInstStub_Succ(void* obj, BackupParams backupParams) {
    BackupParams emptyInfo;
    return make_unique<FS_Backup::BackupTest>(emptyInfo);
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
static unique_ptr<Scanner> CreateScanInstStub_Succ(void* obj, const ScanConfig& scanConfig) {
    ScanConfig emptInfo;
    unique_ptr<Scanner> scanInst = make_unique<ScanTest>(emptInfo);
    return scanInst;
}

bool Function_True()
{
    return true;
}
bool Function_False()
{
    return false;
}

class HostBackupTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUpScanFunc();
    void SetUpBackupFunc();
    void SetUpResourceManagerFunc();
    HostBackup m_hostBackupJobObj;
    Stub stub;
};

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
    backupJob.protectObject.name = "Fileset_Supengpeng_Test";
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
    vector<string> fileFilterVal {"/home/supengpeng/Dir1/f1", "/home/supengpeng/Dir2/f2"};
    fileFilter.values = fileFilterVal;

    AppProtect::ResourceFilter dirFilter {};
    dirFilter.filterBy = "Name";
    dirFilter.type = "Dir";
    dirFilter.mode = "EXCLUDE";
    vector<string> dirFilterVal {"/home/supengpeng/Dir4", "/home/supengpeng/Dir5"};
    dirFilter.values = dirFilterVal;
    backupJob.jobParam.filters.push_back(fileFilter);
    backupJob.jobParam.filters.push_back(dirFilter);
    return backupJob;
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

void HostBackupTest::SetUpTestCase()
{}

void HostBackupTest::TearDownTestCase()
{}

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

static bool StubFunction_ZERO(void *ob)
{
    return 0;
}

static SCANNER_STATUS Stub_ScanStat_SUCCESS(void *ob)
{
    return SCANNER_STATUS::SUCCESS;
}

static SCANNER_STATUS Stub_ScanStat_COMPLETED(void *ob)
{
    return SCANNER_STATUS::COMPLETED;
}

static bool IsDirExist_SUCCESS(void *ob, const std::string& pathName)
{
    return true;
}

static shared_ptr<JobCommonInfo> Stub_GetJobInfo_correct_value(void *obj)
{
    AppProtect::BackupJob backupJob = BackupJobSetup();
    auto backupJobInfoPtr = make_shared<JobCommonInfo>(make_shared<BackupJob>(backupJob));
    return backupJobInfoPtr;
}

static shared_ptr<JobCommonInfo> Stub_GetJobInfo_PERMANENT_INCREMENTAL_BACKUP(void *obj)
{
    AppProtect::BackupJob backupJob = BackupJobSetup();
    backupJob.jobParam.backupType = AppProtect::BackupJobType::PERMANENT_INCREMENTAL_BACKUP;
    auto backupJobInfoPtr = make_shared<JobCommonInfo>(make_shared<BackupJob>(backupJob));
    return backupJobInfoPtr;
}

static shared_ptr<JobCommonInfo> Stub_GetJobInfo_No_protectSubObject(void *obj)
{
    AppProtect::BackupJob backupJob = BackupJobSetup();
    backupJob.protectSubObject.clear();
    auto backupJobInfoPtr = make_shared<JobCommonInfo>(make_shared<BackupJob>(backupJob));
    return backupJobInfoPtr;
}

static shared_ptr<JobCommonInfo> Stub_GetJobInfo_Not_FullBackup_No_protectSubObject(void *obj)
{
    AppProtect::BackupJob backupJob = BackupJobSetup();
    backupJob.protectSubObject.clear();
    backupJob.jobParam.backupType = AppProtect::BackupJobType::PERMANENT_INCREMENTAL_BACKUP;
    auto backupJobInfoPtr = make_shared<JobCommonInfo>(make_shared<BackupJob>(backupJob));
    return backupJobInfoPtr;
}

static shared_ptr<JobCommonInfo> Stub_GetJobInfo_No_metaPath(void *obj)
{
    AppProtect::BackupJob backupJob = BackupJobSetup();
    backupJob.repositories[2].path.clear();
    auto backupJobInfoPtr = make_shared<JobCommonInfo>(make_shared<BackupJob>(backupJob));
    return backupJobInfoPtr;
}

static shared_ptr<JobCommonInfo> Stub_GetJobInfo_IsAggregate_True(void *obj)
{
    AppProtect::BackupJob backupJob = BackupJobSetup();
    backupJob.extendInfo = "{\"consistent_backup\":\"false\","
                            "\"cross_file_system\":\"false\","
                            "\"backup_nfs\":\"false\","
                            "\"sparse_file_detection\":\"false\","
                            "\"ads_file_detection\":\"false\","
                            "\"backup_continue_with_files_backup_failed\":\"false\","
                            "\"small_file_aggregation\":\"true\","
                            "\"aggregation_file_size\":\"0\","
                            "\"aggregation_file_max_size\":\"0\"}";
    auto backupJobInfoPtr = make_shared<JobCommonInfo>(make_shared<BackupJob>(backupJob));
    return backupJobInfoPtr;
}

static shared_ptr<JobCommonInfo> Stub_GetJobInfo_isCrossFileSystem(void *obj)
{
    AppProtect::BackupJob backupJob = BackupJobSetup();
    backupJob.extendInfo = "{\"consistent_backup\":\"false\","
                            "\"cross_file_system\":\"true\","
                            "\"backup_nfs\":\"false\","
                            "\"sparse_file_detection\":\"false\","
                            "\"ads_file_detection\":\"false\","
                            "\"backup_continue_with_files_backup_failed\":\"false\","
                            "\"small_file_aggregation\":\"false\","
                            "\"aggregation_file_size\":\"0\","
                            "\"aggregation_file_max_size\":\"0\"}";
    auto backupJobInfoPtr = make_shared<JobCommonInfo>(make_shared<BackupJob>(backupJob));
    return backupJobInfoPtr;
}

static bool CheckFilePathAndGetSrcFileList_SUCCESS(void *obj, string srcDir, string dstDir, vector<string> &srcFileList)
{
    srcFileList.push_back("/tmp/MetaFS/local/cache/backup-job/scan/ctrl/control_test");
    return true;
}

static bool ReadPrevBackupCopyInfoFromFile(void *obj, HostBackupCopy& t)
{
    t.m_backupFormat = "false";
    return true;
}

static void ReportCopyAdditionalInfo_STUB(void *obj, ActionResult& returnValue, const std::string& jobId, const Copy& copy)
{
    returnValue.code = Module::SUCCESS;
}

static void Stub_AddNewJob_SUCCESS(ActionResult& returnValue, void *obj)
{
    returnValue.code = Module::SUCCESS;
}

static SnapshotResult Stub_CreateSnapshot_SUCCESS(const std::string& filePath, bool isCrossVolume, void* obj)
{
    SnapshotResult snapshotResult {};
    snapshotResult.snapShotStatus = SNAPSHOT_STATUS::SUCCESS;
    snapshotResult.snapshotsMapper.emplace("/home", "/mnt/lvm_snapshots/jobId123/home");
    snapshotResult.snapshotVolumeMapper.emplace("/home", "volumeId123");
    snapshotResult.spacelessVgs.emplace("/home/device123");
    return snapshotResult;
}

static SnapshotResult Stub_CreateSnapshot_FAILED(const std::string& filePath, bool isCrossVolume, void* obj)
{
    SnapshotResult snapshotResult {};
    snapshotResult.snapShotStatus = SNAPSHOT_STATUS::FAILED;
    return snapshotResult;
}

static SnapshotResult Stub_CreateSnapshot_UNSUPPORTED(const std::string& filePath, bool isCrossVolume, void* obj)
{
    SnapshotResult snapshotResult {};
    snapshotResult.snapShotStatus = SNAPSHOT_STATUS::UNSUPPORTED;
    return snapshotResult;
}

static int Stub_RunShellCmd_SUCCESS(const severity_level& severity, const string& moduleName,
        const size_t& requestID, const string& cmd, const vector<string> params,
        vector<string>& cmdoutput, vector<string>& stderroutput)
{
    return 0;
}

static bool Stub_GetSubVolumes(void* obj,std::string path,
    std::vector<std::shared_ptr<FsDevice>>& outputEntryList)
{
    std::shared_ptr<FsDevice> fsDevice = std::make_shared<FsDevice>(1,"/home","ext4","/dev/mapper/lv");
    outputEntryList.push_back(fsDevice);
    return true;
}

void HostBackupTest::SetUp()
{
    INFOLOG("Enter HostBackupTest::SetUp");
    stub.set(sleep, StubFunction_VOID);
    stub.set(ADDR(JobService, ReportJobDetails), StubFunction_VOID);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_SUCCESS);
    stub.set(PluginUtils::CreateDirectory, StubFunction_TRUE);
    stub.set(ADDR(BasicJob, GetJobInfo), Stub_GetJobInfo_correct_value);
    SetUpResourceManagerFunc();
    INFOLOG("Exist HostBackupTest::SetUp");
}

void HostBackupTest::TearDown()
{}

void HostBackupTest::SetUpScanFunc()
{
    stub.set(ADDR(Scanner, Start), Stub_ScanStat_SUCCESS);
    stub.set(ADDR(Scanner, Enqueue), Stub_ScanStat_SUCCESS);
    stub.set(ADDR(Scanner, GetStatus), Stub_ScanStat_COMPLETED);
    stub.set(ADDR(Scanner, Destroy), Stub_ScanStat_SUCCESS);
    stub.set(ADDR(Scanner, Abort), Stub_ScanStat_SUCCESS);
}

void HostBackupTest::SetUpResourceManagerFunc()
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

/*
 * 用例名称:检查前置任务流程
 * 前置条件：无
 * check点：检查前置任务流程顺利运行
 */
TEST_F(HostBackupTest, PrerequisiteJob)
{
    INFOLOG("Enter HostBackupTest.PrerequisiteJob");
    // metaRepo为空
    stub.set(ADDR(BasicJob, GetJobInfo), Stub_GetJobInfo_No_metaPath);
    EXPECT_EQ(m_hostBackupJobObj.PrerequisiteJob(), Module::FAILED);
    // protectSubObject.size() == 0
    stub.set(ADDR(BasicJob, GetJobInfo), Stub_GetJobInfo_No_protectSubObject);
    EXPECT_EQ(m_hostBackupJobObj.PrerequisiteJob(), Module::FAILED);

    stub.set(ADDR(HostBackup, GetBackupJobInfo), Function_False);
    EXPECT_EQ(m_hostBackupJobObj.PrerequisiteJob(), Module::FAILED);

    TearDown();
    INFOLOG("Exist HostBackupTest.PrerequisiteJob");
}

TEST_F(HostBackupTest, GetAggCopyExtendInfo)
{
    INFOLOG("Enter HostBackupTest.GetAggCopyExtendInfo");
    m_hostBackupJobObj.GetBackupJobInfo();
    m_hostBackupJobObj.m_backupJobPtr->jobParam.backupType = AppProtect::BackupJobType::PERMANENT_INCREMENTAL_BACKUP;
    m_hostBackupJobObj.m_fileset.m_advParms.m_isAggregate = "true";
    m_hostBackupJobObj.m_fileset.m_advParms.m_maxSizeToAggregate ="128";
    m_hostBackupJobObj.m_fileset.m_advParms.m_maxSizeAfterAggregate ="512";
    m_hostBackupJobObj.m_backupJobPtr->copy.id = "123456";
    std::string extendInfo;
    bool result = m_hostBackupJobObj.GetAggCopyExtendInfo(extendInfo);
    EXPECT_EQ(result, true);
}

TEST_F(HostBackupTest, GetNativeCopyExtendInfo)
{
    INFOLOG("Enter HostBackupTest.GetNativeCopyExtendInfo");
    m_hostBackupJobObj.GetBackupJobInfo();
    m_hostBackupJobObj.m_backupJobPtr->jobParam.backupType = AppProtect::BackupJobType::PERMANENT_INCREMENTAL_BACKUP;
    m_hostBackupJobObj.m_fileset.m_advParms.m_isAggregate = "false";
    m_hostBackupJobObj.m_fileset.m_advParms.m_maxSizeToAggregate ="0";
    m_hostBackupJobObj.m_fileset.m_advParms.m_maxSizeAfterAggregate ="0";
    m_hostBackupJobObj.m_backupJobPtr->copy.id = "123456";
    std::string extendInfo;
    bool result = m_hostBackupJobObj.GetAggCopyExtendInfo(extendInfo);
    EXPECT_EQ(result, true);
}
/*
 * 用例名称:检查分解任务流程
 * 前置条件：无
 * check点：检查分解任务流程顺利运行
 */
TEST_F(HostBackupTest, GenerateSubJob)
{
    stub.set(ADDR(HostBackup, IsBackupDevice), StubFunction_TRUE);
    stub.set(ADDR(HostBackup, CheckScanRedo), StubFunction_FALSE);
    stub.set(ADDR(HostBackup, WriteScannSuccess), StubFunction_TRUE);
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInstStub_Succ);
    stub.set(ADDR(HostBackup, CheckFilePathAndGetSrcFileList), CheckFilePathAndGetSrcFileList_SUCCESS);
    stub.set(PluginUtils::CopyFile, StubFunction_TRUE);
    EXPECT_EQ(m_hostBackupJobObj.GenerateSubJob(), Module::SUCCESS);
    stub.set(ADDR(BasicJob, GetJobInfo), Stub_GetJobInfo_No_protectSubObject);
    EXPECT_EQ(m_hostBackupJobObj.GenerateSubJob(), Module::FAILED);  // 初始化失败
    INFOLOG("Exist HostBackupTest.GenerateSubJob");

    stub.set(ADDR(HostBackup, GetBackupJobInfo), Function_False);
    EXPECT_EQ(m_hostBackupJobObj.GenerateSubJob(), Module::FAILED);
}

/*
 * 用例名称:检查分解任务流程
 * 前置条件：无
 * check点：检查分解任务流程顺利运行
 */
TEST_F(HostBackupTest, GenerateSubJob_SubVolume)
{
    INFOLOG("Enter HostBackupTest.GenerateSubJob");
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInstStub_Succ);
    stub.set(ADDR(HostBackup, CheckFilePathAndGetSrcFileList), CheckFilePathAndGetSrcFileList_SUCCESS);
    stub.set(ADDR(HostBackup, CheckScanRedo), StubFunction_FALSE);
    stub.set(ADDR(HostBackup, WriteScannSuccess), StubFunction_TRUE);
    stub.set(PluginUtils::CopyFile, StubFunction_TRUE);
    m_hostBackupJobObj.m_subVolInfo = {{"1", 1}};
    EXPECT_EQ(m_hostBackupJobObj.GenerateSubJob(), Module::SUCCESS);

    stub.set(ADDR(BasicJob, GetJobInfo), Stub_GetJobInfo_No_protectSubObject);
    EXPECT_EQ(m_hostBackupJobObj.GenerateSubJob(), Module::FAILED);  // 初始化失败

    stub.set(ADDR(HostBackup, GetBackupJobInfo), Function_False);
    EXPECT_EQ(m_hostBackupJobObj.GenerateSubJob(), Module::FAILED);
}


/*
 * 用例名称:永久增量关闭聚合特性
 * 前置条件：无
 * check点：检查聚合特性打开情况下，永久增量关闭该特性
 */
TEST_F(HostBackupTest, CloseAggregateSwitchByBackupType)
{
    INFOLOG("Enter HostBackupTest.CloseAggregateSwitchByBackupType");
    m_hostBackupJobObj.GetBackupJobInfo();
    m_hostBackupJobObj.m_backupJobPtr->jobParam.backupType = AppProtect::BackupJobType::PERMANENT_INCREMENTAL_BACKUP;
    m_hostBackupJobObj.m_fileset.m_advParms.m_isAggregate = "true";
    m_hostBackupJobObj.CloseAggregateSwitchByBackupType();
    bool result = (m_hostBackupJobObj.m_fileset.m_advParms.m_isAggregate == "false");
    EXPECT_EQ(result, true);
    TearDown();
}

/*
 * 用例名称:判断当前设备是否在备份选项内
 * 前置条件：无
 * check点：备份的设备返回true，不备份的设备返回false
 */
TEST_F(HostBackupTest, IsBackupDevice)
{
    INFOLOG("Enter HostBackupTest.IsBackupDevice");
    shared_ptr<FsDevice> fsDevice = make_shared<FsDevice>();
    fsDevice->fsType = "ext2";
    EXPECT_EQ(m_hostBackupJobObj.IsBackupDevice(fsDevice), true);
    fsDevice->mountPoint = "/home";
    EXPECT_EQ(m_hostBackupJobObj.IsBackupDevice(fsDevice), true);
    fsDevice->mountPoint = "/home/supngepng";
    TearDown();
    INFOLOG("Exist HostBackupTest.IsBackupDevice");
}

/*
 * 用例名称: 获取当前文件集不备份的目录集合
 * 前置条件：无
 * check点：检查是否成功获取
 */
TEST_F(HostBackupTest, GetExcludeSubPath)
{
    INFOLOG("Enter HostBackupTest.GetExcludeSubPath");
    stub.set(ADDR(DeviceMount, GetSubVolumes), Stub_GetSubVolumes);
    m_hostBackupJobObj.GetBackupJobInfo();
    m_hostBackupJobObj.m_jobCtrlPhase = JOB_CTRL_PHASE_PREJOB;
    m_hostBackupJobObj.InitJobInfo();
    set<string> excludeSubPathList;
    m_hostBackupJobObj.GetExcludeSubPath("/home", excludeSubPathList);
    excludeSubPathList.clear();
    EXPECT_EQ(excludeSubPathList.size(), 0);
    m_hostBackupJobObj.GetExcludeSubPath("/dev", excludeSubPathList);
    EXPECT_NE(excludeSubPathList.size(), 0);
    m_hostBackupJobObj.GetExcludeSubPath("/sys", excludeSubPathList);
    EXPECT_NE(excludeSubPathList.size(), 0);
    // 跨文件系统
    stub.set(ADDR(BasicJob, GetJobInfo), Stub_GetJobInfo_isCrossFileSystem);
    (void)m_hostBackupJobObj.GetBackupJobInfo();
    (void)m_hostBackupJobObj.InitJobInfo();
    m_hostBackupJobObj.GetExcludeSubPath("/dev", excludeSubPathList);
    EXPECT_NE(excludeSubPathList.size(), 0);
    TearDown();
    INFOLOG("Exist HostBackupTest.GetExcludeSubPath");
}

/*
 * 用例名称: 填充扫描过滤参数
 * 前置条件：无
 * check点：检查参数是否填充正确
 */
TEST_F(HostBackupTest, FillScanFilterConfig)
{
    INFOLOG("Enter HostBackupTest.FillScanFilterConfig");
    m_hostBackupJobObj.GetBackupJobInfo();
    m_hostBackupJobObj.InitJobInfo();
    vector<string> fileFilterVal {"/home/supengpeng/Dir1/f1", "/home/supengpeng/Dir2/f2"};
    vector<string> dirFilterVal {"/home/supengpeng/Dir4", "/home/supengpeng/Dir5"};
    ScanConfig scanConfig {};
    m_hostBackupJobObj.FillScanFilterConfig(scanConfig);
    EXPECT_EQ(scanConfig.dFilter.dirList, dirFilterVal);
    EXPECT_EQ(scanConfig.fFilter.fileList, fileFilterVal);
    TearDown();
    INFOLOG("Exist HostBackupTest.FillScanFilterConfig");
}

/*
 * 用例名称: 根据mode参数填充scan filter type
 * 前置条件：无
 * check点：检查填充是否正确
 */
TEST_F(HostBackupTest, FillScanFilterType)
{
    INFOLOG("Enter HostBackupTest.FillScanFilterType");
    m_hostBackupJobObj.GetBackupJobInfo();
    FILTER_TYPE filterType;
    string mode = "INCLUDE";
    m_hostBackupJobObj.FillScanFilterType(mode, filterType);
    EXPECT_EQ(filterType, FILTER_TYPE::INCLUDE);

    mode = "EXCLUDE";
    m_hostBackupJobObj.FillScanFilterType(mode, filterType);
    EXPECT_EQ(filterType, FILTER_TYPE::EXCLUDE);

    mode = "";
    m_hostBackupJobObj.FillScanFilterType(mode, filterType);
    EXPECT_EQ(filterType, FILTER_TYPE::DISABLED);
    TearDown();
    INFOLOG("Exist HostBackupTest.GetExcludeSubPath");
}

/*
 * 用例名称: 检查备份类型
 * 前置条件：无
 * check点：返回成功或失败
 */
TEST_F(HostBackupTest, CheckBackupJobType)
{
    INFOLOG("Enter HostBackupTest.CheckBackupJobType");
    int ret = m_hostBackupJobObj.CheckBackupJobType();
    EXPECT_EQ(ret, Module::SUCCESS);  // 为全量备份，返回成功

    stub.set(ADDR(BasicJob, GetJobInfo), Stub_GetJobInfo_PERMANENT_INCREMENTAL_BACKUP);
    stub.set(PluginUtils::IsDirExist, IsDirExist_SUCCESS);
    stub.set(JsonFileTool::ReadFromFile<HostBackupCopy>, StubFunction_TRUE);
    ret = m_hostBackupJobObj.CheckBackupJobType();
    EXPECT_EQ(ret, Module::FAILED);  // 增量备份，与上次备份格式不一致，需要转为全量备份，返回失败

    stub.set(JsonFileTool::ReadFromFile<HostBackupCopy>, ReadPrevBackupCopyInfoFromFile);
    ret = m_hostBackupJobObj.CheckBackupJobType();
    EXPECT_EQ(ret, Module::FAILED);  // 增量备份，与上次备份格式一致,不需要转全量，返回成功

    stub.set(ADDR(BasicJob, GetJobInfo), Stub_GetJobInfo_Not_FullBackup_No_protectSubObject);
    ret = m_hostBackupJobObj.CheckBackupJobType();
    EXPECT_EQ(ret, Module::FAILED);  // 增量备份由于protectSubObject为空导致初始化失败
    INFOLOG("Exist HostBackupTest.CheckBackupJobType");

    stub.set(ADDR(HostBackup, GetBackupJobInfo), Function_False);
    ret = m_hostBackupJobObj.CheckBackupJobType();
    EXPECT_EQ(ret, Module::FAILED);
    stub.reset(ADDR(HostBackup, GetBackupJobInfo));
}

/*
 * 用例名称: 检查备份类型
 * 前置条件：无
 * check点：返回成功或失败
 */
TEST_F(HostBackupTest, CheckBackupJobTypeInner)
{
    stub.set(ADDR(HostBackup, IsFullBackup), Function_False);
    stub.set(ADDR(HostBackup, InitJobInfo), Function_True);
    stub.set(PluginUtils::IsDirExist, Function_False);
    EXPECT_EQ(m_hostBackupJobObj.CheckBackupJobTypeInner(), Module::FAILED);
    stub.reset(ADDR(HostBackup, IsFullBackup));
    stub.reset(ADDR(HostBackup, InitJobInfo));
    stub.reset(PluginUtils::IsDirExist);
}

/*
 * 用例名称: 检查是否需要增量备份转全量
 * 前置条件：无
 * check点：返回成功或失败
 */
TEST_F(HostBackupTest, NeedChangeIncToFull)
{
    stub.set(ADDR(HostBackup, GetPrevBackupCopyInfo), Function_False);
    EXPECT_EQ(m_hostBackupJobObj.NeedChangeIncToFull(), true);
    stub.set(ADDR(HostBackup, GetPrevBackupCopyInfo), Function_True);
    EXPECT_EQ(m_hostBackupJobObj.NeedChangeIncToFull(), false);
    m_hostBackupJobObj.m_prevBackupCopyInfo.m_backupFilter = "/home/test";
    EXPECT_EQ(m_hostBackupJobObj.NeedChangeIncToFull(), true);
    stub.reset(ADDR(HostBackup, GetPrevBackupCopyInfo));
}

/*
 * 用例名称: 执行后置任务
 * 前置条件：无
 * check点：返回成功
 */
TEST_F(HostBackupTest, PostJob)
{
    m_hostBackupJobObj.m_statisticsPath = "111";
    INFOLOG("Enter HostBackupTest.PostJob");
    stub.set(PluginUtils::IsDirExist, IsDirExist_SUCCESS);
    EXPECT_EQ(m_hostBackupJobObj.PostJob(), Module::SUCCESS);
    stub.set(ADDR(BasicJob, GetJobInfo), Stub_GetJobInfo_No_protectSubObject);
    EXPECT_EQ(m_hostBackupJobObj.PostJob(), Module::SUCCESS);
    EXPECT_EQ(m_hostBackupJobObj.InitJobInfo(), false);
    stub.set(ADDR(HostBackup, GetBackupJobInfo), Function_False);
    EXPECT_EQ(m_hostBackupJobObj.PostJob(), Module::FAILED);
}

/*
 * 用例名称: 执行子任务-DATACOPY_COPY_PHASE
 * 前置条件：已分发子任务
 * check点：返回成功
 */
TEST_F(HostBackupTest, ExecuteSubJob_DATACOPY_COPY_PHASE)
{
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);
    AppProtect::SubJob subJob = GetSubJob();
    stub.set(JsonFileTool::WriteToFile<HostBackupCopy>, StubFunction_TRUE);
    m_hostBackupJobObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostBackupJobObj.ExecuteSubJob(), Module::SUCCESS);  // 执行备份子任务成功

    stub.set(ADDR(HostBackup, GetBackupJobInfo), Function_False);
    EXPECT_EQ(m_hostBackupJobObj.ExecuteSubJob(), Module::FAILED);
}

/*
 * 用例名称: 执行子任务-DATACOPY_DELETE_PHASE
 * 前置条件：已分发子任务
 * check点：返回成功
 */
TEST_F(HostBackupTest, ExecuteSubJob_DATACOPY_DELETE_PHASE)
{
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);
    AppProtect::SubJob subJob = GetSubJob();
    stub.set(JsonFileTool::WriteToFile<HostBackupCopy>, StubFunction_TRUE);
    subJob.jobInfo = "{\"ControlFile\":\"/cache/backup-job/scan/ctrl\","
                    "\"SubTaskType\":2,"
                    "\"Ext\":\"abc\"}";
    m_hostBackupJobObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostBackupJobObj.ExecuteSubJob(), Module::SUCCESS);  // 执行备份子任务成功
}

/*
 * 用例名称: 执行子任务-DATACOPY_DELETE_PHASE，控制文件名含有delete_
 * 前置条件：已分发子任务
 * check点：返回成功
 */
TEST_F(HostBackupTest, ExecuteSubJob_DATACOPY_DELETE_PHASE_WITH_delete_)
{
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);
    AppProtect::SubJob subJob = GetSubJob();
    stub.set(JsonFileTool::WriteToFile<HostBackupCopy>, StubFunction_TRUE);
    subJob.jobInfo = "{\"ControlFile\":\"/cache/backup-job/scan/delete_ctrl\","
                    "\"SubTaskType\":2,"
                    "\"Ext\":\"abc\"}";
    m_hostBackupJobObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostBackupJobObj.ExecuteSubJob(), Module::SUCCESS);  // 直接返回成功
}

/*
 * 用例名称: 执行子任务-DATACOPY_HARDLINK_PHASE
 * 前置条件：已分发子任务
 * check点：返回成功
 */
TEST_F(HostBackupTest, ExecuteSubJob_DATACOPY_HARDLINK_PHASE)
{
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);
    AppProtect::SubJob subJob = GetSubJob();
    stub.set(JsonFileTool::WriteToFile<HostBackupCopy>, StubFunction_TRUE);
    subJob.jobInfo = "{\"ControlFile\":\"/cache/backup-job/scan/ctrl\","
                    "\"SubTaskType\":3,"
                    "\"Ext\":\"abc\"}";
    m_hostBackupJobObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostBackupJobObj.ExecuteSubJob(), Module::SUCCESS);  // 执行备份子任务成功
}

/*
 * 用例名称: 执行子任务-DATACOPY_DIRMTIME_PHASE
 * 前置条件：已分发子任务
 * check点：返回成功
 */
TEST_F(HostBackupTest, ExecuteSubJob_DATACOPY_DIRMTIME_PHASE)
{
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);
    AppProtect::SubJob subJob = GetSubJob();
    stub.set(JsonFileTool::WriteToFile<HostBackupCopy>, StubFunction_TRUE);
    subJob.jobInfo = "{\"ControlFile\":\"/cache/backup-job/scan/ctrl\","
                    "\"SubTaskType\":4,"
                    "\"Ext\":\"abc\"}";
    m_hostBackupJobObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostBackupJobObj.ExecuteSubJob(), Module::SUCCESS);  // 执行备份子任务成功
}

/*
 * 用例名称: 执行子任务-TEARDOWN_PHASE
 * 前置条件：已分发子任务
 * check点：返回成功
 */
TEST_F(HostBackupTest, ExecuteSubJob_TEARDOWN_PHASE)
{
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);
    AppProtect::SubJob subJob = GetSubJob();
    stub.set(JsonFileTool::WriteToFile<HostBackupCopy>, StubFunction_TRUE);
    subJob.jobInfo = "{\"ControlFile\":\"/cache/backup-job/scan/ctrl\","
                    "\"SubTaskType\":5,"
                    "\"Ext\":\"abc\"}";
    m_hostBackupJobObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostBackupJobObj.ExecuteSubJob(), Module::SUCCESS);  // 执行Teardown子任务成功
}

/*
 * 用例名称: 执行子任务-TEARDOWN_PHASE-聚合模式
 * 前置条件：已分发子任务
 * check点：返回成功
 */
TEST_F(HostBackupTest, ExecuteSubJob_TEARDOWN_PHASE_IsAggregate)
{
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);
    AppProtect::SubJob subJob = GetSubJob();
    stub.set(JsonFileTool::WriteToFile<HostBackupCopy>, StubFunction_TRUE);
    stub.set(ADDR(BasicJob, GetJobInfo), Stub_GetJobInfo_IsAggregate_True);
    stub.set(ADDR(JobService, ReportCopyAdditionalInfo), ReportCopyAdditionalInfo_STUB);
    stub.set(ADDR(HostCommonService, ReadDirCountForMtimeStats), StubFunction_ZERO);
    subJob.jobInfo = "{\"ControlFile\":\"/cache/backup-job/scan/ctrl\","
                    "\"SubTaskType\":5,"
                    "\"Ext\":\"abc\"}";
    m_hostBackupJobObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostBackupJobObj.ExecuteSubJob(), Module::SUCCESS);  // 执行Teardown子任务成功
}

static bool IsAggregate_Stub_True(void *obj)
{
    return true;
}

/*
 * 用例名称: ExecuteSubJob_TEARDOWN
 * 前置条件：
 * check点：检查ExecuteSubJob返回值
 */
TEST_F(HostBackupTest, ExecuteSubJob_TEARDOWN)
{
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);
    stub.set(PluginUtils::IsDirExist, StubFunction_TRUE);
    stub.set(PluginUtils::CreateDirectory, StubFunction_TRUE);
    stub.set(Module::runShellCmdWithOutput, Stub_RunShellCmd_SUCCESS);

    AppProtect::SubJob subJob = GetSubJob();
    stub.set(JsonFileTool::WriteToFile<HostBackupCopy>, StubFunction_TRUE);
    stub.set(ADDR(HostBackup, IsAggregate), IsAggregate_Stub_True);
    subJob.jobInfo = "{\"controlFile\":\"/cache/backup-job/scan/ctrl\","
                    "\"subTaskType\":6,"
                    "\"prefix\":\"abc\"}";
    m_hostBackupJobObj.SetSubJob(std::make_shared<SubJob>(subJob));
    m_hostBackupJobObj.m_scanMetaPath = "111";
    EXPECT_EQ(m_hostBackupJobObj.ExecuteSubJob(), Module::FAILED);  // 执行拷贝元数据子任务成功
    stub.reset(ADDR(HostBackup, IsAggregate));
}

static bool UpdateMainBackupStats_Stub_True(void *obj, BackupStatistic& mainStats)
{
    return true;
}

/*
 * 用例名称: ExecuteSubJob_TEARDOWN
 * 前置条件：
 * check点：检查ExecuteSubJob返回值
 */
TEST_F(HostBackupTest, ExecuteSubJob_TEARDOWN_UpdateMain)
{
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);
    stub.set(PluginUtils::IsDirExist, StubFunction_TRUE);
    stub.set(PluginUtils::CreateDirectory, StubFunction_TRUE);
    stub.set(Module::runShellCmdWithOutput, Stub_RunShellCmd_SUCCESS);

    AppProtect::SubJob subJob = GetSubJob();
    stub.set(JsonFileTool::WriteToFile<HostBackupCopy>, StubFunction_TRUE);
    stub.set(ADDR(HostBackup, IsAggregate), IsAggregate_Stub_True);
    stub.set(ADDR(HostBackup, UpdateMainBackupStats), UpdateMainBackupStats_Stub_True);

    subJob.jobInfo = "{\"controlFile\":\"/cache/backup-job/scan/ctrl\","
                    "\"subTaskType\":6,"
                    "\"prefix\":\"abc\"}";
    m_hostBackupJobObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostBackupJobObj.ExecuteSubJob(), Module::FAILED);
    stub.reset(ADDR(HostBackup, IsAggregate));
    stub.reset(ADDR(HostBackup, UpdateMainBackupStats));
}

/*
 * 用例名称: 执行子任务-COPYMETA_PHASE
 * 前置条件：已分发子任务
 * check点：返回成功
 */
TEST_F(HostBackupTest, ExecuteSubJob_COPYMETA)
{
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);
    stub.set(PluginUtils::IsDirExist, StubFunction_TRUE);
    stub.set(PluginUtils::CreateDirectory, StubFunction_TRUE);
    stub.set(Module::runShellCmdWithOutput, Stub_RunShellCmd_SUCCESS);

    AppProtect::SubJob subJob = GetSubJob();
    stub.set(JsonFileTool::WriteToFile<HostBackupCopy>, StubFunction_TRUE);
    subJob.jobInfo = "{\"controlFile\":\"/cache/backup-job/scan/ctrl\","
                    "\"subTaskType\":7,"
                    "\"prefix\":\"abc\"}";
    m_hostBackupJobObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostBackupJobObj.ExecuteSubJob(), Module::FAILED);
}

/*
 * 用例名称: 执行子任务-subJobId为空
 * 前置条件：已分发子任务
 * check点：返回失败
 */
TEST_F(HostBackupTest, ExecuteSubJob_subJobId_Empty)
{
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);
    AppProtect::SubJob subJob = GetSubJob();
    stub.set(JsonFileTool::WriteToFile<HostBackupCopy>, StubFunction_TRUE);
    subJob.subJobId = "";
    m_hostBackupJobObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostBackupJobObj.ExecuteSubJob(), Module::FAILED);  // 执行备份资源初始化失败
}

/*
 * 用例名称: 执行子任务-protectSubObject为空
 * 前置条件：已分发子任务
 * check点：返回失败
 */
TEST_F(HostBackupTest, ExecuteSubJob_protectSubObject_Empty)
{
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);
    AppProtect::SubJob subJob = GetSubJob();
    stub.set(JsonFileTool::WriteToFile<HostBackupCopy>, StubFunction_TRUE);
    stub.set(ADDR(BasicJob, GetJobInfo), Stub_GetJobInfo_No_protectSubObject);
    m_hostBackupJobObj.SetSubJob(std::make_shared<SubJob>(subJob));
    EXPECT_EQ(m_hostBackupJobObj.ExecuteSubJob(), Module::FAILED);  // protectSubObject为空，返回失败
}

/*
 * 用例名称: 创建快照
 * 前置条件：无
 * check点：检查创建快照是否成功
 */
TEST_F(HostBackupTest, CreateSnapshot)
{
    m_hostBackupJobObj.GetBackupJobInfo();
    m_hostBackupJobObj.InitJobInfo();
    m_hostBackupJobObj.m_fileset.m_advParms.m_isConsistent = "false";
    bool ret = m_hostBackupJobObj.CreateSnapshot();
    EXPECT_EQ(ret, true);

    m_hostBackupJobObj.m_fileset.m_advParms.m_isConsistent = "true";
    m_hostBackupJobObj.m_deviceMountPtr = make_shared<DeviceMount>();
    stub.set(ADDR(HostBackup, MountSnapshot), StubFunction_TRUE);
    typedef int32_t (*fptr) (LvmSnapshotProvider*, const string&, bool);
    fptr LvmSnapshotProvider_CreateSnapshot = (fptr)(&LvmSnapshotProvider::CreateSnapshot);
    stub.set(LvmSnapshotProvider_CreateSnapshot, Stub_CreateSnapshot_FAILED);
    ret = m_hostBackupJobObj.CreateSnapshot();
    EXPECT_EQ(ret, false);

    stub.set(LvmSnapshotProvider_CreateSnapshot, Stub_CreateSnapshot_UNSUPPORTED);
    ret = m_hostBackupJobObj.CreateSnapshot();
    EXPECT_EQ(ret, true);

    stub.set(LvmSnapshotProvider_CreateSnapshot, Stub_CreateSnapshot_SUCCESS);
    ret = m_hostBackupJobObj.CreateSnapshot();
    EXPECT_EQ(ret, true);

    stub.set(ADDR(HostBackup, MountSnapshot), StubFunction_FALSE);
    stub.set(LvmSnapshotProvider_CreateSnapshot, Stub_CreateSnapshot_SUCCESS);
    ret = m_hostBackupJobObj.CreateSnapshot();
    EXPECT_EQ(ret, false);
}

/*
 * 用例名称:检查前置任务流程
 * 前置条件：无
 * check点：检查前置任务流程顺利运行
 */
TEST_F(HostBackupTest, PrerequisiteJobInner)
{
    m_hostBackupJobObj.GetBackupJobInfo();
    stub.set(ADDR(HostBackup, SetupCacheFsForBackupJob), Function_True);
    stub.set(ADDR(HostBackup, InitJobInfo), Function_False);
    int ret = m_hostBackupJobObj.PrerequisiteJobInner();
    EXPECT_EQ(ret, Module::FAILED);
    stub.reset(ADDR(HostBackup, InitJobInfo));

    stub.set(ADDR(HostBackup, CreateSnapshot), Function_False);
    EXPECT_EQ(ret, Module::FAILED);
    stub.set(ADDR(HostBackup, CreateSnapshot), Function_True);

    stub.set(ADDR(HostBackup, SetupCacheFsForBackupJob), Function_False);
    EXPECT_EQ(ret, Module::FAILED);
    stub.set(ADDR(HostBackup, SetupCacheFsForBackupJob), Function_True);

    ret = m_hostBackupJobObj.PrerequisiteJobInner();
    EXPECT_EQ(ret, Module::SUCCESS);

    m_hostBackupJobObj.m_fileset.m_protectedPaths.clear();
    EXPECT_EQ(ret, Module::SUCCESS);
}

/*
 * 用例名称:初始化子任务管理资源
 * 前置条件：无
 * check点：初始化子任务管理资源是否成功
 */
TEST_F(HostBackupTest, InitSubBackupJobResources)
{
    m_hostBackupJobObj.m_subJobId = "iobdId123";
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager, InitResource), Function_False);
    int ret = m_hostBackupJobObj.InitSubBackupJobResources();
    EXPECT_EQ(ret, false);

    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager, InitResource), Function_True);
    ret = m_hostBackupJobObj.InitSubBackupJobResources();
    EXPECT_EQ(ret, true);
}

/*
 * 用例名称: 检查分解子任务流程
 * 前置条件：无
 * check点： 检查分解子任务流程顺利运行
 */
// TEST_F(HostBackupTest, GenerateSubJobInner)
// {
    // m_hostBackupJobObj.GetBackupJobInfo();
    // stub.set(ADDR(HostBackup, GetPrevBackupCopyInfo), Function_False);
    // int ret = m_hostBackupJobObj.GenerateSubJobInner();
    // EXPECT_EQ(ret, Module::FAILED);
    // stub.reset(ADDR(HostBackup, GetPrevBackupCopyInfo));

    // stub.set(ADDR(HostBackup, InitJobInfo), StubFunction_FALSE);
    // EXPECT_EQ(m_hostBackupJobObj.GenerateSubJob(), Module::FAILED);
    // stub.reset(ADDR(HostBackup, InitJobInfo));

    // stub.set(ADDR(HostBackup, GetPrevBackupCopyInfo), StubFunction_FALSE);
    // EXPECT_EQ(m_hostBackupJobObj.GenerateSubJob(), Module::FAILED);
    // stub.reset(ADDR(HostBackup, GetPrevBackupCopyInfo));

    // stub.set(ADDR(HostBackup, StartScannerForPrimaryVolume), StubFunction_FALSE);
    // EXPECT_EQ(m_hostBackupJobObj.GenerateSubJob(), Module::FAILED);
    // stub.set(ADDR(HostBackup, StartScannerForPrimaryVolume), StubFunction_TRUE);

    // stub.set(ADDR(HostBackup, StartScannerForSubVolume), StubFunction_FALSE);
    // EXPECT_EQ(m_hostBackupJobObj.GenerateSubJob(), Module::FAILED);
    // stub.set(ADDR(HostBackup, StartScannerForSubVolume), StubFunction_TRUE);

    // stub.set(ADDR(HostBackup, CreateBackupSubJobTask), StubFunction_FALSE);
    // EXPECT_EQ(m_hostBackupJobObj.GenerateSubJob(), Module::FAILED);
    // stub.set(ADDR(HostBackup, CreateBackupSubJobTask), StubFunction_TRUE);

    // EXPECT_EQ(m_hostBackupJobObj.GenerateSubJob(), Module::SUCCESS);
// }

/*
 * 用例名称: 上报扫描完成
 * 前置条件：无
 * check点：有无异常
 */
TEST_F(HostBackupTest, ReportScannerCompleteStatus)
{
    // HostScanStatistics scanStatistic;
    // scanStatistic.m_totFailedDirs = 1;
    EXPECT_NO_THROW(m_hostBackupJobObj.ReportScannerCompleteStatus());
}

/*
 * 用例名称: 上报备份完成
 * 前置条件：无
 * check点：是否成功
 */
TEST_F(HostBackupTest, ReportBackupCompletionStatus)
{
    stub.set(ADDR(HostBackup, UpdateMainBackupStats), Function_True);
    stub.set(Module::JsonHelper::StructToJsonString<BackupStatistic>, Function_True);
    stub.set(PluginUtils::WriteFile, StubFunction_TRUE);
    bool ret = m_hostBackupJobObj.ReportBackupCompletionStatus();
    EXPECT_EQ(ret, true);

    stub.set(Module::JsonHelper::StructToJsonString<BackupStatistic>, Function_False);
    ret = m_hostBackupJobObj.ReportBackupCompletionStatus();
    EXPECT_EQ(ret, false);

    stub.set(Module::JsonHelper::StructToJsonString<BackupStatistic>, Function_True);
    stub.set(PluginUtils::WriteFile, Function_False);
    ret = m_hostBackupJobObj.ReportBackupCompletionStatus();
    EXPECT_EQ(ret, false);

    stub.reset(ADDR(HostBackup, UpdateMainBackupStats));
}

/*
 * 用例名称: 填充聚合文件集
 * 前置条件：无
 * check点：是否填充
 */
TEST_F(HostBackupTest, FillAggregateFileSet)
{
    stub.set(PluginUtils::IsDirExist, Function_True);
    std::vector<std::string> aggregateFileSet;
    EXPECT_NO_THROW(m_hostBackupJobObj.FillAggregateFileSet(aggregateFileSet));
}

/*
 * 用例名称: 填充监控scanner的结果
 * 前置条件：无
 * check点：返回失败
 */
TEST_F(HostBackupTest, FillMonitorScannerVarDetails)
{
    auto obj_backup = std::make_unique<HostBackup>();
    SCANNER_TASK_STATUS scanTaskStatus = SCANNER_TASK_STATUS::INPROGRESS;
    SubJobStatus::type jobStatus = SubJobStatus::RUNNING;
    string jobLogLabel = "";
    obj_backup->m_scanStatus = SCANNER_STATUS::FAILED;
    obj_backup->FillMonitorScannerVarDetails(scanTaskStatus, jobStatus, jobLogLabel);
    EXPECT_EQ(jobStatus, SubJobStatus::FAILED);

    obj_backup->m_scanStatus = SCANNER_STATUS::ABORT_IN_PROGRESS;
    obj_backup->FillMonitorScannerVarDetails(scanTaskStatus, jobStatus, jobLogLabel);
    EXPECT_EQ(jobStatus, SubJobStatus::ABORTING);

    obj_backup->m_scanStatus = SCANNER_STATUS::ABORTED;
    obj_backup->FillMonitorScannerVarDetails(scanTaskStatus, jobStatus, jobLogLabel);
    EXPECT_EQ(jobStatus, SubJobStatus::ABORTED);

    obj_backup->m_scanStatus = SCANNER_STATUS::SECONDARY_SERVER_NOT_REACHABLE;
    obj_backup->FillMonitorScannerVarDetails(scanTaskStatus, jobStatus, jobLogLabel);
    EXPECT_EQ(jobStatus, SubJobStatus::FAILED);

    obj_backup->m_scanStatus = SCANNER_STATUS::PROTECTED_SERVER_NOT_REACHABLE;
    obj_backup->FillMonitorScannerVarDetails(scanTaskStatus, jobStatus, jobLogLabel);
    EXPECT_EQ(jobStatus, SubJobStatus::FAILED);
}

/*
 * 用例名称: 判断backup是否在运行中
 * 前置条件：无
 * check点：返回失败
 */
TEST_F(HostBackupTest, IsBackupStatusInprogress)
{
    auto obj_backup = std::make_unique<HostBackup>();
    SubJobStatus::type jobStatus = SubJobStatus::RUNNING;
    obj_backup->m_backupStatus = BackupPhaseStatus::COMPLETED;
    int ret = obj_backup->IsBackupStatusInprogress(jobStatus);
    EXPECT_EQ(ret, false);

    obj_backup->m_backupStatus = BackupPhaseStatus::FAILED;
    ret = obj_backup->IsBackupStatusInprogress(jobStatus);
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

static SnapshotResult CreateSnapshot_Stub_FAILED(void *obj, const std::string& filePath, bool isCrossVolume)
{
    SnapshotResult snapshotResult;
    snapshotResult.snapShotStatus = SNAPSHOT_STATUS::FAILED;
    return snapshotResult;
}

static SnapshotResult CreateSnapshot_Stub_SUCCESS(void *obj, const std::string& filePath, bool isCrossVolume)
{
    SnapshotResult snapshotResult;
    snapshotResult.snapShotStatus = SNAPSHOT_STATUS::SUCCESS;
    return snapshotResult;
}

TEST_F(HostBackupTest, CheckCreateSnapshot)
{
    m_hostBackupJobObj.m_filesetSnapInfoFilePath = "111";
    m_hostBackupJobObj.m_subVolSnapInfoFilePath = "111";

    typedef SnapshotResult (*fptr)(LvmSnapshotProvider*, const std::string&, bool);
    fptr LvmSnapshotProvider_CreateSnapshot = (fptr)((SnapshotResult(LvmSnapshotProvider::*)(const std::string&, bool))&LvmSnapshotProvider::CreateSnapshot);

    stub.set(ADDR(BasicJob, GetJobInfo), Stub_GetJobInfo_correct_value);
    m_hostBackupJobObj.GetBackupJobInfo();
    m_hostBackupJobObj.m_fileset.m_advParms.m_isConsistent = "true";
    m_hostBackupJobObj.m_fileset.m_protectedPaths = {"/home/1.txt"};

    stub.set(LvmSnapshotProvider_CreateSnapshot, CreateSnapshot_Stub_FAILED);
    EXPECT_EQ(m_hostBackupJobObj.CreateSnapshot(), false);
    stub.reset(LvmSnapshotProvider_CreateSnapshot);

    stub.set(LvmSnapshotProvider_CreateSnapshot, CreateSnapshot_Stub_SUCCESS);
    EXPECT_EQ(m_hostBackupJobObj.CreateSnapshot(), true);
    stub.reset(LvmSnapshotProvider_CreateSnapshot);

    stub.reset(ADDR(BasicJob, GetJobInfo));
}

TEST_F(HostBackupTest, FillScanConfigMapFunc)
{
    ScanConfig scanConfig {};
    std::string orginalMntPoint = "/home";
    EXPECT_NO_THROW(m_hostBackupJobObj.FillScanConfigMapFunc(scanConfig, orginalMntPoint));
}

TEST_F(HostBackupTest, StartScanner)
{
    ScanConfig scanConfig {};
    std::vector<std::string> paths = {"/root", "/home"};
    std::string prefix = "/lvm_snapshot";
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInstStub_Succ);
    EXPECT_EQ(m_hostBackupJobObj.StartScanner(scanConfig, paths, prefix), true);
}
