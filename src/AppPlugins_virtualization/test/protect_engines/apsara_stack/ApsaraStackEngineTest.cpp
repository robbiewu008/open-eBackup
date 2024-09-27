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
#include <protect_engines/apsara_stack/ApsaraStackProtectEngine.h>
#include "common/CommonMock.h"

using namespace OpenStackPlugin;

namespace HDT_TEST {
using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using namespace ApsaraStackPlugin;

class ApsaraStackEngineTest : public testing::Test {
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

};


void ApsaraStackEngineTest::SetUp()
{
    InitLogger();
    stub.set(sleep, Stub_Sleep);
}

void ApsaraStackEngineTest::InitLogger()
{
    std::string logFileName = "virt_plugin_apsara_stack_engine_test.log";
    std::string logFilePath = "/tmp/log/";
    int logLevel = DEBUG;
    int logFileCount = 10;
    int logFileSize = 30;
    Module::CLogger::GetInstance().Init(
        logFileName.c_str(), logFilePath, logLevel, logFileCount, logFileSize);
}

void ApsaraStackEngineTest::TearDown()
{}

void ApsaraStackEngineTest::SetUpTestCase()
{}

void ApsaraStackEngineTest::TearDownTestCase()
{}

TEST_F(ApsaraStackEngineTest, PreHook_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    ExecHookParam para;
    EXPECT_EQ(SUCCESS, apsaraStackEngine.PreHook(para));
}

TEST_F(ApsaraStackEngineTest, PostHook_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    ExecHookParam para;
    EXPECT_EQ(SUCCESS, apsaraStackEngine.PostHook(para));
}

TEST_F(ApsaraStackEngineTest, GetTaskId_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    apsaraStackEngine.m_requestId = "007";
    EXPECT_EQ("TaskId=007.", apsaraStackEngine.GetTaskId());
}

TEST_F(ApsaraStackEngineTest, CreateSnapshot_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    SnapshotInfo snapshot;
    std::string errCode;
    EXPECT_EQ(0, apsaraStackEngine.CreateSnapshot(snapshot, errCode));
}

TEST_F(ApsaraStackEngineTest, DeleteSnapshot_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    SnapshotInfo snapshot;
    EXPECT_EQ(0, apsaraStackEngine.DeleteSnapshot(snapshot));
}

TEST_F(ApsaraStackEngineTest, QuerySnapshotExists_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    SnapshotInfo snapshot;
    EXPECT_EQ(0, apsaraStackEngine.QuerySnapshotExists(snapshot));
}

TEST_F(ApsaraStackEngineTest, GetSnapshotsOfVolume_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    VolInfo volInfo;
    std::vector<VolSnapInfo> snapList {};
    EXPECT_EQ(0, apsaraStackEngine.GetSnapshotsOfVolume(volInfo, snapList));
}

TEST_F(ApsaraStackEngineTest, GetMachineMetadata_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    VMInfo vmInfo;
    EXPECT_EQ(0, apsaraStackEngine.GetMachineMetadata(vmInfo));
}

TEST_F(ApsaraStackEngineTest, GetVolumesMetadata_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    VMInfo vmInfo;
    std::unordered_map<std::string, std::string> volsMetadata {};
    EXPECT_EQ(0, apsaraStackEngine.GetVolumesMetadata(vmInfo, volsMetadata));
}

TEST_F(ApsaraStackEngineTest, DetachVolume_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    VolInfo volObj;
    EXPECT_EQ(0, apsaraStackEngine.DetachVolume(volObj));
}

TEST_F(ApsaraStackEngineTest, AttachVolume_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    VolInfo volObj;
    EXPECT_EQ(0, apsaraStackEngine.AttachVolume(volObj));
}

TEST_F(ApsaraStackEngineTest, DeleteVolume_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    VolInfo volObj;
    EXPECT_EQ(0, apsaraStackEngine.DeleteVolume(volObj));
}

TEST_F(ApsaraStackEngineTest, ReplaceVolume_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    VolInfo volObj;
    EXPECT_EQ(0, apsaraStackEngine.ReplaceVolume(volObj));
}

TEST_F(ApsaraStackEngineTest, CreateMachine_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    VMInfo vmInfo;
    EXPECT_EQ(0, apsaraStackEngine.CreateMachine(vmInfo));
}

TEST_F(ApsaraStackEngineTest, CheckBeforeRecover_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    VMInfo vmInfo;
    EXPECT_EQ(0, apsaraStackEngine.CheckBeforeRecover(vmInfo));
}

TEST_F(ApsaraStackEngineTest, CheckBeforeBackup_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    EXPECT_EQ(0, apsaraStackEngine.CheckBeforeBackup());
}

TEST_F(ApsaraStackEngineTest, DeleteMachine_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    VMInfo vmInfo;
    EXPECT_EQ(0, apsaraStackEngine.DeleteMachine(vmInfo));
}

TEST_F(ApsaraStackEngineTest, RenameMachine_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    VMInfo vmInfo;
    std::string newName;
    EXPECT_EQ(0, apsaraStackEngine.RenameMachine(vmInfo, newName));
}

TEST_F(ApsaraStackEngineTest, PowerOnMachine_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    VMInfo vmInfo;
    EXPECT_EQ(0, apsaraStackEngine.PowerOnMachine(vmInfo));
}

TEST_F(ApsaraStackEngineTest, PowerOffMachine_Test)
{
    ApsaraStackProtectEngine apsaraStackEngine;
    VMInfo vmInfo;
    EXPECT_EQ(0, apsaraStackEngine.PowerOffMachine(vmInfo));
}
}
