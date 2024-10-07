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
#include "stub.h"
#include <protect_engines/cnware/CNwareProtectEngine.h>

using namespace VirtPlugin;
using namespace CNwarePlugin;

namespace HDT_TEST {

class CNwareProtectEngineTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void InitLogger();

public:
    Stub stub;

private:
    AppProtect::BackupJob m_backupInfo;
    AppProtect::RestoreJob m_restoreInfo;

}


void CNwareProtectEngineTest::SetUp()
{
    InitLogger();
    stub.set(sleep, Stub_Sleep);
}

void CNwareProtectEngineTest::InitLogger()
{
    std::string logFileName = "virt_plugin_cnware_engine_test.log";
    std::string logFilePath = "/tmp/log/";
    int logLevel = DEBUG;
    int logFileCount = 10;
    int logFileSize = 30;
    Module::CLogger::GetInstance().Init(
        logFileName.c_str(), logFilePath, logLevel, logFileCount, logFileSize);
}

bool StubParseCertSuccess(const std::string certInfo)
{
    return true;
}

bool StubParseCertFailed(const std::string certInfo)
{
    return false;
}

bool StubParseCertInfoF(const std::string &certInfo, T &cert)
{
    return false;
}

bool StubParseCertInfoT(const std::string &certInfo, T &cert)
{
    return true;
}

bool StubCommonBoolF()
{
    return false;
}

bool StubCommonBoolT()
{
    return true;
}

bool StubSaveCertToFileF(const std::string& fileName)
{
    return false;
}

bool StubSaveCertToFileT(const std::string& fileName)
{
    return true;
}

void StubRemoveCrtFile(const std::string& markId)
{
    return;
}

bool StubInitCNwareClientF(const ApplicationEnvironment &appEnv)
{
    return false;
}

bool StubInitCNwareClientT(const ApplicationEnvironment &appEnv)
{
    return true;
}

bool StubInitClientF(int64_t& errorCode)
{
    return false;
}

bool StubInitClientT(int64_t& errorCode)
{
    return true;
}

int32_t StubGetSessionAndloginF(CNwareRequest &req, int64_t &errorCode)
{
    return FAILED;
}

int32_t StubGetSessionAndloginT(CNwareRequest &req, int64_t &errorCode)
{
    return SUCCESS;
}

int32_t StubGetVersionInfoF(CNwareRequest &req, ApplicationEnvironment &returnEnv)
{
    returnEnv.__set_extendInfo = "007";
    return FAILED;
}

int32_t StubGetVersionInfoT(CNwareRequest &req, ApplicationEnvironment &returnEnv)
{
    returnEnv.__set_extendInfo = "007";
    return SUCCESS;
}

int32_t StubGetTargetResourceF(ResourceResultByPage& page, CNwareRequest &req)
{
    return FAILED;
}

int32_t StubGetTargetResourceT(ResourceResultByPage& page, CNwareRequest &req)
{
    page.__set_total(6);
    return SUCCESS;
}

bool StubCheckCNwareVersionFailed()
{
    return false;
}

bool StubCheckCNwareVersionSuccess()
{
    return true;
}

int32_t StubCheckStorageConnectionFailed(const AppProtect::BackupJob &job, int32_t &erroCode)
{
    return FAILED;
}

int32_t StubCheckStorageConnectionFailed(const AppProtect::BackupJob &job, int32_t &erroCode)
{
    return SUCCESS;
}

std::shared_ptr<BuildNewVMResponse> StubBuildNewClient(BuildNewVMRequest &req)
{
    std::shared_ptr<BuildNewVMResponse> response = std::make_shared<BuildNewVMResponse>();
    return response;
}

bool StubCheckTaskStatusSuccess(const std::string &taskId)
{
    return true;
}

std::shared_ptr<CheckNameUniqueResponse> StubCheckNameUniqueSuccess(CheckNameUniqueRequest &req)
{
    std::shared_ptr<CheckNameUniqueResponse> response = std::make_shared<CheckNameUniqueResponse>();
    response->m_checkRes.mName = true;
    return response;
}

