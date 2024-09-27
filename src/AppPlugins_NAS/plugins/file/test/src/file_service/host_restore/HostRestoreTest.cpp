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
#include "host_restore/HostRestore.h"
#include "Backup.h"
#include "ScanMgr.h"

using namespace std;
using namespace FilePlugin;
namespace{
    struct FileSetInfo {
        string filters;
        string paths;
        string templateId;
        string templateName;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(filters, filters)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(paths, paths)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(templateId, templateId)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(templateId, templateId)
        END_SERIAL_MEMEBER
    };
}

namespace FS_Backup {
class testBackup : public Backup
{
public:
    explicit testBackup(const BackupParams& backupParams);
    testBackup(std::string source, std::string destination);
    ~testBackup() override;

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
    BackupRetCode Enqueue(std::string contrlFile) override;
    BackupPhaseStatus GetStatus() override;
    BackupStats GetStats() override;
    std::unordered_set<FailedRecordItem, FailedRecordItemHash> GetFailedDetails() override;
};

testBackup::testBackup(const BackupParams& backupParams) :  Backup(backupParams) {};
testBackup::~testBackup() {};

BackupRetCode testBackup::Start()
{
    return BackupRetCode::SUCCESS;
}
BackupRetCode testBackup::Abort()
{
    return BackupRetCode::SUCCESS;
}
BackupRetCode testBackup::Destroy()
{
    return BackupRetCode::SUCCESS;
}
BackupRetCode testBackup::Enqueue(std::string contrlFile)
{
    return BackupRetCode::SUCCESS;
}
BackupPhaseStatus testBackup::GetStatus()
{
    return BackupPhaseStatus::COMPLETED;
}
BackupStats testBackup::GetStats()
{
    BackupStats backupStats;
    return backupStats;
}
std::unordered_set<FailedRecordItem, FailedRecordItemHash> testBackup::GetFailedDetails() {
    std::unordered_set<FailedRecordItem, FailedRecordItemHash> ret;
    return ret;
}
}

