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
#include "VolumeProtector.h"
#include "common/VolumeProtectMacros.h"
#include "task/VolumeBackupTask.h"
#include "task/VolumeRestoreTask.h"
#include "common/VolumeUtils.h"
#include "native/FileSystemAPI.h"

using namespace volumeprotect;
using namespace volumeprotect::common;
using namespace volumeprotect::task;
using namespace volumeprotect::fsapi;

namespace {
    constexpr auto VOLUME_NAME_LEN_MAX = 260;
    constexpr unsigned int BACKUP_LOG_COUNT = 100;
    constexpr unsigned int BACKUP_LOG_MAX_SIZE = 30;
}

/**
 * @brief mapping from TaskStatus enum to literal
 */
static std::unordered_map<int, std::string> g_statusStringTable {
    { static_cast<int>(TaskStatus::INIT), "INIT" },
    { static_cast<int>(TaskStatus::RUNNING), "RUNNING" },
    { static_cast<int>(TaskStatus::SUCCEED), "SUCCEED" },
    { static_cast<int>(TaskStatus::ABORTING), "ABORTING" },
    { static_cast<int>(TaskStatus::ABORTED), "ABORTED" },
    { static_cast<int>(TaskStatus::FAILED), "FAILED" },
};

/*
 * Backup copy folder hierarchy (Full/Synethetic Copy):
 * Example1: a copy with format "CopyFormat::BIN" and copyName "backupvolume" with 3 session
 *  It should have fragment copy data files and corresponding checksum meta files with same nums of sessions
 *  ${CopyID}
 *       |
 *      ${Volume UUID}
 *          |------data
 *          |       |------backupvolume.copydata.bin.part1
 *          |       |------backupvolume.copydata.bin.part2
 *          |       |------backupvolume.copydata.bin.part3
 *          |
 *          |------meta
 *                  |------volumecopy.meta.json
 *                  |------backupvolume.1.meta.bin
 *                  |------backupvolume.2.sha256.meta.bin
 *                  |------backupvolume.3.sha256.meta.bin
 *
 * Example2: a copy with format "CopyFormat::IMAGE" and copyName "raspberry" with 2 session
 *  It should have only one data file and 2 checksum files
 *  ${CopyID}
 *       |
 *      ${Volume UUID}
 *          |------data
 *          |       |------raspberry.copydata.img
 *          |
 *          |------meta
 *                  |------volumecopy.meta.json
 *                  |------raspberry.1.meta.bin
 *                  |------raspberry.2.sha256.meta.bin
 *
 * For Windows OS, *.vhd, *.vhdx are also supported and it's handled similar to image format
 * volumecopy.meta.json saves meta data (format, sessions) of the copy and it's critical for the copy to mount/restore
 */

std::unique_ptr<VolumeProtectTask> VolumeProtectTask::BuildBackupTask(const VolumeBackupConfig& backupConfig)
{
    // fill missing BackupConfig fields
    VolumeBackupConfig finalBackupConfig = backupConfig;
    if (finalBackupConfig.copyName.empty() || VOLUME_NAME_LEN_MAX < finalBackupConfig.copyName.length()) {
        namespace chrono = std::chrono;
        using clock = std::chrono::system_clock;
        auto timestamp = std::chrono::duration_cast<chrono::microseconds>(clock::now().time_since_epoch()).count();
        finalBackupConfig.copyName = std::to_string(timestamp);
        WARNLOG("invalid copy name %s, generate new copyname %s",
            backupConfig.copyName.c_str(), finalBackupConfig.copyName.c_str());
    }

    // 2. check volume size
    uint64_t volumeSize = 0;
    try {
        volumeSize = fsapi::ReadVolumeSize(backupConfig.volumePath, backupConfig.copyFormat);
    } catch (const SystemApiException& e) {
        ERRLOG("retrive volume size got exception: %s", e.what());
        return nullptr;
    }
    if (volumeSize == 0) { // invalid volume
        return nullptr;
    }

    // 3. check dir existence
    if (!fsapi::IsDirectoryExists(backupConfig.outputCopyDataDirPath) ||
        !fsapi::IsDirectoryExists(backupConfig.outputCopyMetaDirPath) ||
        (backupConfig.backupType == BackupType::FOREVER_INC &&
        !fsapi::IsDirectoryExists(backupConfig.prevCopyMetaDirPath))) {
        ERRLOG("failed to prepare copy directory");
        return nullptr;
    }

    return std::unique_ptr<VolumeProtectTask>(new VolumeBackupTask(finalBackupConfig, volumeSize));
}

