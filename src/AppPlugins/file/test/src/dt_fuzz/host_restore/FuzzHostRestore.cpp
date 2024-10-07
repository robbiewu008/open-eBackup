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
#include "host_restore/HostRestore.h"
#include "Backup.h"
#include "ScanMgr.h"

using namespace std;
using namespace FilePlugin;
using namespace AppProtect;
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
    return std::unordered_set<FailedRecordItem, FailedRecordItemHash> {};
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
static unique_ptr<Scanner> CreateScanInst_Stub (void* obj, ScanConfig scanConfig) {
    ScanConfig xx;
    return make_unique<Scanner_test>(xx);
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
static int FunctionConfigGetInt(void* obj,const string & sectionName, const string & keyName,bool logFlag)
{
    return 1000;
}
static int Stub_Config_getInt(void* obj, const string & sectionName, const string & keyName,bool logFlag)
{
    return 100;
}
static std::string Stub_Config_getString(void* obj) {
    return "";
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


class FuzzHostRestore : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    AppProtect::RestoreJob RestoreJobInfoSetUp();
    static void ReportProgressSucc();
    HostRestore m_ins;
    Stub m_stub;
   
};

void FuzzHostRestore::TearDown() {}

void FuzzHostRestore::SetUpTestCase() {}

void FuzzHostRestore::TearDownTestCase() {}
void FuzzHostRestore::SetUp()
{
    m_stub.set(ADDR(JobService, ReportJobDetails), FuzzHostRestore::ReportProgressSucc);
    m_stub.set(ADDR(Module::ConfigReader, getInt), Stub_Config_getInt);
    m_stub.set(ADDR(Module::ConfigReader, getString), Stub_Config_getString);
}

void FuzzHostRestore::ReportProgressSucc() {
    return;
}
static unique_ptr<FS_Backup::Backup> CreateBackupInstStub_Succ(void* obj, BackupParams backupParams) {
    BackupParams xx;
    return make_unique<FS_Backup::testBackup>(xx);
}
AppProtect::RestoreJob FuzzHostRestore::RestoreJobInfoSetUp()
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
    fileSetInfo.paths = "[{\"name\":\"/xiaoding_restore\"}]";
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
    dataRepo.remotePath = "/tmp/BackupFS/remote";
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
    return restoreJob;
}



TEST_F(FuzzHostRestore, PrerequisiteJob)
{
    std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
    std::make_shared<AppProtect::RestoreJob>(FuzzHostRestore::RestoreJobInfoSetUp());
    m_ins.m_jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);
    DT_FUZZ_START(0, 10,(char*)"FuzzHostRestore_PrerequisiteJob" ,0)
    {
        int ret = m_ins.PrerequisiteJob();
        EXPECT_EQ(ret, Module::SUCCESS);
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzHostRestore, GenerateSubJob)
{
     std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
        std::make_shared<AppProtect::RestoreJob>(FuzzHostRestore::RestoreJobInfoSetUp());
    m_ins.m_jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);
    g_firstCall = true;
    Stub stub;
    stub.set(ADDR(ScanMgr, CreateScanInst), CreateScanInst_Stub);
    stub.set(PluginUtils::CreateDirectory, FunctionBoolSucc);
    stub.set(PluginUtils::RemoveFile, FunctionBoolSucc);
    stub.set(PluginUtils::CopyFile, FunctionBoolSucc);
    stub.set(Module::runShellCmdWithOutput, FunctionIntSucc);
    stub.set(PluginUtils::GetFileListInDirectory, GetFileListInDirectoryStub_Succ);
    stub.set(ADDR(JobService, AddNewJob), FunctionVoidSucc);
    stub.set((bool(ShareResourceManager::*)(ShareResourceType, const std::string &,
        BackupStatistic&))ADDR(ShareResourceManager,InitResource), FunctionBoolSucc);

    DT_FUZZ_START(0, 10,(char*)"FuzzHostRestore_GenerateSubJob" ,0)
    {
        auto restoreJobPtr = dynamic_pointer_cast<AppProtect::RestoreJob>(m_ins.m_jobCommonInfo->GetJobInfo());
        char *targetName = DT_SetGetString(&g_Element[0], 4, 100 ,(char *)"123");
        restoreJobPtr->targetObject.name = targetName;
        int ret = m_ins.GenerateSubJob();
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzHostRestore, ExecuteSubJob)
{
     std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
        std::make_shared<AppProtect::RestoreJob>(FuzzHostRestore::RestoreJobInfoSetUp());
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

    DT_FUZZ_START(0, 10, (char*)"FuzzHostRestore_ExecuteSubJob" ,0)
    {
        auto restoreJobPtr = dynamic_pointer_cast<AppProtect::RestoreJob>(m_ins.m_jobCommonInfo->GetJobInfo());
        char *targetName = DT_SetGetString(&g_Element[0], 4, 100 ,(char *)"123");
        restoreJobPtr->targetObject.name = targetName;
        int ret = m_ins.ExecuteSubJob();
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzHostRestore, PostJob)
{
    std::shared_ptr<ThriftDataBase> restoreJobInfoPtr =
        std::make_shared<AppProtect::RestoreJob>(FuzzHostRestore::RestoreJobInfoSetUp());
    m_ins.m_jobCommonInfo = std::make_shared<JobCommonInfo>(restoreJobInfoPtr);
    Stub stub;
    stub.set(PluginUtils::Remove, FunctionBoolSucc);
    DT_FUZZ_START(0, 10,(char*)"FuzzHostRestore_PostJob" ,0)
    {
        auto restoreJobPtr = dynamic_pointer_cast<AppProtect::RestoreJob>(m_ins.m_jobCommonInfo->GetJobInfo());
        char *jobIdStr = DT_SetGetString(&g_Element[0], 10, 100 ,(char *)"123456789");
        restoreJobPtr->jobId = jobIdStr;
        int ret = m_ins.PostJob();
    }
    DT_FUZZ_END()
    return;
}