class Scanner_test : public Scanner {
public:
    explicit Scanner_test(const ScanConfig& scanConfig);
    ~Scanner_test() {};

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

Scanner_test::Scanner_test(const ScanConfig& scanConfig) : Scanner(scanConfig) {}
SCANNER_STATUS Scanner_test::Start() {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test::Abort() {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test::Destroy() {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test::Enqueue(const std::string& directory, const std::string& prefix, uint8_t filterFlag) {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test::EnqueueV2(const std::string& directory) {
    return SCANNER_STATUS::SUCCESS;
}
SCANNER_STATUS Scanner_test::GetStatus() {
    return SCANNER_STATUS::COMPLETED;
}
ScanStatistics Scanner_test::GetStatistics() {
    ScanStatistics scanStatistics;
    return scanStatistics;
}

static void returnVoidStub(void* obj)
{
    return;
}

static void Stub_AddNewJob_SUCCESS(ActionResult& returnValue, void *obj)
{
    returnValue.code = Module::SUCCESS;
}

class HostRestoreTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    AppProtect::RestoreJob RestoreJobInfoSetUp();
    AppProtect::RestoreJob RestoreJobInfoForAggregateSetUp();
    static void ReportProgressSucc();
    HostRestore m_ins;
    Stub m_stub;
};

void HostRestoreTest::SetUp()
{
    m_stub.set(sleep, returnVoidStub);
    m_stub.set(ADDR(JobService, ReportJobDetails), HostRestoreTest::ReportProgressSucc);
    m_stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_SUCCESS);
}

void HostRestoreTest::TearDown()
{}

void HostRestoreTest::SetUpTestCase()
{
    std::cout << "Enter HostRestoreTest::SetUpTestCase" <<std::endl;
}

void HostRestoreTest::TearDownTestCase()
{
    std::cout << "Exit HostRestoreTest::TearDownTestCase" <<std::endl; 
}

void HostRestoreTest::ReportProgressSucc() {
    return;
}

static bool g_firstCall = true;
static bool GetFileListInDirectoryStub_Succ(std::string dir, std::vector<string>& fileList)
{
    if (g_firstCall) {
        for (int i = 0; i <= 12 ; i++) {
            string controlFile = "File/mtime_" + to_string(i) +  ".txt";
            fileList.push_back(controlFile);
        }
        string controlFile1 = "File/hardlink_1.txt";
        string controlFile2 = "File/delete_1.txt";
        string controlFile3 = "File/control_1.txt";
        fileList.push_back(controlFile1);
        fileList.push_back(controlFile2);
        fileList.push_back(controlFile3);
        g_firstCall = false;
    }
    return true;
}

static bool GetDirListInDirectoryStub_Succ(std::string dir, std::vector<string>& dirList)
{
    string dir1 = "xx_Volume";
    string dir2 = "yy_Volume";
    string dir3 = "zz_Volume";
    dirList.push_back(dir1);
    dirList.push_back(dir2);
    dirList.push_back(dir3);
    return true;
}

AppProtect::RestoreJob HostRestoreTest::RestoreJobInfoSetUp()
{
    AppProtect::RestoreJob restoreJob;
    restoreJob.requestId = "123456789";
    restoreJob.jobId = "111111";
    restoreJob.targetEnv.subType = "1";
    restoreJob.targetObject.id = "123";
    restoreJob.targetObject.name = "123";
    restoreJob.targetObject.subType ="5";

    FileSetInfo fileSetInfo;
    fileSetInfo.filters = "";
    fileSetInfo.paths = "[{\"name\":\"/l30015744_restore\"}]";
    fileSetInfo.templateId = "";
    fileSetInfo.templateName = "";

    string filesetInfoStr;
    Module::JsonHelper::StructToJsonString(fileSetInfo, filesetInfoStr);
    restoreJob.targetEnv.extendInfo = filesetInfoStr;

    string path = "/MetaFS/cache";
    StorageRepository cacheRepo;
    cacheRepo.repositoryType = RepositoryDataType::CACHE_REPOSITORY;
    cacheRepo.path.push_back(path);

    StorageRepository dataRepo;
    dataRepo.repositoryType = RepositoryDataType::DATA_REPOSITORY;
    dataRepo.path.push_back(dataRepo.remotePath);

    StorageRepository metaRepo;
    metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
    metaRepo.path.push_back(path);

    Copy copy;      
    copy.repositories.push_back(cacheRepo);
    copy.repositories.push_back(dataRepo);
    copy.repositories.push_back(metaRepo);
    restoreJob.copies.push_back(copy);
    restoreJob.extendInfo = "{\"failed_script\":\"\",\"post_script\":\"\",\"pre_script\":\"\","
                           "\"restoreOption\":\"SKIP\"}";
    restoreJob.jobParam.restoreType = AppProtect::RestoreJobType::type::FINE_GRAINED_RESTORE;

    return restoreJob;
}

AppProtect::RestoreJob HostRestoreTest::RestoreJobInfoForAggregateSetUp()
{
    AppProtect::RestoreJob restoreJob;
    restoreJob.requestId = "123456789";
    restoreJob.jobId = "111111";
    restoreJob.targetEnv.subType = "1";
    restoreJob.targetObject.id = "123";
    restoreJob.targetObject.name = "123";
    restoreJob.targetObject.subType ="5";

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


static void FunctionVoidSucc(void* obj) {
    return;
}

static bool FunctionBoolSucc(void* obj) {
    return true;
}

static bool FunctionBoolFailed(void* obj) {
    return false;
}

static int FunctionIntSucc(void* obj) {
    return Module::SUCCESS;
}

static int  FunctionIntFailed(void* obj) {
    return Module::FAILED;
}


static SCANNER_STATUS ScannerStub1_Succ() {
    return SCANNER_STATUS::SUCCESS;
}

static SCANNER_STATUS ScannerStub2_Succ() {
    return SCANNER_STATUS::COMPLETED;
}

static BackupRetCode BackupStub1_Succ() {
    return BackupRetCode::SUCCESS;
}

static BackupPhaseStatus BackupStub2_Succ() {
    return BackupPhaseStatus::COMPLETED;
}

static unique_ptr<Scanner> CreateScanInst_Stub (void* obj, ScanConfig scanConfig) {
    ScanConfig xx;
    return make_unique<Scanner_test>(xx);
}

static unique_ptr<FS_Backup::Backup> CreateBackupInstStub_Succ(void* obj, BackupParams backupParams) {
    BackupParams xx;
    return make_unique<FS_Backup::testBackup>(xx);
}

/*
 * 用例名称:检查前置任务流程
 * 前置条件：无
 * check点：检查前置任务流程顺利运行
 */
// TEST_F(HostRestoreTest, PrerequisiteJob)
// {
//     std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
//         std::make_shared<AppProtect::RestoreJob>(HostRestoreTest::RestoreJobInfoSetUp());
//     m_ins.m_jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);
//     int ret = m_ins.PrerequisiteJob();
//     EXPECT_EQ(ret, Module::SUCCESS);

//     Stub stub;
//     stub.set(ADDR(HostRestore, PrerequisiteJobInner), FunctionIntFailed);
//     ret = m_ins.PrerequisiteJob();
//     EXPECT_EQ(ret, Module::FAILED);

//     stub.reset(ADDR(HostRestore, PrerequisiteJobInner));
// }

/*
 * 用例名称:检查分解任务流程
 * 前置条件：无
 * check点：检查分解任务流程顺利运行
 */
TEST_F(HostRestoreTest, GenerateSubJob)
{
    std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
        std::make_shared<AppProtect::RestoreJob>(HostRestoreTest::RestoreJobInfoSetUp());
    m_ins.m_jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);
    g_firstCall = true;
    Stub stub;
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInst_Stub);
    stub.set(Module::JsonHelper::JsonStringToJsonValue, FunctionBoolSucc);
    stub.set(PluginUtils::CreateDirectory, FunctionBoolSucc);
    stub.set(PluginUtils::GetDirListInDirectory, GetDirListInDirectoryStub_Succ);
    stub.set(ADDR(HostRestore, CheckScanRedo), FunctionVoidSucc);
    stub.set(ADDR(HostRestore, CreateRestoreSubJob), FunctionBoolSucc);
    
    stub.set(PluginUtils::RemoveFile, FunctionBoolSucc);
    stub.set(PluginUtils::CopyFile, FunctionBoolSucc);
    stub.set(Module::runShellCmdWithOutput, FunctionIntSucc);
    stub.set(PluginUtils::GetFileListInDirectory, GetFileListInDirectoryStub_Succ);
    stub.set(ADDR(JobService, AddNewJob), FunctionVoidSucc);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,InitResource), FunctionBoolSucc);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        HostTaskInfo&))ADDR(ShareResourceManager,InitResource), FunctionBoolSucc);
    int ret = m_ins.GenerateSubJob();
    EXPECT_EQ(ret, Module::FAILED);

    stub.reset(PluginUtils::CreateDirectory);
    stub.reset(PluginUtils::RemoveFile);
    stub.reset(PluginUtils::CopyFile);
    stub.reset(PluginUtils::GetFileListInDirectory);
    stub.reset(PluginUtils::GetDirListInDirectory);
    stub.reset(ADDR(JobService, AddNewJob));
    stub.reset((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,InitResource));
}

