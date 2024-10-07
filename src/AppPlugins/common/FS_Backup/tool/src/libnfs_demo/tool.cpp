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
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include "BackupQueue.h"
#include "Backup.h"
#include "BackupMgr.h"
#include "log/Log.h"
#include "Path.h"
#include "ThreadPoolFactory.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
    constexpr auto CONTROL_FILE = "/home/shuai/NaveenScan/ctrl/control_0_5a7233256ccc000.txt";
    constexpr auto META_FILE_PATH = "/home/shuai/NaveenScan/meta/latest";
    constexpr auto SRC_ROOT_PATH = "/Ajo_New";     // Source share path
    constexpr auto DST_ROOT_PATH = "/Ajo_R";       // Dest share path
    constexpr auto MODULE = "BACKUP_DEMO";
}

const string LOG_NAME = "backup_demo.log";
const string LOG_PATH = "/home/shuai/NaveenBackup/log";
static const int procNum = 20;
void Producer();
void Consumer();
void PrintBackupStatus(BackupPhaseStatus status);
void FillSmbBackupParams(BackupParams& backupParams);

void InitLog(int argc, char** argv)
{
    CPath::GetInstance().Init(argv[0]);
    int iLogLevel = 0;
    int iLogCount = 100;
    int iLogMaxSize = 10;
    unsigned int runShellType = 1;
    CLogger::GetInstance().Init(LOG_NAME.c_str(), LOG_PATH);
    CLogger::GetInstance().SetLogConf(iLogLevel, iLogCount, iLogMaxSize);
    cout << "Log path: " << LOG_PATH << endl;
    return;
}

int main(int argc, char** argv)
{
    ThreadPoolFactory::InitThreadPool(32, 32);
    InitLog(argc, argv);
    unique_ptr<Backup> m_backup = nullptr;
    BackupParams params {};
    FillSmbBackupParams(params);
    m_backup = BackupMgr::CreateBackupInst(params);

    if (m_backup == nullptr) {
        cout << "back up init fail" << endl;
        return 0;
    }

    if (BackupRetCode::SUCCESS != m_backup->Enqueue(CONTROL_FILE)) {
        cout << "enqueue control file failed" << endl;
        return 0;
    }

    if (BackupRetCode::SUCCESS != m_backup->Start()) {
        cout << "Start Backup failed" << endl;
        return 0;
    }

    BackupPhaseStatus m_backupStatus;

    do {
        m_backupStatus = m_backup->GetStatus();
        PrintBackupStatus(m_backupStatus);

        if (m_backupStatus == BackupPhaseStatus::COMPLETED) {
            m_backup->GetStats();
            cout << "backup is complete" << endl;
            m_backup->Destroy();
            m_backup.reset();
            break;
        }
        if (m_backupStatus == BackupPhaseStatus::INPROGRESS) {
            cout << "backup is in progress" << endl;
        }
        sleep(10);
    } while (true);

    // BackupQueueConfig config {};
    // BackupQueue<string> backupQueue = BackupQueue<string>(config);
    // backupQueue.SetSize(6);
    // thread producerThread(Producer);
    // thread consumerThread(Consumer);
    // producerThread.join();
    // consumerThread.join();
    return 0;
}

void Producer()
{
    // for (int i = 0; i < procNum; i++) {
    //     // 1s push
    //     sleep(1);
    //     string s = "string" + to_string(i);
    //     cout << "Push value : " << s << endl;
    //     backupQueue.WaitAndPush(s);
    // }
}

void Consumer()
{
    // static int cnt = 60;
    // // 10s之后才pop
    // sleep(10);
    // while (true) {
    //     string value;
    //     // 超时3s
    //     backupQueue.WaitAndPop(value, 7000);
    //     if (value.empty()) {
    //         cout << "timeout" << endl;
    //     } else {
    //         cout << "get value : " << value << endl;
    //     }
    //     // sleep(1);
    //     if (++cnt == procNum) {
    //         break;
    //     }
    // }
}

void FillSmbBackupParams(BackupParams& backupParams)
{
    backupParams.backupType = BackupType::BACKUP_FULL;
    backupParams.srcEngine = BackupIOEngine::LIBSMB;
    backupParams.dstEngine = BackupIOEngine::LIBSMB;

    LibsmbBackupAdvanceParams libnfsBackupAdvanceParams {};
    backupParams.srcAdvParams = make_shared<LibsmbBackupAdvanceParams>(libnfsBackupAdvanceParams);
    backupParams.dstAdvParams = make_shared<LibsmbBackupAdvanceParams>(libnfsBackupAdvanceParams);
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.srcAdvParams)->server = "192.168.99.85";
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.srcAdvParams)->share = "/Naveen_V5_1M_4MB__1M_5MB_Files";
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.srcAdvParams)->version = "3.0";
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.srcAdvParams)->encryption = false;
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.srcAdvParams)->sign = false;
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.srcAdvParams)->timeout = 60;
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.srcAdvParams)->authType = "ntlmssp";
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.srcAdvParams)->user = "test_admin";
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.srcAdvParams)->password = "Huawei@123";

    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.dstAdvParams)->server = "192.168.130.170";
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.dstAdvParams)->share = "/cifs_backup_20221213164013";
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.dstAdvParams)->version = "3.0";
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.dstAdvParams)->encryption = false;
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.dstAdvParams)->sign = false;
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.dstAdvParams)->timeout = 60;
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.dstAdvParams)->authType = "ntlmssp";
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.dstAdvParams)->rootPath = "";
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.dstAdvParams)->serverCheckMaxCount = 100;
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.dstAdvParams)->serverCheckSleepTime = 30;
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.dstAdvParams)->serverCheckRetry = 3;
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.dstAdvParams)->user = "zs_cifs";
    dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupParams.dstAdvParams)->password = "Huawei@123";

    CommonParams commonParams {};
    commonParams.maxBufferCnt = 2000;
    commonParams.maxBufferSize = 52428800; // 10kb
    commonParams.maxErrorFiles = 100;
    commonParams.blockSize = 128 * 1024;
    commonParams.backupDataFormat = BackupDataFormat::NATIVE;
    commonParams.restoreReplacePolicy = RestoreReplacePolicy::NONE;
    commonParams.jobId = "jobId";
    commonParams.subJobId = "subJobId";
    backupParams.commonParams = commonParams;

    ScanAdvanceParams scanAdvParams {};
    scanAdvParams.metaFilePath = META_FILE_PATH;
    backupParams.scanAdvParams = scanAdvParams;

    backupParams.backupType = BackupType::BACKUP_FULL;
    backupParams.phase = BackupPhase::COPY_STAGE;
}

void PrintBackupStatus(BackupPhaseStatus status)
{
    char enumItemStr[][50] = {"", "INPROGRESS", "ABORT_INPROGRESS", "ABORTED", "FAILED", "COMPLETED", "FAILED_NOACCESS",
                              "FAILED_NOSPACE", "FAILED_SEC_SERVER_NOTREACHABLE", "FAILED_PROT_SERVER_NOTREACHABLE"};
    int idx = static_cast<int>(status);
    if (idx >= sizeof(enumItemStr) / sizeof(enumItemStr[0])) {
        std::cout << "Error status\n";
        return;
    }
    DBGLOG("backup status : %s", enumItemStr[idx]);
}