std::shared_ptr<CNwareResponse> StubDeleteVMSuccess(DeleteVMRequest &req)
{
    std::shared_ptr<CNwareResponse> response = std::make_shared<CNwareResponse>();
    return response;
}

std::shared_ptr<CNwareResponse> StubPowerOnVMSuccess(CNwareRequest &req)
{
    std::shared_ptr<CNwareResponse> response = std::make_shared<CNwareResponse>();
    return response;
}

std::shared_ptr<CNwareResponse> StubPowerOffVMSuccess(CNwareRequest &req)
{
    std::shared_ptr<CNwareResponse> response = std::make_shared<CNwareResponse>();
    return response;
}

TEST_F(CNwareProtectEngineTest, Init_Test)
{
    AppProtect::ApplicationEnvironment appEnv;
    CNwareProtectEngine cnwareEngine;
    cnwareEngine.SetAppEnv(appEnv);
    stub.set(ADDR(CNwareProtectEngine, ParseCert), StubParseCertFailed);
    // Failed
    EXPECT_EQ(false, cnwareEngine.init());
    // Success
    stub.set(ADDR(CNwareProtectEngine, ParseCert), StubParseCertSuccess);
    EXPECT_EQ(true, cnwareEngine.init());
}

TEST_F(CNwareProtectEngineTest, ParseCert_Test)
{
    CNwareProtectEngine cnwareEngine;
    std::string certInfo;
    // Failed
    EXPECT_EQ(false, cnwareEngine.ParseCert(certInfo));

    certInfo = "007";
    EXPECT_EQ(false, cnwareEngine.ParseCert(certInfo));

    cnwareEngine.m_certMgr = std::make_shared<CertManger>();
    stub.set(ADDR(CertManger, ParseCertInfo), StubParseCertInfoF);
    EXPECT_EQ(false, cnwareEngine.ParseCert(certInfo));

    stub.set(ADDR(CertManger, ParseCertInfo), StubParseCertInfoT);
    stub.set(ADDR(CertManger, IsVerifyCert), StubCommonBoolT);
    stub.set(ADDR(CertManger, SaveCertToFile), StubSaveCertToFileF);
    EXPECT_EQ(false, cnwareEngine.ParseCert(certInfo));

    // Success
    stub.set(ADDR(CertManger, ParseCertInfo), StubParseCertInfoT);
    stub.set(ADDR(CertManger, IsVerifyCert), StubCommonBoolT);
    stub.set(ADDR(CertManger, SaveCertToFile), StubSaveCertToFileT);
    EXPECT_EQ(true, cnwareEngine.ParseCert(certInfo));

    stub.set(ADDR(CertManger, ParseCertInfo), StubParseCertInfoT);
    stub.set(ADDR(CertManger, IsVerifyCert), StubCommonBoolF);
    stub.set(ADDR(CertManger, RemoveCrtFile), StubRemoveCrtFile);
    EXPECT_EQ(true, cnwareEngine.ParseCert(certInfo));
}

TEST_F(CNwareProtectEngineTest, InitClient_Test)
{
    CNwareProtectEngine cnwareEngine;
    int64_t errorCode;

    // false
    EXPECT_EQ(false, cnwareEngine.InitClient(errorCode));

    AppProtect::ApplicationEnvironment appEnv;
    cnwareEngine.SetAppEnv(appEnv);
    cnwareEngine.m_cnwareClient = std::make_shared<CNwareClient>(m_appEnv.auth);
    stub.set(ADDR(CNwareClient, InitCNwareClient), StubInitCNwareClientF);
    EXPECT_EQ(false, cnwareEngine.InitClient(errorCode));

    stub.set(ADDR(CertManger, IsVerifyCert), StubCommonBoolF);
    stub.set(ADDR(CNwareClient, InitCNwareClient), StubInitCNwareClientT);
    stub.set(ADDR(CNwareClient, GetSessionAndlogin), StubGetSessionAndloginF);
    EXPECT_EQ(false, cnwareEngine.InitClient(errorCode));

    // success
    stub.set(ADDR(CNwareClient, GetSessionAndlogin), StubGetSessionAndloginT);
    EXPECT_EQ(true, cnwareEngine.InitClient(errorCode));
}