/*
 * 用例名称:检查执行子任务流程
 * 前置条件：无
 * check点：检查执行子任务流程顺利运行
 */
TEST_F(HostRestoreTest, ExecuteSubJob)
{
    std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
        std::make_shared<AppProtect::RestoreJob>(HostRestoreTest::RestoreJobInfoSetUp());
    m_ins.m_jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);

    SubJobInfo subJobInfo;
    subJobInfo.controlFileName = "test_control.txt";
    subJobInfo.extendInfo = "";
    subJobInfo.subJobType = SUBJOB_TYPE_DATACOPY_COPY_PHASE;
    string subJobInfoStr;
    Module::JsonHelper::StructToJsonString(subJobInfo, subJobInfoStr);
    SubJob subJob;
    subJob.jobInfo = subJobInfoStr;
    m_ins.SetSubJob(std::make_shared<SubJob>(subJob));

    Stub stub;
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,InitResource), FunctionBoolSucc);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,QueryResource), FunctionBoolSucc);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,UpdateResource), FunctionBoolSucc);
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);

    int ret = m_ins.ExecuteSubJob();
    EXPECT_EQ(ret, Module::SUCCESS);

    stub.reset((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,InitResource));
    stub.reset((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,QueryResource));
    stub.reset((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,UpdateResource));
    stub.reset(ADDR(FS_Backup::BackupMgr, CreateBackupInst));
}

/*
 * 用例名称:检查后置任务流程
 * 前置条件：无
 * check点：检查后置任务流程顺利运行
 */
