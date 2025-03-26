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
#include "AntiRansomware.h"
#include "ParserUtils.h"
#include "log/Log.h"
#include "FSBackupUtils.h"
#include "IOEngines.h"

using namespace std;
using namespace FS_Backup;

AntiRansomware::AntiRansomware(const BackupParams& backupParams) : Backup(backupParams)
{
    CreateBackupStatistic();
    CreateBackupQueue();
    CreateBackupEngine(backupParams);
}

AntiRansomware::~AntiRansomware()
{
    m_controlInfo.reset();
    m_blockBufferMap.reset();
    m_controlFileReader.reset();
    m_reader.reset();
    m_writer.reset();
    m_readQueue.reset();
    m_writeQueue.reset();
    INFOLOG("Destruct AntiRansomware Instance");
}

/* Public APIs */
BackupRetCode AntiRansomware::Start()
{
    INFOLOG("AntiRansomware phase start!");
    if (m_controlFileReader->Start() != BackupRetCode::SUCCESS) {
        return BackupRetCode::FAILED;
    }
    if (m_reader->Start() != BackupRetCode::SUCCESS) {
        return BackupRetCode::FAILED;
    }
    if (m_writer->Start() != BackupRetCode::SUCCESS) {
        return BackupRetCode::FAILED;
    }

    return BackupRetCode::SUCCESS;
}

