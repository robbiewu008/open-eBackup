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
#include <chrono>
#include "BackupQueue.h"
#include "Backup.h"
#include "BackupMgr.h"
#include "common/Thread.h"
#include "log/Log.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
    const string LOG_NAME = "backup_demo.log";
    const string LOG_PATH = R"(D:\\version\\1.3.0\\win_backup_dev\\log)";
    const string CONTROL_FILE = R"(D:\\version\\1.3.0\\win_backup_dev\\scan\\ctrl\\control_0_18ab649b0dde8000.txt)";
    const string META_FILE_PATH = R"(D:\\version\\1.3.0\\win_backup_dev\\scan\\meta\\latest)";
    const string SRC_ROOT_PATH = "";
    const string DST_ROOT_PATH = R"(D:\version\1.3.0\win_backup_dev\dst)";
    const string MODULE = "BACKUP_DEMO";
}

void FillBackupParams(BackupParams& backupParams);
void PrintBackupStatus(BackupPhaseStatus status);

void InitLog()
{
    int logLevel = 0;
    int logCount = 100;
    int logMaxSize = 30;
    CLogger::GetInstance().Init(LOG_NAME.c_str(), LOG_PATH, logLevel, logCount, logMaxSize);
    cout << "Log path: " << LOG_PATH << endl;
    return;
}

int main(int argc, char** argv)
{
    InitLog();
    DBGLOG("Get DmeClient Ins failed. jobId=%d.", 123);
    unique_ptr<Backup> m_backup = nullptr;
    BackupParams params{};
    FillBackupParams(params);
    m_backup = BackupMgr::CreateBackupInst(params);

    if (m_backup == nullptr) {
        cout << "back up init fail" << endl;
        return 0;
    }

    if (BackupRetCode::SUCCESS != m_backup->Enqueue(CONTROL_FILE)) {
        cout << "enqueue control file failed" << endl;
        return 0;
    }

    time_t startTime = time(NULL);
    if (BackupRetCode::SUCCESS != m_backup->Start()) {
        cout << "Start Backup failed" << endl;
        return 0;
    }
    BackupPhaseStatus backupStatus;
    BackupStats stats;

    do {
        backupStatus = m_backup->GetStatus();
        stats = m_backup->GetStats();
        PrintBackupStatus(backupStatus);

        if (backupStatus == BackupPhaseStatus::COMPLETED) {
            time_t endTime = time(NULL);
            cout << "backup complete" << endl;
			cout << "total cast " << endTime - startTime << " s" << endl;
            break;
        }
        else if (backupStatus == BackupPhaseStatus::FAILED) {
            cout << "backup failed" << endl;
            break;
        }
        else if (backupStatus == BackupPhaseStatus::INPROGRESS) {
            cout << "backup is in progress" << endl;
        }
        Module::SleepFor(std::chrono::seconds(1));
    } while (true);

    return 0;
}

void FillBackupParams(BackupParams& backupParams)
{
    backupParams.srcEngine = BackupIOEngine::WIN32_IO;
    backupParams.dstEngine = BackupIOEngine::WIN32_IO;

    HostBackupAdvanceParams win32BackupAdvanceParams{};
    backupParams.srcAdvParams = make_shared<HostBackupAdvanceParams>(win32BackupAdvanceParams);
    backupParams.dstAdvParams = make_shared<HostBackupAdvanceParams>(win32BackupAdvanceParams);
    dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->dataPath = "";
    dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.dstAdvParams)->dataPath = DST_ROOT_PATH;

    CommonParams commonParams{};
    commonParams.maxBufferCnt = 10;
    commonParams.maxBufferSize = 1024 * 1024; // 1MB
    commonParams.maxErrorFiles = 100;
    commonParams.backupDataFormat = BackupDataFormat::NATIVE;
    commonParams.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    commonParams.jobId = "jobId";
    commonParams.subJobId = "subJobId";
    commonParams.skipFailure = true; // stop backup if any item backup failed.
    backupParams.commonParams = commonParams;

    ScanAdvanceParams scanAdvParams{};
    scanAdvParams.metaFilePath = META_FILE_PATH;
    backupParams.scanAdvParams = scanAdvParams;

    backupParams.backupType = BackupType::BACKUP_FULL;
    backupParams.phase = BackupPhase::COPY_STAGE;
}

void PrintBackupStatus(BackupPhaseStatus status)
{
    switch (status) {
        case BackupPhaseStatus::INPROGRESS:
        {
            DBGLOG("backup status : INPROGRESS");
            break;
        }
        case BackupPhaseStatus::ABORT_INPROGRESS:
        {
            DBGLOG("backup status : ABORT_INPROGRESS");
            break;
        }
        case BackupPhaseStatus::ABORTED:
        {
            DBGLOG("backup status : ABORTED");
            break;
        }
        case BackupPhaseStatus::FAILED:
        {
            DBGLOG("backup status : FAILED");
            break;
        }
        case BackupPhaseStatus::COMPLETED:
        {
            DBGLOG("backup status : COMPLETED");
            break;
        }
        case BackupPhaseStatus::FAILED_NOACCESS:
        {
            DBGLOG("backup status : FAILED_NOACCESS");
            break;
        }
        case BackupPhaseStatus::FAILED_NOSPACE:
        {
            DBGLOG("backup status : FAILED_NOSPACE");
            break;
        }
        case BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE:
        {
            DBGLOG("backup status : FAILED_SEC_SERVER_NOTREACHABLE");
            break;
        }
        case BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE:
        {
            DBGLOG("backup status : FAILED_PROT_SERVER_NOTREACHABLE");
            break;
        }
    }
}
