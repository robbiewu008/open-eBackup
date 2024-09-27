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
#include <memory>
#include "common/FSBackupUtils.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "addr_pri.h"
#include "BackupMgr.h"
#include "Backup.h"

using namespace std;
using namespace FS_Backup;

class BackupMgrTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    void FillBackupParams(BackupIOEngine srcIOEngine, BackupIOEngine dstIOEngine);
    void FillBackupAdvanceParams(shared_ptr<BackupAdvanceParams>& advParams, BackupIOEngine engine);
    BackupParams m_backupParams;

    std::unique_ptr<BackupMgr> m_backupMgr = nullptr;

};

void BackupMgrTest::SetUp()
{

}

void BackupMgrTest::TearDown()
{}

void BackupMgrTest::SetUpTestCase()
{}

void BackupMgrTest::TearDownTestCase()
{}

void BackupMgrTest::FillBackupParams(BackupIOEngine srcIOEngine, BackupIOEngine dstIOEngine)
{
    m_backupParams.srcEngine = srcIOEngine;
    m_backupParams.dstEngine = dstIOEngine;
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.phase = BackupPhase::COPY_STAGE;
    FillBackupAdvanceParams(m_backupParams.srcAdvParams, srcIOEngine);
    FillBackupAdvanceParams(m_backupParams.dstAdvParams, dstIOEngine);
}

void BackupMgrTest::FillBackupAdvanceParams(shared_ptr<BackupAdvanceParams>& advParams, BackupIOEngine engine)
{
    switch (engine)
    {
        case BackupIOEngine::POSIX:
            advParams = make_shared<HostBackupAdvanceParams>();
            break;
        case BackupIOEngine::LIBNFS:
            advParams = make_shared<LibnfsBackupAdvanceParams>();
            break;
        case BackupIOEngine::LIBS3IO:
            advParams = make_shared<LibS3IOBackupAdvanceParams>();
            break;
        case BackupIOEngine::LIBSMB:
            advParams = make_shared<LibsmbBackupAdvanceParams>();
            break;
        case BackupIOEngine::WIN32_IO:
            advParams = make_shared<HostBackupAdvanceParams>();
            break;
        default:
            break;
    }
    return;
}

/*
* 用例名称2：使用正确的高级备份参数创建备份实例
* 前置条件：填写正确的高级备份参数，各个参数符合接口要求的最大最小值范围
* check点：创建备份实例接口返回的备份实例指针不为空
*/
TEST_F(BackupMgrTest, CreateBackupInstWithCorrectParams)
{
    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    unique_ptr<Backup> backupInst = BackupMgr::CreateBackupInst(m_backupParams);
    EXPECT_NE(backupInst, nullptr);
}

/*
* 用例名称：CheckCreateBackupInst
* 前置条件：
* check点：检查CreateBackupInst返回值
*/
TEST_F(BackupMgrTest, CheckCreateBackupInst)
{
    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_backupParams.phase = BackupPhase::DELETE_STAGE;
    unique_ptr<Backup> backupInst = BackupMgr::CreateBackupInst(m_backupParams);
    EXPECT_NE(backupInst, nullptr);
}

/*
* 用例名称：CheckCreateBackupInst
* 前置条件：
* check点：检查CreateBackupInst返回值
*/
TEST_F(BackupMgrTest, CheckCreateBackupInst1)
{
    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_backupParams.phase = BackupPhase::HARDLINK_STAGE;
    unique_ptr<Backup> backupInst = BackupMgr::CreateBackupInst(m_backupParams);
    EXPECT_NE(backupInst, nullptr);
}

/*
* 用例名称：CheckCreateBackupInst1
* 前置条件：
* check点：检查CreateBackupInst返回值
*/
TEST_F(BackupMgrTest, CheckCreateBackupInst2)
{
    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_backupParams.phase = BackupPhase::DIR_STAGE;
    unique_ptr<Backup> backupInst = BackupMgr::CreateBackupInst(m_backupParams);
    EXPECT_NE(backupInst, nullptr);
}

/*
* 用例名称：CheckCreateBackupInst2
* 前置条件：
* check点：检查CreateBackupInst返回值
*/
TEST_F(BackupMgrTest, CheckCreateBackupInst3)
{
    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_backupParams.phase = BackupPhase::UNKNOWN_STAGE;
    EXPECT_NO_THROW(BackupMgr::CreateBackupInst(m_backupParams));
}