TEST_F(CNwareProtectEngineTest, DiscoverApplications_Test)
{
    CNwareProtectEngine cnwareEngine;
    std::vector<Application> returnValue;
    std::string appType;
    cnwareEngine.DiscoverApplications(returnValue, appType);
}

TEST_F(CNwareProtectEngineTest, CheckApplication_Test)
{
    CNwareProtectEngine cnwareEngine;
    AppProtect::Application application;
    ActionResult returnValue;
    ApplicationEnvironment appEnv;
    stub.set(ADDR(CNwareProtectEngine, Init), StubCommonBoolF);
    cnwareEngine.CheckApplication(returnValue, appEnv, application);
    EXPECT_EQ(200, returnValue.code);

    stub.set(ADDR(CNwareProtectEngine, Init), StubCommonBoolT);
    stub.set(ADDR(CNwareProtectEngine, InitClient), StubInitClientF);
    cnwareEngine.CheckApplication(returnValue, appEnv, application);
    EXPECT_EQ(200, returnValue.code);

    stub.set(ADDR(CNwareProtectEngine, InitClient), StubInitClientT);
    cnwareEngine.CheckApplication(returnValue, appEnv, application);
    EXPECT_EQ(0, returnValue.code);
}

TEST_F(CNwareProtectEngineTest, DiscoverHostCluster_Test)
{
    CNwareProtectEngine cnwareEngine;
    ApplicationEnvironment returnEnv;
    ApplicationEnvironment appEnv;
    cnwareEngine.DiscoverHostCluster(returnValue, appEnv);
}

TEST_F(CNwareProtectEngineTest, DiscoverAppCluster_Test)
{
    CNwareProtectEngine cnwareEngine;
    ApplicationEnvironment returnEnv;
    ApplicationEnvironment appEnv;
    appEnv.id = "007";
    Application application;
    stub.set(ADDR(CNwareProtectEngine, Init), StubCommonBoolF);
    cnwareEngine.DiscoverAppCluster(returnValue, appEnv, application);

    stub.set(ADDR(CNwareProtectEngine, Init), StubCommonBoolT);
    stub.set(ADDR(CNwareProtectEngine, InitClient), StubInitClientF);
    cnwareEngine.DiscoverAppCluster(returnValue, appEnv, application);

    stub.set(ADDR(CNwareProtectEngine, InitClient), StubInitClientT);
    cnwareEngine.SetAppEnv(appEnv);
    cnwareEngine.m_cnwareClient = nullptr;
    cnwareEngine.DiscoverAppCluster(returnValue, appEnv, application);

    cnwareEngine.m_cnwareClient = std::make_shared<CNwareClient>(m_appEnv.auth);
    stub.set(ADDR(CNwareClient, GetVersionInfo), StubGetVersionInfoF);
    cnwareEngine.DiscoverAppCluster(returnValue, appEnv, application);
    EXPECT_EQ("", returnValue.id);

    stub.set(ADDR(CNwareClient, GetVersionInfo), StubGetVersionInfoT);
    cnwareEngine.DiscoverAppCluster(returnValue, appEnv, application);
    EXPECT_EQ("007", returnValue.id);
    EXPECT_EQ("007", returnValue.extendInfo);
}