TEST_F(HostRestoreTest, PostJob)
{
    std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
        std::make_shared<AppProtect::RestoreJob>(HostRestoreTest::RestoreJobInfoSetUp());
    m_ins.m_jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);

    Stub stub;
    stub.set( PluginUtils::Remove, FunctionBoolSucc);
    int ret = m_ins.PostJob();
    EXPECT_EQ(ret, Module::SUCCESS);

    stub.set( PluginUtils::Remove, FunctionBoolFailed);
    ret = m_ins.PostJob();
    EXPECT_EQ(ret, Module::SUCCESS);

    stub.reset( PluginUtils::Remove);
}

/*
 * 用例名称:检查聚合多副本分解任务流程
 * 前置条件：无
 * check点：检查聚合多副本分解任务流程顺利运行
 */
TEST_F(HostRestoreTest, GenerateSubJobForAggregate)
{
    std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
        std::make_shared<AppProtect::RestoreJob>(HostRestoreTest::RestoreJobInfoForAggregateSetUp());
    m_ins.m_jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);
    g_firstCall = true;

    Stub stub;
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInst_Stub);
    stub.set(ADDR(HostRestore, CheckScanRedo), FunctionVoidSucc);
    stub.set(ADDR(HostRestore, CreateRestoreSubJob), FunctionBoolSucc);
    stub.set(PluginUtils::CreateDirectory, FunctionBoolSucc);
    stub.set(PluginUtils::RemoveFile, FunctionBoolSucc);
    stub.set(PluginUtils::CopyFile, FunctionBoolSucc);
    stub.set(Module::runShellCmdWithOutput, FunctionIntSucc);
    stub.set(PluginUtils::GetDirListInDirectory, GetDirListInDirectoryStub_Succ);
    stub.set(PluginUtils::GetFileListInDirectory, GetFileListInDirectoryStub_Succ);
    stub.set(ADDR(JobService, AddNewJob), FunctionVoidSucc);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,InitResource), FunctionBoolSucc);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        HostTaskInfo&))ADDR(ShareResourceManager,InitResource), FunctionBoolSucc);
    int ret = m_ins.GenerateSubJob();
    EXPECT_EQ(ret, Module::FAILED);

    stub.reset(PluginUtils::CreateDirectory);
    stub.reset(PluginUtils::RemoveFile);
    stub.reset(PluginUtils::CopyFile);
    stub.reset(PluginUtils::GetFileListInDirectory);
    stub.reset(PluginUtils::GetDirListInDirectory);
    stub.reset(ADDR(JobService, AddNewJob));
    stub.reset((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,InitResource));
}

/*
 * 用例名称:检查聚合细粒度多副本分解任务流程
 * 前置条件：无
 * check点：检查聚合细粒度多副本分解任务流程顺利运行
 */
TEST_F(HostRestoreTest, GenerateSubJobForAggregateFineGrain)
{
    AppProtect::RestoreJob jobInfo = HostRestoreTest::RestoreJobInfoForAggregateSetUp();
    jobInfo.jobParam.restoreType = AppProtect::RestoreJobType::type::FINE_GRAINED_RESTORE;
    std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
        std::make_shared<AppProtect::RestoreJob>(jobInfo);
    m_ins.m_jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);
    g_firstCall = true;

    Stub stub;
    stub.set(ADDR(HostRestore, CheckScanRedo), FunctionVoidSucc);
    stub.set(ADDR(HostRestore, CreateRestoreSubJob), FunctionBoolSucc);
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInst_Stub);
    stub.set(PluginUtils::CreateDirectory, FunctionBoolSucc);
    stub.set(PluginUtils::RemoveFile, FunctionBoolSucc);
    stub.set(PluginUtils::CopyFile, FunctionBoolSucc);
    stub.set(Module::runShellCmdWithOutput, FunctionIntSucc);
    stub.set(PluginUtils::GetDirListInDirectory, GetDirListInDirectoryStub_Succ);
    stub.set(PluginUtils::GetFileListInDirectory, GetFileListInDirectoryStub_Succ);
    stub.set(ADDR(JobService, AddNewJob), FunctionVoidSucc);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,InitResource), FunctionBoolSucc);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        HostTaskInfo&))ADDR(ShareResourceManager,InitResource), FunctionBoolSucc);
    int ret = m_ins.GenerateSubJob();
    EXPECT_EQ(ret, Module::FAILED);

    jobInfo = HostRestoreTest::RestoreJobInfoForAggregateSetUp();
    jobInfo.jobParam.restoreType = AppProtect::RestoreJobType::type::FINE_GRAINED_RESTORE;
    jobInfo.copies.pop_back();
    jobInfo.copies.pop_back();

    restoreJobInfoPtr = std::make_shared<AppProtect::RestoreJob>(jobInfo);
    m_ins.m_jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);
    ret = m_ins.GenerateSubJob();
    EXPECT_EQ(ret, Module::FAILED);

    stub.reset(PluginUtils::CreateDirectory);
    stub.reset(PluginUtils::RemoveFile);
    stub.reset(PluginUtils::CopyFile);
    stub.reset(PluginUtils::GetFileListInDirectory);
    stub.reset(PluginUtils::GetDirListInDirectory);
    stub.reset(ADDR(JobService, AddNewJob));
    stub.reset((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,InitResource));
}