/*
* 用例名称：CheckGetBackupPlatform
* 前置条件：
* check点：检查GetBackupPlatform返回值
*/
TEST_F(BackupMgrTest, CheckGetBackupPlatform)
{
    EXPECT_EQ(BackupMgr::GetBackupPlatform(BackupIOEngine::LIBSMB), BackupPlatform::WINDOWS);
    EXPECT_EQ(BackupMgr::GetBackupPlatform(BackupIOEngine::LIBS3IO), BackupPlatform::UNKNOWN_PLATFORM);
    EXPECT_EQ(BackupMgr::GetBackupPlatform(BackupIOEngine::UNKNOWN_ENGINE), BackupPlatform::UNKNOWN_PLATFORM);
}

/*
* 用例名称：CheckValidateBackupParams
* 前置条件：
* check点：检查ValidateBackupParams返回值
*/
TEST_F(BackupMgrTest, CheckValidateBackupParams)
{
    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_backupParams.backupType = BackupType::RESTORE;
    m_backupParams.commonParams.restoreReplacePolicy = RestoreReplacePolicy::NONE;
    EXPECT_EQ(BackupMgr::ValidateBackupParams(m_backupParams), false);

    FillBackupParams(BackupIOEngine::LIBAIO, BackupIOEngine::LIBSMB);
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.commonParams.restoreReplacePolicy = RestoreReplacePolicy::IGNORE_EXIST;
    EXPECT_EQ(BackupMgr::ValidateBackupParams(m_backupParams), false);

    FillBackupParams(BackupIOEngine::LIBSMB, BackupIOEngine::LIBAIO);
    EXPECT_EQ(BackupMgr::ValidateBackupParams(m_backupParams), false);

    FillBackupParams(BackupIOEngine::LIBSMB, BackupIOEngine::LIBSMB);
    m_backupParams.commonParams.writeExtendAttribute = true;
    EXPECT_EQ(BackupMgr::ValidateBackupParams(m_backupParams), false);

    FillBackupParams(BackupIOEngine::LIBS3IO, BackupIOEngine::LIBS3IO);
    m_backupParams.commonParams.writeExtendAttribute = true;
    m_backupParams.commonParams.writeAcl = true;
    EXPECT_EQ(BackupMgr::ValidateBackupParams(m_backupParams), true);

    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_backupParams.commonParams.backupDataFormat = BackupDataFormat::AGGREGATE;
    m_backupParams.commonParams.maxAggregateFileSize = 0;
    m_backupParams.commonParams.maxFileSizeToAggregate = 1;
    EXPECT_EQ(BackupMgr::ValidateBackupParams(m_backupParams), false);
}

/*
* 用例名称：CheckPrintBackupSrcParams
* 前置条件：
* check点：检查PrintBackupSrcParams返回值
*/
TEST_F(BackupMgrTest, CheckPrintBackupSrcParams)
{
    FillBackupParams(BackupIOEngine::WIN32_IO, BackupIOEngine::POSIX);
    EXPECT_NO_THROW(BackupMgr::PrintBackupSrcParams(m_backupParams));

    FillBackupParams(BackupIOEngine::POSIXAIO, BackupIOEngine::POSIX);
    EXPECT_NO_THROW(BackupMgr::PrintBackupSrcParams(m_backupParams));

    FillBackupParams(BackupIOEngine::WINDOWSAIO, BackupIOEngine::POSIX);
    EXPECT_NO_THROW(BackupMgr::PrintBackupSrcParams(m_backupParams));
    
    FillBackupParams(BackupIOEngine::ARCHIVE_CLIENT, BackupIOEngine::POSIX);
    EXPECT_NO_THROW(BackupMgr::PrintBackupSrcParams(m_backupParams));

    FillBackupParams(BackupIOEngine::UNKNOWN_ENGINE, BackupIOEngine::POSIX);
    EXPECT_NO_THROW(BackupMgr::PrintBackupSrcParams(m_backupParams));
}

/*
* 用例名称：CheckPrintBackupDstParams
* 前置条件：
* check点：检查PrintBackupDstParams返回值
*/
TEST_F(BackupMgrTest, CheckPrintBackupDstParams)
{
    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::WIN32_IO);
    EXPECT_NO_THROW(BackupMgr::PrintBackupDstParams(m_backupParams));

    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::POSIXAIO);
    EXPECT_NO_THROW(BackupMgr::PrintBackupDstParams(m_backupParams));

    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::WINDOWSAIO);
    EXPECT_NO_THROW(BackupMgr::PrintBackupDstParams(m_backupParams));
    
    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::ARCHIVE_CLIENT);
    EXPECT_NO_THROW(BackupMgr::PrintBackupDstParams(m_backupParams));

    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::UNKNOWN_ENGINE);
    EXPECT_NO_THROW(BackupMgr::PrintBackupDstParams(m_backupParams));
}