TEST_F(CNwareProtectEngineTest, ListApplicationResourceV2_Test)
{
    CNwareProtectEngine cnwareEngine;
    ResourceResultByPage page;
    ListResourceRequest request;

    stub.set(ADDR(CNwareProtectEngine, Init), StubCommonBoolF);
    cnwareEngine.ListApplicationResourceV2(page, request);

    stub.set(ADDR(CNwareProtectEngine, Init), StubCommonBoolT);
    stub.set(ADDR(CNwareProtectEngine, InitClient), StubInitClientF);
    cnwareEngine.ListApplicationResourceV2(page, request);

    stub.set(ADDR(CNwareProtectEngine, Init), StubCommonBoolT);
    stub.set(ADDR(CNwareProtectEngine, InitClient), StubInitClientT);
    stub.set(ADDR(CNwareResourceManager, InitClient), StubGetTargetResourceF);
    cnwareEngine.ListApplicationResourceV2(page, request);

    stub.set(ADDR(CNwareProtectEngine, Init), StubCommonBoolT);
    stub.set(ADDR(CNwareProtectEngine, InitClient), StubInitClientT);
    stub.set(ADDR(CNwareResourceManager, InitClient), StubGetTargetResourceT);
    cnwareEngine.ListApplicationResourceV2(page, request);
    EXPECT_EQ(6, page.total);
}

TEST_F(CNwareProtectEngineTest, PreHook_Test)
{
    CNwareProtectEngine cnwareEngine;
    ExecHookParam para;
    EXPECT_EQ(SUCCESS, cnwareEngine.PreHook(para));
}

TEST_F(CNwareProtectEngineTest, PostHook_Test)
{
    CNwareProtectEngine cnwareEngine;
    ExecHookParam para;
    EXPECT_EQ(SUCCESS, cnwareEngine.PostHook(para));
}

TEST_F(CNwareProtectEngineTest, GetTaskId_Test)
{
    CNwareProtectEngine cnwareEngine;
    cnwareEngine.m_requestId = "007";
    EXPECT_EQ("TaskId=007.", cnwareEngine.GetTaskId());
}

TEST_F(CNwareProtectEngineTest, CreateSnapshot_Test)
{
    CNwareProtectEngine cnwareEngine;
    SnapshotInfo snapshot;
    std::string errCode;
    EXPECT_EQ(0, cnwareEngine.CreateSnapshot(snapshot, errCode));
}

TEST_F(CNwareProtectEngineTest, DeleteSnapshot_Test)
{
    CNwareProtectEngine cnwareEngine;
    SnapshotInfo snapshot;
    EXPECT_EQ(0, cnwareEngine.DeleteSnapshot(snapshot));
}

TEST_F(CNwareProtectEngineTest, QuerySnapshotExists_Test)
{
    CNwareProtectEngine cnwareEngine;
    SnapshotInfo snapshot;
    EXPECT_EQ(0, cnwareEngine.QuerySnapshotExists(snapshot));
}

TEST_F(CNwareProtectEngineTest, GetSnapshotsOfVolume_Test)
{
    CNwareProtectEngine cnwareEngine;
    VolInfo volInfo;
    std::vector<VolSnapInfo> snapList {};
    EXPECT_EQ(0, cnwareEngine.GetSnapshotsOfVolume(volInfo, snapList));
}

TEST_F(CNwareProtectEngineTest, GetMachineMetadata_Test)
{
    CNwareProtectEngine cnwareEngine;
    VMInfo vmInfo;
    EXPECT_EQ(0, cnwareEngine.GetMachineMetadata(vmInfo));
}

TEST_F(CNwareProtectEngineTest, GetVolumesMetadata_Test)
{
    CNwareProtectEngine cnwareEngine;
    VMInfo vmInfo;
    std::unordered_map<std::string, std::string> volsMetadata {};
    EXPECT_EQ(0, cnwareEngine.GetVolumesMetadata(vmInfo, volsMetadata));
}

TEST_F(CNwareProtectEngineTest, DetachVolume_Test)
{
    CNwareProtectEngine cnwareEngine;
    VolInfo volObj;
    EXPECT_EQ(0, cnwareEngine.DetachVolume(volObj));
}

TEST_F(CNwareProtectEngineTest, AttachVolume_Test)
{
    CNwareProtectEngine cnwareEngine;
    VolInfo volObj;
    EXPECT_EQ(0, cnwareEngine.AttachVolume(volObj));
}