std::unique_ptr<VolumeProtectTask> VolumeProtectTask::BuildRestoreTask(const VolumeRestoreConfig& restoreConfig)
{
    // 1. check volume size
    uint64_t volumeSize = 0;
    try {
        volumeSize = fsapi::ReadVolumeSize(restoreConfig.volumePath);
    } catch (const SystemApiException& e) {
        ERRLOG("retrive volume size got exception: %s", e.what());
        return nullptr;
    }
    if (volumeSize == 0) { // invalid volume
        return nullptr;
    }

    // 2. check dir existence
    if (!fsapi::IsDirectoryExists(restoreConfig.copyDataDirPath) ||
        !fsapi::IsDirectoryExists(restoreConfig.copyMetaDirPath)) {
        ERRLOG("restore copy directory not prepared");
        return nullptr;
    }

    // 3. read copy meta json and validate
    VolumeCopyMeta volumeCopyMeta {};
    if (!common::ReadVolumeCopyMeta(restoreConfig.copyMetaDirPath, restoreConfig.copyName, volumeCopyMeta)) {
        ERRLOG("failed to read copy meta json from dir: %s", restoreConfig.copyMetaDirPath.c_str());
        return nullptr;
    }
    if (volumeSize != volumeCopyMeta.volumeSize) {
        WARNLOG("restore volume size mismatch ! (copy : %llu, target: %llu)", volumeCopyMeta.volumeSize, volumeSize);
    }
    if (volumeSize < volumeCopyMeta.volumeSize) {
        ERRLOG("restore volume size illegal ! (copy : %llu, target: %llu)", volumeCopyMeta.volumeSize, volumeSize);
        return nullptr;
    }

    return std::unique_ptr<VolumeProtectTask>(new VolumeRestoreTask(restoreConfig, volumeCopyMeta));
}

void StatefulTask::Abort()
{
    m_abort = true;
    if (m_status == TaskStatus::INIT) {
        m_status = TaskStatus::ABORTED;
        return;
    }
    if (IsTerminated()) {
        return;
    }
    m_status = TaskStatus::ABORTING;
}

TaskStatus StatefulTask::GetStatus() const
{
    return m_status;
}

bool StatefulTask::IsFailed() const
{
    return m_status == TaskStatus::FAILED;
}

bool StatefulTask::IsTerminated() const
{
    return (
        m_status == TaskStatus::SUCCEED ||
        m_status == TaskStatus::ABORTED ||
        m_status == TaskStatus::FAILED);
}

std::string StatefulTask::GetStatusString() const
{
    return g_statusStringTable[static_cast<int>(m_status)];
}

void StatefulTask::AssertTaskNotStarted()
{
    (m_status != TaskStatus::INIT) ? throw std::runtime_error("task already started") : void();
}

ErrCodeType StatefulTask::GetErrorCode() const
{
    return m_errorCode;
}

TaskStatistics TaskStatistics::operator + (const TaskStatistics& statistic) const
{
    TaskStatistics res;
    res.bytesToRead     = statistic.bytesToRead + this->bytesToRead;
    res.bytesRead       = statistic.bytesRead + this->bytesRead;
    res.blocksToHash    = statistic.blocksToHash + this->blocksToHash;
    res.blocksHashed    = statistic.blocksHashed + this->blocksHashed;
    res.bytesToWrite    = statistic.bytesToWrite + this->bytesToWrite;
    res.bytesWritten    = statistic.bytesWritten + this->bytesWritten;
    return res;
}

// implement C style interface ...
inline static std::string StringFromCStr(char* str)
{
    return str == nullptr ? std::string("") : std::string(str);
}