/*
* 用例名称：CheckCreateBackupInst4
* 前置条件：
* check点：检查CreateBackupInst返回值
*/
TEST_F(BackupMgrTest, CheckCreateBackupInst4)
{
    const char* source = "111";
    const char* destination = "111";
    const char* metaPath = "111";
    bool writeMeta = true;

    int phase = 0;
    EXPECT_NO_THROW(CreateBackupInst(source, destination, metaPath, phase, writeMeta));

    phase = BACKUP_PHASE_COPY;
    EXPECT_NO_THROW(CreateBackupInst(source, destination, metaPath, phase, writeMeta));

    phase = BACKUP_PHASE_DELETE;
    EXPECT_NO_THROW(CreateBackupInst(source, destination, metaPath, phase, writeMeta));

    phase = BACKUP_PHASE_HARDLINK;
    EXPECT_NO_THROW(CreateBackupInst(source, destination, metaPath, phase, writeMeta));

    phase = BACKUP_PHASE_DIR;
    EXPECT_NO_THROW(CreateBackupInst(source, destination, metaPath, phase, writeMeta));
}

class Backup_1 : public Backup {
public:
    Backup_1(const BackupParams& backupParams) : Backup(backupParams) {}
    BackupRetCode Start() {}
    BackupRetCode Abort() {}
    BackupRetCode Destroy() {}
    BackupRetCode Enqueue(std::string controlFile) {}
    BackupPhaseStatus GetStatus() {}
    BackupStats GetStats() {}
    std::unordered_set<FailedRecordItem, FailedRecordItemHash> GetFailedDetails() {}
};

/*
* 用例名称：CheckConfigureThreadPool
* 前置条件：
* check点：检查ConfigureThreadPool返回值
*/
TEST_F(BackupMgrTest, CheckConfigureThreadPool)
{
    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    Backup_1* backupHandle = new Backup_1(m_backupParams);
    int readThreadNum = 1;
    int writeThreadNum = 1;
    EXPECT_EQ(ConfigureThreadPool((void*)(backupHandle), readThreadNum, writeThreadNum), 0);
    delete backupHandle;
}

/*
* 用例名称：CheckConfigureMemory
* 前置条件：
* check点：检查ConfigureMemory返回值
*/
TEST_F(BackupMgrTest, CheckConfigureMemory)
{
    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    Backup_1* backupHandle = new Backup_1(m_backupParams);
    int maxMemory = 1;
    EXPECT_EQ(ConfigureMemory((void*)(backupHandle), maxMemory), 0);
    delete backupHandle;
}

/*
* 用例名称：CheckEnqueue
* 前置条件：
* check点：检查Enqueue返回值
*/
TEST_F(BackupMgrTest, CheckEnqueue)
{
    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    Backup_1* backupHandle = new Backup_1(m_backupParams);
    const char* backupControlFile = "111";
    EXPECT_EQ(Enqueue((void*)(backupHandle), backupControlFile), -1);
    delete backupHandle;
}

/*
* 用例名称：CheckStart
* 前置条件：
* check点：检查Start返回值
*/
TEST_F(BackupMgrTest, CheckStart)
{
    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    Backup_1* backupHandle = new Backup_1(m_backupParams);
    EXPECT_EQ(Start((void*)(backupHandle)), -1);
    delete backupHandle;
}

/*
* 用例名称：CheckGetStatus
* 前置条件：
* check点：检查GetStatus返回值
*/
TEST_F(BackupMgrTest, CheckGetStatus)
{
    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    Backup_1* backupHandle = new Backup_1(m_backupParams);
    EXPECT_NO_THROW(GetStatus((void*)(backupHandle)));
    delete backupHandle;
}

/*
* 用例名称：CheckGetStats
* 前置条件：
* check点：检查GetStats返回值
*/
TEST_F(BackupMgrTest, CheckGetStats)
{
    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    Backup_1* backupHandle = new Backup_1(m_backupParams);
    BackupStatistics backupStats = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    EXPECT_NO_THROW(GetStats((void*)(backupHandle), &backupStats));
    delete backupHandle;
}

/*
* 用例名称：CheckDestroyBackupInst
* 前置条件：
* check点：检查DestroyBackupInst返回值
*/
TEST_F(BackupMgrTest, CheckDestroyBackupInst)
{
    FillBackupParams(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    Backup_1* backupHandle = new Backup_1(m_backupParams);
    EXPECT_NO_THROW(DestroyBackupInst((void*)(backupHandle)));
}