BackupRetCode AntiRansomware::Abort()
{
    INFOLOG("AntiRansomware phase abort!");
    if (m_abort) {
        return BackupRetCode::SUCCESS;
    }
    m_controlFileReader->Abort();
    m_reader->Abort();
    m_writer->Abort();
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode AntiRansomware::Destroy()
{
    return BackupRetCode::SUCCESS;
}

BackupRetCode AntiRansomware::Enqueue(string contrlFile)
{
    m_controlFileReader->Enqueue(contrlFile);
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus AntiRansomware::GetStatus()
{
    if (IsMemberNull()) {
        ERRLOG("Exit GetStatus. Member ptr null. Backup Failed");
        return BackupPhaseStatus::FAILED;
    }

    BackupPhaseStatus controlFileReaderStatus = m_controlFileReader->GetStatus();
    BackupPhaseStatus readerStatus = m_reader->GetStatus();
    BackupPhaseStatus writerStatus = m_writer->GetStatus();

    if (m_abort) {
        INFOLOG("controlFileReaderStatus: %u readerStatus: %u writerStatus: %u",
            (int)(controlFileReaderStatus), (int)readerStatus, (int)writerStatus);
        if (IsAborted(controlFileReaderStatus, readerStatus, writerStatus)) {
            INFOLOG("Exit GetStatus. Aborted");
            return BackupPhaseStatus::ABORTED;
        }
        INFOLOG("Exit GetStatus. Abort in progress");
        return BackupPhaseStatus::ABORT_INPROGRESS;
    }

    if (IsFailed(controlFileReaderStatus, readerStatus, writerStatus)) {
        return FSBackupUtils::GetFailureStatus(readerStatus, writerStatus);
    }

    DBGLOG("AntiRansomware check complete: %d, %d, %d faild: %d",
        m_controlInfo->m_controlReaderPhaseComplete.load(),
        m_controlInfo->m_readPhaseComplete.load(),
        m_controlInfo->m_writePhaseComplete.load(),
        m_controlInfo->m_failed.load());
    if (IsCompleted(controlFileReaderStatus, readerStatus, writerStatus)) {
        INFOLOG("Exit GetStatus. Backup completed");
        return BackupPhaseStatus::COMPLETED;
    }

    return BackupPhaseStatus::INPROGRESS;
}

BackupStats AntiRansomware::GetStats()
{
    BackupStats stats;
    stats.noOfDirToBackup   = m_controlInfo->m_noOfDirToBackup;
    stats.noOfFilesToBackup = m_controlInfo->m_noOfFilesToBackup;
    stats.noOfBytesToBackup = m_controlInfo->m_noOfBytesToBackup;
    stats.noOfDirToDelete   = m_controlInfo->m_noOfDirToDelete;
    stats.noOfFilesToDelete = m_controlInfo->m_noOfFilesToDelete;
    stats.noOfDirCopied     = m_controlInfo->m_noOfDirCopied;
    stats.noOfFilesCopied   = m_controlInfo->m_noOfFilesCopied;
    stats.noOfBytesCopied   = m_controlInfo->m_noOfBytesCopied;
    stats.noOfDirDeleted    = m_controlInfo->m_noOfDirDeleted;
    stats.noOfFilesDeleted  = m_controlInfo->m_noOfFilesDeleted;
    stats.noOfDirFailed     = m_controlInfo->m_noOfDirFailed;
    stats.noOfFilesFailed   = m_controlInfo->m_noOfFilesFailed;
    stats.startTime         = m_controlInfo->m_startTime;
    stats.noOfSrcRetryCount = m_controlInfo->m_noOfSrcRetryCount;
    stats.noOfDstRetryCount = m_controlInfo->m_noOfDstRetryCount;

    time_t timeElapsed = Module::ParserUtils::GetCurrentTimeInSeconds() - m_controlInfo->m_startTime;
    uint64_t elapsedTime = static_cast<uint64_t>(timeElapsed);
    if (elapsedTime != 0) {
        stats.backupspeed = (stats.noOfBytesCopied / elapsedTime);
    }

    INFOLOG("Backup Speed: %s, noOfDirToBackup: %lu, noOfFilesToBackup: %lu, noOfBytesToBackup: %lu, "
        "noOfDirCopied: %lu, noOfFilesCopied: %lu, noOfBytesCopied: %lu, noOfDirFailed: %lu, noOfFilesFailed: %lu",
        FSBackupUtils::FormatSpeed(stats.backupspeed).c_str(), stats.noOfDirToBackup, stats.noOfFilesToBackup,
        stats.noOfBytesToBackup, stats.noOfDirCopied, stats.noOfFilesCopied, stats.noOfBytesCopied,
        stats.noOfDirFailed, stats.noOfFilesFailed);

    return stats;
}

/* Private APIs */
void AntiRansomware::CreateBackupStatistic()
{
    m_controlInfo = make_shared<BackupControlInfo>();
}

void AntiRansomware::CreateBackupQueue()
{
    m_blockBufferMap = make_shared<BlockBufferMap>();
    BackupQueueConfig config;
    config.maxSize = DEFAULT_BACKUP_QUEUE_SIZE;
    config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
    m_readQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);

    return;
}

void AntiRansomware::CreateBackupEngine(const BackupParams& backupParams)
{
    m_backupParams = backupParams;

    AntiReaderParams antiReaderParams {};
    FillReaderParams(antiReaderParams);

    AntiWriterParams antiWriterParams {};
    FillWriterParams(antiWriterParams);

    Module::ThreadPoolFactory::InitThreadPool(Module::DEFAULT_THREADS_CNT, Module::DEFAULT_THREADS_CNT);

    if (dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->backupAntiType ==
        BackupAntiType::WORM) {
        m_reader = make_unique<NfsWormReader>(antiReaderParams);
    }

    if (dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.dstAdvParams)->backupAntiType ==
        BackupAntiType::WORM) {
        m_writer = make_unique<NfsWormWriter>(antiWriterParams);
    }
    m_controlFileReader = make_unique<AntiControlFileReader>(m_backupParams, m_readQueue, m_writeQueue,
                                                             m_controlInfo, m_blockBufferMap);

    return;
}

bool AntiRansomware::IsMemberNull() const
{
    if (m_controlFileReader == nullptr || m_reader == nullptr || m_writer == nullptr) {
        return true;
    }
    return false;
}