TEST_F(CNwareProtectEngineTest, DeleteVolume_Test)
{
    CNwareProtectEngine cnwareEngine;
    VolInfo volObj;
    EXPECT_EQ(0, cnwareEngine.DeleteVolume(volObj));
}

TEST_F(CNwareProtectEngineTest, ReplaceVolume_Test)
{
    CNwareProtectEngine cnwareEngine;
    VolInfo volObj;
    EXPECT_EQ(0, cnwareEngine.ReplaceVolume(volObj));
}

TEST_F(CNwareProtectEngineTest, CreateMachine_Test)
{
    CNwareProtectEngine cnwareEngine;
    cnwareEngine.m_cnwareClient = std::make_shared<CNwareClient>(m_appEnv.auth);
    stub.set(ADDR(CNwareClient, BuildNewClient), StubBuildNewClient);
    stub.set(ADDR(CNwareProtectEngine, CheckTaskStatus), StubCheckTaskStatusSuccess);
    VMInfo vmInfo;
    EXPECT_EQ(0, cnwareEngine.CreateMachine(vmInfo));
}

TEST_F(CNwareProtectEngineTest, CheckBeforeRecover_Test)
{
    CNwareProtectEngine cnwareEngine;
    cnwareEngine.m_cnwareClient = std::make_shared<CNwareClient>(m_appEnv.auth);
    cnwareEngine.m_RestoreLevel = RestoreLevel::RESTORE_TYPE_VM;
    stub.set(ADDR(CNwareClient, CheckNameUnique), StubCheckNameUniqueSuccess);
    stub.set(ADDR(CNwareProtectEngine, CheckTaskStatus), StubCheckTaskStatusSuccess);
    VMInfo vmInfo;
    EXPECT_EQ(0, cnwareEngine.CheckBeforeRecover(vmInfo));
}

TEST_F(CNwareProtectEngineTest, CheckBeforeBackup_Test)
{
    CNwareProtectEngine cnwareEngine;
    EXPECT_EQ(0, cnwareEngine.CheckBeforeBackup());
}

TEST_F(CNwareProtectEngineTest, DeleteMachine_Test)
{
    CNwareProtectEngine cnwareEngine;
    cnwareEngine.m_cnwareClient = std::make_shared<CNwareClient>(m_appEnv.auth);
    stub.set(ADDR(CNwareClient, DeleteVM), StubDeleteVMSuccess);
    stub.set(ADDR(CNwareProtectEngine, CheckTaskStatus), StubCheckTaskStatusSuccess);
    VMInfo vmInfo;
    EXPECT_EQ(0, cnwareEngine.DeleteMachine(vmInfo));
}

TEST_F(CNwareProtectEngineTest, RenameMachine_Test)
{
    CNwareProtectEngine cnwareEngine;
    VMInfo vmInfo;
    std::string newName;
    EXPECT_EQ(0, cnwareEngine.RenameMachine(vmInfo, newName));
}

TEST_F(CNwareProtectEngineTest, PowerOnMachine_Test)
{
    CNwareProtectEngine cnwareEngine;
    cnwareEngine.m_cnwareClient = std::make_shared<CNwareClient>(m_appEnv.auth);
    stub.set(ADDR(CNwareClient, PowerOnVM), StubPowerOnVMSuccess);
    stub.set(ADDR(CNwareProtectEngine, CheckTaskStatus), StubCheckTaskStatusSuccess);
    VMInfo vmInfo;
    EXPECT_EQ(0, cnwareEngine.PowerOnMachine(vmInfo));
}

TEST_F(CNwareProtectEngineTest, PowerOffMachine_Test)
{
    CNwareProtectEngine cnwareEngine;
    cnwareEngine.m_cnwareClient = std::make_shared<CNwareClient>(m_appEnv.auth);
    stub.set(ADDR(CNwareClient, PowerOffVM), StubPowerOffVMSuccess);
    stub.set(ADDR(CNwareProtectEngine, CheckTaskStatus), StubCheckTaskStatusSuccess);
    VMInfo vmInfo;
    EXPECT_EQ(0, cnwareEngine.PowerOffMachine(vmInfo));
}

