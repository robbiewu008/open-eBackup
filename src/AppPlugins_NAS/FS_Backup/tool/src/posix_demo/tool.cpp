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
#include <chrono>
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
    std::string g_scanOutDir = "";
    std::string g_srcPath = "";
    std::string g_dstPath = "/home/backup_dst";
    std::string g_logPath = "/home/backup_dst";
    const string LOG_NAME = "backup_demo.log";
    constexpr auto MODULE = "BACKUP_DEMO";
    const uint64_t KILOBYTE = 1024;
    const uint64_t MEGABYTE = 1024 * 1024;
    const uint64_t GIGABYTE = 1024 * 1024 * 1024;
    const uint64_t TARABYTE = 0x010000000000;
}

void FillBackupParams(BackupParams& backupParams);
unsigned long GetTimestamp();
void PrintStat(const BackupStats& stats, unsigned long startTime);

void InitLog(int argc, char** argv)
{
    int logLevel = 0;
    int logCount = 100;
    int logMaxSize = 100;
    unsigned int runShellType = 1;
    CPath::GetInstance().Init(argv[0]);
    CLogger::GetInstance().Init(LOG_NAME.c_str(), g_logPath, logLevel, logCount, logMaxSize);
    CLogger::GetInstance().SetLogConf(logLevel, logCount, logMaxSize);
    cout << "Log path: " << g_logPath << endl;
    return;
}

int ParsePara(int argc, char** argv)
{
    if (argc < 5) {
        std::cout << "Miss params, usage: backup_tool -c <scan_out_path> -s <src> [-d <dst>] [-l <logpath>]\n";
        return -1;
    }

    for (int i = 1; (i < argc) && (i + 1 < argc); ++i) {
        if (std::string(argv[i]) == "-c") {        // control & meta file
            g_scanOutDir = std::string(argv[i + 1]);
            ++i;
        } else if (std::string(argv[i]) == "-s") { // src path that to be backup
            g_srcPath = std::string(argv[i + 1]);
            ++i;
        } else if (std::string(argv[i]) == "-d") { // backup destination
            g_dstPath = std::string(argv[i + 1]);
            ++i;
        } else if (std::string(argv[i]) == "-l") { // log path
            g_logPath = std::string(argv[i + 1]);
            ++i;
        } else {
            std::cout << "Error para" << endl;
            return -1;
        }
    }

    if (g_scanOutDir.empty() || g_srcPath.empty()) {
        std::cout << "Must input contol&meta file and the src file.\n";
        return -1;
    }

    return 0;
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

int main(int argc, char** argv)
{
    if (ParsePara(argc, argv) != 0) {
        return 0;
    }
    InitLog(argc, argv);

    ThreadPoolFactory::InitThreadPool(32, 32);
    BackupParams params {};
    FillBackupParams(params);
    unique_ptr<Backup> m_backup = BackupMgr::CreateBackupInst(params);
    if (m_backup == nullptr) {
        cout << "back up init fail" << endl;
        return 0;
    }

    std::string ctrlFile = g_scanOutDir + "ctrl";
    if (BackupRetCode::SUCCESS != m_backup->Enqueue(ctrlFile)) {
        cout << "enqueue control file failed" << endl;
        return 0;
    }

    if (BackupRetCode::SUCCESS != m_backup->Start()) {
        cout << "Start Backup failed" << endl;
        return 0;
    }
    BackupPhaseStatus backupStatus;
    BackupStats stats;
    unsigned long startTime = GetTimestamp();

    do {
        backupStatus = m_backup->GetStatus();
        stats = m_backup->GetStats();
        PrintBackupStatus(backupStatus);

        if (backupStatus == BackupPhaseStatus::COMPLETED) {
            PrintStat(stats, startTime);
            cout << "backup complete" << endl;
            break;
        }
        if (backupStatus == BackupPhaseStatus::FAILED) {
            cout << "backup failed" << endl;
            break;
        }
        if (backupStatus == BackupPhaseStatus::INPROGRESS) {
            PrintStat(stats, startTime);
        }
        sleep(1);
    } while (true);

    return 0;
}

void FillBackupParams(BackupParams& backupParams)
{
    std::string metaFile = g_scanOutDir + "/meta/latest";
    backupParams.srcEngine = BackupIOEngine::POSIX;
    backupParams.dstEngine = BackupIOEngine::POSIX;

    HostBackupAdvanceParams posixBackupAdvanceParams {};
    backupParams.srcAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);
    backupParams.dstAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);
    dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->dataPath = g_srcPath;
    dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.dstAdvParams)->dataPath = g_dstPath;

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
    scanAdvParams.metaFilePath = metaFile;
    backupParams.scanAdvParams = scanAdvParams;

    backupParams.backupType = BackupType::BACKUP_FULL;
    backupParams.phase = BackupPhase::COPY_STAGE;
}

unsigned long GetTimestamp()
{
    return chrono::system_clock::now().time_since_epoch().count()/chrono::system_clock::period::den;
}

string FormatDouble(double d)
{
    char cstr[1024] = { 0 };
    sprintf(cstr, "%.2f", d);
    string str(cstr);
    return str;
}

string FormatBytes(uint64_t bytes)
{
    double d = bytes;
    string unit = "Byte";
    if ((bytes / TARABYTE) > 0) {
        d = (double)bytes / (double)TARABYTE;
        unit = "TB";
    } else if ((bytes / GIGABYTE) > 0) {
        d = (double)bytes / (double)GIGABYTE;
        unit = "GB";
    } else if ((bytes / MEGABYTE) > 0) {
        d = (double)bytes / (double)MEGABYTE;
        unit = "MB";
    } else if ((bytes / KILOBYTE) > 0) {
        d = (double)bytes / (double)KILOBYTE;
        unit = "KB";
    }
    return FormatDouble(d) + " " + unit;
}

void PrintStat(const BackupStats& stats, unsigned long startTime)
{
    string copied = FormatBytes(stats.noOfBytesCopied);
    string total = FormatBytes(stats.noOfBytesToBackup);
    double percent = (double)(stats.noOfBytesCopied * 100) / (double)stats.noOfBytesToBackup;
    unsigned long timeElapse = GetTimestamp() - startTime;

    cout << "copied: " << copied << "    total: " << total
        << "    percent " << FormatDouble(percent) << "%" << "    duration(s): " << timeElapse << endl;
}