/*
 * 用例名称:检查聚合执行子任务流程
 * 前置条件：无
 * check点：检查执行子任务流程顺利运行
 */
TEST_F(HostRestoreTest, ExecuteSubJobForAggregate)
{
    std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
        std::make_shared<AppProtect::RestoreJob>(HostRestoreTest::RestoreJobInfoForAggregateSetUp());
    m_ins.m_jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);

    SubJobInfo subJobInfo;
    subJobInfo.controlFileName = "test_control.txt";
    subJobInfo.subJobType = SUBJOB_TYPE_DATACOPY_COPY_PHASE;
    subJobInfo.copyOrder = 0;
    string subJobInfoStr;
    Module::JsonHelper::StructToJsonString(subJobInfo, subJobInfoStr);
    SubJob subJob;
    subJob.jobInfo = subJobInfoStr;
    m_ins.SetSubJob(std::make_shared<SubJob>(subJob));
    Stub stub;
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,InitResource), FunctionBoolSucc);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,QueryResource), FunctionBoolSucc);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,UpdateResource), FunctionBoolSucc);
    stub.set(ADDR(FS_Backup::BackupMgr, CreateBackupInst), CreateBackupInstStub_Succ);

    int ret = m_ins.ExecuteSubJob();
    EXPECT_EQ(ret, Module::SUCCESS);

    stub.reset((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,InitResource));
    stub.reset((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,QueryResource));
    stub.reset((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,UpdateResource));
    stub.reset(ADDR(FS_Backup::BackupMgr, CreateBackupInst));
}

/*
 * 用例名称:检查最后一次上报的任务流程
 * 前置条件：无
 * check点：检查后置任务流程顺利运行
 */
TEST_F(HostRestoreTest, ExecuteSubJobForFinalReport)
{
    std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
        std::make_shared<AppProtect::RestoreJob>(HostRestoreTest::RestoreJobInfoForAggregateSetUp());
    m_ins.m_jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);

    SubJobInfo subJobInfo;
    subJobInfo.controlFileName = "test_control.txt";
    subJobInfo.extendInfo = "";
    subJobInfo.subJobType = SUBJOB_TYPE_TEARDOWN_PHASE;
    string subJobInfoStr;
    Module::JsonHelper::StructToJsonString(subJobInfo, subJobInfoStr);
    SubJob subJob;
    subJob.jobInfo = subJobInfoStr;
    m_ins.SetSubJob(std::make_shared<SubJob>(subJob));
    Stub stub;
    stub.set(PluginUtils::GetFileListInDirectory, GetFileListInDirectoryStub_Succ);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,QueryResource), FunctionBoolSucc);
    int ret = m_ins.ExecuteSubJob();
    EXPECT_EQ(ret, Module::SUCCESS);
    stub.reset(PluginUtils::GetFileListInDirectory);
    stub.reset((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,QueryResource));
}

/*
 * 用例名称:检查执行子任务第一次上报的任务流程
 * 前置条件：无
 * check点：检查后置任务流程顺利运行
 */
// TEST_F(HostRestoreTest, ExecuteSubJobForFirstReport)
// {
//     std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
//         std::make_shared<AppProtect::RestoreJob>(HostRestoreTest::RestoreJobInfoForAggregateSetUp());
//     m_ins.m_jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);