TEST_F(CNwareProtectEngineTest, AllowBackupInLocalNode_Test)
{
    CNwareProtectEngine cnwareEngine;
    AppProtect::BackupJob job;
    AppProtect::ApplicationEnvironment appEnv;

    job.protectEnv = appEnv;
    job.job.protectObject.id = "testId";
    int32_t errcode = 0;

    // Failed
    stub.set(ADDR(CNwareProtectEngine, ParseCert), StubParseCertFailed);
    stub.set(ADDR(CNwareProtectEngine, CheckStorageConnection), StubCheckStorageConnectionFailed);
    EXPECT_EQ(-1, cnwareEngine.AllowBackupInLocalNode(job, errcode));

    stub.set(ADDR(CNwareProtectEngine, ParseCert), StubParseCertSuccess);
    EXPECT_EQ(-1, cnwareEngine.AllowBackupInLocalNode(job, errcode));

    // Success
    stub.set(ADDR(CNwareProtectEngine, CheckStorageConnection), StubCheckStorageConnectionSuccess);
    EXPECT_EQ(0, cnwareEngine.AllowBackupInLocalNode(job, errcode));

}

TEST_F(CNwareProtectEngineTest, AllowBackupInLocalNode_Test)
{
    CNwareProtectEngine cnwareEngine;
    AppProtect::BackupJob job;
    AppProtect::ApplicationEnvironment appEnv;

    job.protectEnv = appEnv;
    job.job.protectObject.id = "testId";
    int32_t errcode = 0;

    // Failed
    stub.set(ADDR(CNwareProtectEngine, ParseCert), StubParseCertFailed);
    stub.set(ADDR(CNwareProtectEngine, CheckCNwareVersion), StubCheckCNwareVersionFailed);
    EXPECT_EQ(-1, cnwareEngine.AllowBackupInLocalNode(job, errcode));

    stub.set(ADDR(CNwareProtectEngine, ParseCert), StubParseCertSuccess);
    EXPECT_EQ(-1, cnwareEngine.AllowBackupInLocalNode(job, errcode));

    // Success
    stub.set(ADDR(CNwareProtectEngine, CheckCNwareVersion), StubCheckCNwareVersionSuccess);
    EXPECT_EQ(0, cnwareEngine.AllowBackupInLocalNode(job, errcode));

}

TEST_F(CNwareProtectEngineTest, AllowBackupSubJobInLocalNode_Test)
{
    CNwareProtectEngine cnwareEngine;
    AppProtect::BackupJob job;
    AppProtect::SubJob subJob;
    AppProtect::ApplicationEnvironment appEnv;

    job.protectEnv = appEnv;
    job.job.protectObject.id = "testId";
    int32_t errcode = 0;

    // Failed
    stub.set(ADDR(CNwareProtectEngine, ParseCert), StubParseCertFailed);
    stub.set(ADDR(CNwareProtectEngine, CheckCNwareVersion), StubCheckCNwareVersionFailed);
    stub.set(ADDR(CNwareProtectEngine, CheckStorageConnection), StubCheckStorageConnectionFailed);
    EXPECT_EQ(-1, cnwareEngine.AllowBackupSubJobInLocalNode(job, subJob, errcode));

    stub.set(ADDR(CNwareProtectEngine, ParseCert), StubParseCertSuccess);
    EXPECT_EQ(-1, cnwareEngine.AllowBackupSubJobInLocalNode(job, subJob, errcode));

    stub.set(ADDR(CNwareProtectEngine, CheckCNwareVersion), StubCheckCNwareVersionSuccess);
    EXPECT_EQ(-1, cnwareEngine.AllowBackupSubJobInLocalNode(job, subJob, errcode));

    // Success
    stub.set(ADDR(CNwareProtectEngine, CheckStorageConnection), StubCheckStorageConnectionSuccess);
    EXPECT_EQ(0, cnwareEngine.AllowBackupSubJobInLocalNode(job, subJob, errcode));

}
}