void InitLogs(const char* fullLogPath, int logLevel)
{
    unsigned int iLogLevel = static_cast<unsigned int>(logLevel);
    unsigned int iLogCount = BACKUP_LOG_COUNT;
    unsigned int iLogMaxSize = BACKUP_LOG_MAX_SIZE;
    std::string backupLogName = "backup.log";
    Module::CLogger::GetInstance().Init(backupLogName.c_str(),
        fullLogPath,
        iLogLevel,
        iLogCount,
        iLogMaxSize);
}

void* BuildBackupTask(VolumeBackupConf_C cBackupConf)
{
    VolumeBackupConfig backupConfig {};
    backupConfig.backupType = static_cast<BackupType>(cBackupConf.backupType);
    backupConfig.copyFormat = static_cast<CopyFormat>(cBackupConf.copyFormat);
    backupConfig.copyName = StringFromCStr(cBackupConf.copyName);
    backupConfig.volumePath = StringFromCStr(cBackupConf.volumePath);
    backupConfig.prevCopyMetaDirPath = StringFromCStr(cBackupConf.prevCopyMetaDirPath);
    backupConfig.outputCopyDataDirPath = StringFromCStr(cBackupConf.outputCopyDataDirPath);
    backupConfig.outputCopyMetaDirPath = StringFromCStr(cBackupConf.outputCopyMetaDirPath);
    backupConfig.blockSize = cBackupConf.blockSize;
    backupConfig.sessionSize = cBackupConf.sessionSize;
    backupConfig.hasherNum = cBackupConf.hasherNum;
    backupConfig.hasherEnabled = cBackupConf.hasherEnabled;
    backupConfig.enableCheckpoint = cBackupConf.enableCheckpoint;
    std::unique_ptr<VolumeProtectTask> task = VolumeProtectTask::BuildBackupTask(backupConfig);
    return reinterpret_cast<void*>(task.release());
}

void* BuildRestoreTask(VolumeRestoreConf_C cRestoreConf)
{
    VolumeRestoreConfig restoreConfig {};
    restoreConfig.copyName = StringFromCStr(cRestoreConf.copyName);
    restoreConfig.volumePath = StringFromCStr(cRestoreConf.volumePath);
    restoreConfig.copyDataDirPath = StringFromCStr(cRestoreConf.copyDataDirPath);
    restoreConfig.copyMetaDirPath = StringFromCStr(cRestoreConf.copyMetaDirPath);
    restoreConfig.enableCheckpoint = cRestoreConf.enableCheckpoint;
    std::unique_ptr<VolumeProtectTask> task = VolumeProtectTask::BuildRestoreTask(restoreConfig);
    return reinterpret_cast<void*>(task.release());
}

bool StartTask(void* task)
{
    return reinterpret_cast<VolumeProtectTask*>(task)->Start();
}

void DestroyTask(void* task)
{
    delete reinterpret_cast<VolumeProtectTask*>(task);
}

TaskStatistics_C GetTaskStatistics(void* task)
{
    TaskStatistics statistic = reinterpret_cast<VolumeProtectTask*>(task)->GetStatistics();
    TaskStatistics_C cstat;
    if (::memset_s(&cstat, sizeof(TaskStatistics_C), 0, sizeof(TaskStatistics_C)) != 0) {
        WARNLOG("memset error");
    }
    cstat.blocksHashed = statistic.blocksHashed;
    cstat.blocksToHash = statistic.blocksToHash;
    cstat.bytesRead = statistic.bytesRead;
    cstat.bytesToRead = statistic.bytesToRead;
    cstat.bytesToWrite = statistic.bytesToWrite;
    cstat.bytesWritten = statistic.bytesWritten;
    return cstat;
}

void AbortTask(void* task)
{
    reinterpret_cast<VolumeProtectTask*>(task)->Abort();
}

TaskStatus_C GetTaskStatus(void* task)
{
    TaskStatus taskStatus = reinterpret_cast<VolumeProtectTask*>(task)->GetStatus();
    return static_cast<TaskStatus_C>(taskStatus);
}

bool IsTaskFailed(void* task)
{
    return reinterpret_cast<VolumeProtectTask*>(task)->IsFailed();
}

bool IsTaskTerminated(void* task)
{
    return reinterpret_cast<VolumeProtectTask*>(task)->IsTerminated();
}