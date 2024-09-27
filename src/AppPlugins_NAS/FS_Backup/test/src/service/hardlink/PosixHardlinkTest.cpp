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
#include "Backup.h"
#include "BackupMgr.h"

using namespace std;
using namespace FS_Backup;

namespace {
    constexpr auto CONTROL_FILE = "/home/shuai/test_backup/ctrl/control_0_cacfcf5a831b4a7f98c80e772c82ad71.txt";
    constexpr auto META_FILE_PATH = "/home/shuai/test_backup/meta/latest";
    constexpr auto SRC_ROOT_PATH = "/";
    constexpr auto DST_ROOT_PATH = "/home/shuai/test_backup/dst";
    constexpr auto MODULE = "BACKUP_DEMO";
    const uint64_t KILOBYTE = 1024;
    const uint64_t MEGABYTE = 1024 * 1024;
    const uint64_t GIGABYTE = 1024 * 1024 * 1024;
    const uint64_t TARABYTE = 0x010000000000;
}

class PosixHardlinkBackupTest : public testing::Test {

public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    void FillBackupParams(BackupParams& backupParams);
};

void PosixHardlinkBackupTest::SetUp()
{

}

void PosixHardlinkBackupTest::TearDown()
{

}

void PosixHardlinkBackupTest::SetUpTestCase()
{

}

void PosixHardlinkBackupTest::TearDownTestCase()
{

}

void PosixHardlinkBackupTest::FillBackupParams(BackupParams& backupParams)
{
    backupParams.srcEngine = BackupIOEngine::POSIX;
    backupParams.dstEngine = BackupIOEngine::POSIX;

    HostBackupAdvanceParams posixBackupAdvanceParams {};
    backupParams.srcAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);
    backupParams.dstAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);
    dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->dataPath = SRC_ROOT_PATH;
    dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.dstAdvParams)->dataPath = DST_ROOT_PATH;

    CommonParams commonParams {};
    commonParams.maxBufferCnt = 10;
    commonParams.maxBufferSize = 10 * 1024; // 10kb
    commonParams.maxErrorFiles = 100;
    commonParams.backupDataFormat = BackupDataFormat::NATIVE;
    commonParams.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    commonParams.jobId = "jobId";
    commonParams.subJobId = "subJobId";
    commonParams.skipFailure = true; // stop backup if any item backup failed.
    backupParams.commonParams = commonParams;

    ScanAdvanceParams scanAdvParams {};
    scanAdvParams.metaFilePath = META_FILE_PATH;
    backupParams.scanAdvParams = scanAdvParams;

    backupParams.backupType = BackupType::BACKUP_FULL;
    backupParams.phase = BackupPhase::COPY_STAGE;
}

/*
 * 用例名称:检查Posix备份流程
 * 前置条件：生成扫描结果文件controlFile 和 metaFile
 * check点：Posix备份流程顺利运行
 */
// TEST_F(PosixHardlinkBackupTest, testBackup)
// {
//     unique_ptr<Backup> m_backup = nullptr;
//     BackupParams params {};
//     FillBackupParams(params);
//     m_backup = BackupMgr::CreateBackupInst(params);

//     if (BackupRetCode::SUCCESS != m_backup->Enqueue(CONTROL_FILE)) {
//         cout << "enqueue control file failed" << endl;
//     }

//     if (BackupRetCode::SUCCESS != m_backup->Start()) {
//         cout << "Start Backup failed" << endl;
//     }

//     BackupPhaseStatus backupStatus;
//     BackupStats stats;
//     do {
//         backupStatus = m_backup->GetStatus();
//         stats = m_backup->GetStats();

//         if (backupStatus == BackupPhaseStatus::COMPLETED) {
//             cout << "backup complete" << endl;
//             break;
//         } else if (backupStatus == BackupPhaseStatus::FAILED) {
//             cout << "backup failed" << endl;
//             break;
//         } else if (backupStatus == BackupPhaseStatus::INPROGRESS) {
//         }
//         sleep(1);
//     } while (true);

//     m_backup.reset();

//     EXPECT_EQ(backupStatus, BackupPhaseStatus::COMPLETED);
// }