//     SubJobInfo subJobInfo;
//     subJobInfo.controlFileName = "test_control.txt";
//     subJobInfo.extendInfo = "";
//     subJobInfo.subJobType = SUBJOB_TYPE_SETUP_PHASE;
//     string subJobInfoStr;
//     Module::JsonHelper::StructToJsonString(subJobInfo, subJobInfoStr);
//     SubJob subJob;
//     subJob.jobInfo = subJobInfoStr;
//     m_ins.SetSubJob(std::make_shared<SubJob>(subJob));
//     Stub stub;
//     stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
//         BackupStatistic&))ADDR(ShareResourceManager,QueryResource), FunctionBoolSucc);
//     int ret = m_ins.ExecuteSubJob();
//     EXPECT_EQ(ret, Module::SUCCESS);
//     stub.reset((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
//         BackupStatistic&))ADDR(ShareResourceManager,QueryResource));
// }

int Funtion_Module_Failed(){
    return  Module::FAILED;
}
/*
 * 用例名称:检查分解任务异常流程
 * 前置条件：无
 * check点：检查后置任务流程顺利运行
 */
TEST_F(HostRestoreTest, GenerateSubJobInner)
{
    Stub stub;
    stub.set(ADDR(HostRestore, InitGenerateJobInfo), Funtion_Module_Failed);
    RestoreJob restoreJob;
    m_ins.m_restoreJobInfo = std::make_shared<AppProtect::RestoreJob>(restoreJob);
    int ret = m_ins.GenerateSubJobInner();
    EXPECT_EQ(ret, Module::FAILED);
    stub.reset(ADDR(HostRestore, InitGenerateJobInfo));

    stub.set(ADDR(HostRestore, StartToScan), Funtion_Module_Failed);
    ret = m_ins.GenerateSubJobInner();
    EXPECT_EQ(ret, Module::FAILED);
    stub.reset(ADDR(HostRestore, StartToScan));

    stub.set(ADDR(HostRestore, MonitorScannerProgress), Funtion_Module_Failed);
    ret = m_ins.GenerateSubJobInner();
    EXPECT_EQ(ret, Module::FAILED);
    stub.reset(ADDR(HostRestore, MonitorScannerProgress));

    stub.set(ADDR(HostRestore, GenerateTearDownSubJob), Funtion_Module_Failed);
    ret = m_ins.GenerateSubJobInner();
    EXPECT_EQ(ret, Module::FAILED);
    stub.reset(ADDR(HostRestore, GenerateTearDownSubJob));
}

/*
 * 用例名称:GetLastCtrlPath
 * 前置条件：无
 * check点：获取上一次备份控制文件路径
 */
TEST_F(HostRestoreTest, GetLastCtrlPath)
{
    std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
        std::make_shared<AppProtect::RestoreJob>(HostRestoreTest::RestoreJobInfoSetUp());
    m_ins.m_jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);

    m_ins.m_aggregateRestore = false;
    m_ins.m_metaFsPath = "/test";
    string path = m_ins.GetLastCtrlPath();
    EXPECT_EQ(path, "/test/lastCtrl");
}

/*
 * 用例名称:检查聚合格式下的任务流程
 * 前置条件：无
 * check点：检查后置任务流程顺利运行
 */
// TEST_F(HostRestoreTest, GenerateAggregateSubJobInner)
// {
//     std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
//     std::make_shared<AppProtect::RestoreJob>(HostRestoreTest::RestoreJobInfoSetUp());
//     m_ins.m_jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);
//     m_ins.PrerequisiteJob();

//     Stub stub;
//     m_ins.m_restoreJobInfo = std::make_shared<AppProtect::RestoreJob>();
//     stub.set(ADDR(HostRestore, InitAggregateGenerateJobInfo), Funtion_Module_Failed);
//     int ret = m_ins.GenerateAggregateSubJobInner();
//     EXPECT_EQ(ret, Module::FAILED);
//     stub.reset(ADDR(HostRestore, InitAggregateGenerateJobInfo));

//     stub.set(ADDR(HostRestore, InitAggregateGenerateJobInfo), Funtion_Module_Failed);
//     stub.set(ADDR(HostRestore, GenerateRestoreExecuteSubJobsForAggregate), Funtion_Module_Failed);
//     ret = m_ins.GenerateAggregateSubJobInner();
//     EXPECT_EQ(ret, Module::FAILED);
//     stub.reset(ADDR(HostRestore, GenerateRestoreExecuteSubJobsForAggregate));
//     stub.reset(ADDR(HostRestore, InitAggregateGenerateJobInfo));
// }