bool AntiRansomware::IsCompleted(const BackupPhaseStatus &controlFileReaderStatus,
    const BackupPhaseStatus &readerStatus, const BackupPhaseStatus &writerStatus) const
{
    if (controlFileReaderStatus == BackupPhaseStatus::COMPLETED &&
        readerStatus == BackupPhaseStatus::COMPLETED &&
        writerStatus == BackupPhaseStatus::COMPLETED) {
        return true;
    }
    return false;
}

bool AntiRansomware::IsFailed(const BackupPhaseStatus &controlFileReaderStatus,
    const BackupPhaseStatus &readerStatus, const BackupPhaseStatus &writerStatus) const
{
    /* If any of reader/writer/controlReader failed, should wait for all to be either
     * failed or completed. */
    if (IsStatusFailed(controlFileReaderStatus) && IsStatusFailed(writerStatus) && IsStatusFailed(readerStatus)) {
        return true;
    }

    return false;
}

bool AntiRansomware::IsAborted(const BackupPhaseStatus &controlFileReaderStatus,
    const BackupPhaseStatus &readerStatus, const BackupPhaseStatus &writerStatus) const
{
    if (controlFileReaderStatus == BackupPhaseStatus::ABORTED &&
        readerStatus == BackupPhaseStatus::ABORTED &&
        writerStatus == BackupPhaseStatus::ABORTED) {
        return true;
    }

    return false;
}

bool AntiRansomware::IsStatusFailed(const BackupPhaseStatus &antiStatus) const
{
    if  (antiStatus == BackupPhaseStatus::FAILED || antiStatus == BackupPhaseStatus::FAILED_NOACCESS ||
         antiStatus == BackupPhaseStatus::FAILED_NOSPACE ||
         antiStatus == BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE ||
         antiStatus == BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE) {
        return true;
    }
    return false;
}

void AntiRansomware::FillReaderParams(AntiReaderParams &antiReaderParams) const
{
    antiReaderParams.backupParams = m_backupParams;
    antiReaderParams.readQueuePtr = m_readQueue;
    antiReaderParams.writeQueuePtr = m_writeQueue;
    antiReaderParams.controlInfo = m_controlInfo;
    antiReaderParams.blockBufferMap = m_blockBufferMap;
}

void AntiRansomware::FillWriterParams(AntiWriterParams &antiWriterParams) const
{
    antiWriterParams.backupParams = m_backupParams;
    antiWriterParams.writeQueuePtr = m_writeQueue;
    antiWriterParams.readQueuePtr = m_readQueue;
    antiWriterParams.controlInfo = m_controlInfo;
    antiWriterParams.blockBufferMap = m_blockBufferMap;
}

// Hint:: remove later
std::unordered_set<FailedRecordItem, FailedRecordItemHash> AntiRansomware::GetFailedDetails()
{
    std::vector<FileHandle> readerFailedList;
    std::vector<FileHandle> writerFailedList;
    std::unordered_set<FailedRecordItem, FailedRecordItemHash> totalFailedRecords;
    if (m_reader) {
        readerFailedList = m_reader->GetFailedList();
    }
    if (m_writer) {
        writerFailedList = m_writer->GetFailedList();
    }
    for (auto& fileHandle : readerFailedList) {
        FailedRecordItem item;
        item.metaIndex = fileHandle.m_file->m_metaFileIndex;
        item.offset = fileHandle.m_file->m_metaFileOffset;
        item.errNum= fileHandle.m_errNum;
        totalFailedRecords.insert(item);
    }
    for (auto& fileHandle : writerFailedList) {
        FailedRecordItem item;
        item.metaIndex = fileHandle.m_file->m_metaFileIndex;
        item.offset = fileHandle.m_file->m_metaFileOffset;
        item.errNum= fileHandle.m_errNum;
        totalFailedRecords.insert(item);
    }
    return totalFailedRecords;
}