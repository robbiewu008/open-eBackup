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
#ifndef BACKUP_COMMON_SERVICE_BASE_H
#define BACKUP_COMMON_SERVICE_BASE_H

#include <string>
#include <memory>
#include <unordered_map>

#include "CommonServiceParams.h"
#include "WriterBase.h"
#include "ReaderBase.h"
#include "ParserUtils.h"
#include "StreamHostFilePendingMap.h"

using namespace FS_Backup;
using namespace Module;

/*
 * Backup four stage (Copy, Dir, Delete, Hardlink) Abstraction
 */
template<typename ControlFileReaderType, typename AggregatorType>
class BackupServiceBase : public Backup {

public:
    explicit BackupServiceBase(const BackupParams& backupParams) : Backup(backupParams)
    {
        m_failureRecorder = std::make_shared<Module::BackupFailureRecorder>(
            backupParams.commonParams.failureRecordRootPath,
            backupParams.commonParams.jobId,
            backupParams.commonParams.subJobId,
            DEFAULT_MAX_FAILURE_RECORDS_BUFFER_SIZE,
            backupParams.commonParams.maxFailureRecordsNum
        );

        CreateBackupStatistic();
        CreateBackupQueue();
        CreateBackupEngine(backupParams);
    }

    /* only used for host backup */
    BackupServiceBase(
        const std::string&      source,
        const std::string&      destination,
        const std::string&      metaPath,
        bool                    writeMeta
        ) : Backup(source, destination, metaPath)
    {
        CreateBackupStatistic();
        CreateBackupQueue();
        CreateHostBackupEngine(source, destination, metaPath, writeMeta);
    }

    ~BackupServiceBase()
    {
        m_controlInfo.reset();
        m_blockBufferMap.reset();
        m_controlFileReader.reset();
        m_reader.reset();
        m_aggregator.reset();
        m_writer.reset();
        m_readQueue.reset();
        m_aggregateQueue.reset();
        m_writeQueue.reset();
        m_hardlinkMap.reset();
        INFOLOG("Destruct BackupServiceBase Instance");
    }

    /* implemnt public API defined in Backup */
    BackupRetCode Start() override
    {
        INFOLOG("BACKUP PHASE START");
        if (m_controlFileReader->Start() != BackupRetCode::SUCCESS) {
            return BackupRetCode::FAILED;
        }
        if (m_reader->Start() != BackupRetCode::SUCCESS) {
            return BackupRetCode::FAILED;
        }
        if (m_aggregator->Start() != BackupRetCode::SUCCESS) {
            return BackupRetCode::FAILED;
        }
        if (m_writer->Start() != BackupRetCode::SUCCESS) {
            return BackupRetCode::FAILED;
        }
        return BackupRetCode::SUCCESS;
    }

    // Hint:: This method always return BackupRetCode::SUCCESS
    BackupRetCode Abort() override
    {
        INFOLOG("Abort invoked!");
        if (m_abort) {
            return BackupRetCode::SUCCESS;
        }
        if (m_controlFileReader) {
            m_controlFileReader->Abort();
        }
        if (m_reader) {
            m_reader->Abort();
        }
        if (m_aggregator) {
            m_aggregator->Abort();
        }
        if (m_writer) {
            m_writer->Abort();
        }
        m_abort = true;
        return BackupRetCode::SUCCESS;
    }

    // always return BackupRetCode::SUCCESS except m_reader
    BackupRetCode Destroy() override
    {
        INFOLOG("Destory in progress!");
        if (m_controlFileReader) {
            // Hint:: Not implemented yet
        }
        if (m_reader) {
            auto ret = m_reader->Destroy();
            if (ret != BackupRetCode::SUCCESS) {
                ERRLOG("Destroy m_reader failed!");
                return ret;
            }
        }
        if (m_aggregator) {
            // Hint:: Not implemented yet
        }
        if (m_writer) {
            auto ret = m_writer->Destroy();
            if (ret != BackupRetCode::SUCCESS) {
                ERRLOG("Destroy m_writer failed!");
                return ret;
            }
        }
        return BackupRetCode::SUCCESS;
    }

    // Hint:: This method always return BackupRetCode::SUCCESS
    BackupRetCode Enqueue(std::string controlFile) override
    {
        INFOLOG("Backup phrase enqueue : %s", controlFile.c_str());
        m_controlFileReader->Enqueue(controlFile);
        return BackupRetCode::SUCCESS;
    }

    BackupPhaseStatus GetStatus() override
    {
        if (IsMemberNull()) {
            ERRLOG("Exit GetStatus. member ptr null. Backup Failed");
            return BackupPhaseStatus::FAILED;
        }

        BackupPhaseStatus controlFileReaderStatus = m_controlFileReader->GetStatus();
        BackupPhaseStatus readerStatus = m_reader->GetStatus();
        BackupPhaseStatus aggregatorStatus = m_aggregator->GetStatus();
        BackupPhaseStatus writerStatus = m_writer->GetStatus();

        INFOLOG("controlFileReaderStatus: %u readerStatus: %u aggregatorStatus: %u writerStatus: %u",
                (int)(controlFileReaderStatus), (int)readerStatus, (int)aggregatorStatus, (int)writerStatus);

        if (m_abort) {
            if (IsAborted(controlFileReaderStatus, readerStatus, aggregatorStatus, writerStatus)) {
                INFOLOG("Exit GetStatus. Aborted");
                return BackupPhaseStatus::ABORTED;
            } else {
                INFOLOG("Exit GetStatus. Abort in progress");
                return BackupPhaseStatus::ABORT_INPROGRESS;
            }
        }

        INFOLOG("check complete: %d, %d, %d, %d failed: %d",
            m_controlInfo->m_controlReaderPhaseComplete.load(),
            m_controlInfo->m_readPhaseComplete.load(),
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_controlInfo->m_writePhaseComplete.load(),
            m_controlInfo->m_failed.load());

        /* all writer/reader/aggreator completed without error  */
        if (IsCompleted(controlFileReaderStatus, readerStatus, aggregatorStatus, writerStatus)) {
            INFOLOG("Exit GetStatus. Backup completed");
            return BackupPhaseStatus::COMPLETED;
        }

        /* all writer/reader/aggreator completed or failed */
        if (IsFailedOrCompleted(controlFileReaderStatus, readerStatus, aggregatorStatus, writerStatus)) {
            return FSBackupUtils::GetFailureStatus(readerStatus, writerStatus);
        }

        return BackupPhaseStatus::INPROGRESS;
    }

    BackupStats GetStats() override
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
        stats.readConsume       = m_controlInfo->m_readConsume;
        stats.aggregateConsume  = m_controlInfo->m_aggregateConsume;
        stats.writerConsume     = m_controlInfo->m_writerConsume;

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

    std::unordered_set<FailedRecordItem, FailedRecordItemHash> GetFailedDetails() override
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
            item.filePath = fileHandle.m_file->m_fileName;
            totalFailedRecords.insert(item);
        }
        for (auto& fileHandle : writerFailedList) {
            FailedRecordItem item;
            item.metaIndex = fileHandle.m_file->m_metaFileIndex;
            item.offset = fileHandle.m_file->m_metaFileOffset;
            item.errNum= fileHandle.m_errNum;
            item.filePath = fileHandle.m_file->m_fileName;
            totalFailedRecords.insert(item);
        }
        INFOLOG("get failed list total: %u", totalFailedRecords.size());
        return totalFailedRecords;
    }

protected:
    void CreateBackupStatistic()
    {
        m_controlInfo = std::make_shared<BackupControlInfo>();
        m_controlInfo->m_streamHostFilePendingMap = std::make_shared<StreamHostFilePendingMap>();
    }

    void CreateBackupQueue()
    {
        BackupQueueConfig config;
        config.maxSize = DEFAULT_BACKUP_QUEUE_SIZE;
        config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
        m_blockBufferMap = std::make_shared<BlockBufferMap>();
        m_hardlinkMap = std::make_shared<HardLinkMap>();
        m_readQueue = std::make_shared<BackupQueue<FileHandle>>(config);
        m_aggregateQueue = std::make_shared<BackupQueue<FileHandle>>(config);
        m_writeQueue = std::make_shared<BackupQueue<FileHandle>>(config);
        return;
    }

    void CreateBackupEngine(const BackupParams& backupParams)
    {
        m_backupParams = backupParams;

        ReaderParams readerParams = WrapReaderParams();
        AggregatorParams aggregatorParams = WrapAggregatorParams();

        /* init control file reader */
        m_controlFileReader = std::make_unique<ControlFileReaderType>(readerParams);
        /* init aggregator */
        m_aggregator = std::make_unique<AggregatorType>(aggregatorParams);
        return;
    }

    /* only used for host backup */
    void CreateHostBackupEngine(
        const std::string&  source,
        const std::string&  destination,
        const std::string&  metaPath,
        bool                writeMeta)
    {
        m_backupParams.commonParams.subJobId = FSBackupUtils::GenerateRandomStr();
        m_backupParams.scanAdvParams.metaFilePath = metaPath;
        m_backupParams.commonParams.writeMeta = writeMeta;
        m_backupParams.srcEngine = HOST_OS_PLATFORM_IO_ENGINE;
        m_backupParams.dstEngine = HOST_OS_PLATFORM_IO_ENGINE;
        m_backupParams.srcAdvParams = std::make_shared<HostBackupAdvanceParams>();
        m_backupParams.dstAdvParams = std::make_shared<HostBackupAdvanceParams>();
        std::dynamic_pointer_cast<HostBackupAdvanceParams>(m_backupParams.srcAdvParams)->dataPath = source;
        std::dynamic_pointer_cast<HostBackupAdvanceParams>(m_backupParams.dstAdvParams)->dataPath = destination;

        ReaderParams readerParams = WrapReaderParams();
        AggregatorParams aggregatorParams = WrapAggregatorParams();

        /* init control file reader */
        m_controlFileReader = std::make_unique<ControlFileReaderType>(readerParams);
        /* init aggregator */
        m_aggregator = std::make_unique<AggregatorType>(aggregatorParams);
        return;
    }

    bool IsMemberNull() const
    {
        if (m_controlFileReader == nullptr || m_reader == nullptr || m_aggregator == nullptr || m_writer == nullptr) {
            return true;
        }
        return false;
    }

    /* all writer/reader/aggreator completed without error  */
    bool IsCompleted(
        const BackupPhaseStatus &controlFileReaderStatus,
        const BackupPhaseStatus &readerStatus,
        const BackupPhaseStatus &aggregatorStatus,
        const BackupPhaseStatus &writerStatus) const
    {
        if (controlFileReaderStatus == BackupPhaseStatus::COMPLETED &&
            aggregatorStatus == BackupPhaseStatus::COMPLETED &&
            readerStatus == BackupPhaseStatus::COMPLETED &&
            writerStatus == BackupPhaseStatus::COMPLETED) {
            return true;
        }
        return false;
    }

    bool IsStatusFailed(const BackupPhaseStatus &copyStatus) const
    {
        if (copyStatus == BackupPhaseStatus::FAILED ||
            copyStatus == BackupPhaseStatus::FAILED_NOACCESS ||
            copyStatus == BackupPhaseStatus::FAILED_NOSPACE ||
            copyStatus == BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE ||
            copyStatus == BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE) {
            return true;
        }
        return false;
    }

    /* all writer/reader/aggreator completed or terminated with error  */
    bool IsFailedOrCompleted(
        const BackupPhaseStatus &controlFileReaderStatus,
        const BackupPhaseStatus &readerStatus,
        const BackupPhaseStatus &aggregatorStatus,
        const BackupPhaseStatus &writerStatus)
    {
        /* If any of reader/writer/aggreagtor/controlReader failed,
         * should wait for all status to be either failed or completed.
         */
        if ((IsStatusFailed(controlFileReaderStatus) || controlFileReaderStatus == BackupPhaseStatus::COMPLETED) &&
            (IsStatusFailed(aggregatorStatus) || aggregatorStatus == BackupPhaseStatus::COMPLETED) &&
            (IsStatusFailed(writerStatus) || writerStatus == BackupPhaseStatus::COMPLETED) &&
            (IsStatusFailed(readerStatus) || readerStatus == BackupPhaseStatus::COMPLETED)) {
            return true;
        }
        return false;
    }

    bool IsAborted(
        const BackupPhaseStatus &controlFileReaderStatus,
        const BackupPhaseStatus &readerStatus,
        const BackupPhaseStatus &aggregatorStatus,
        const BackupPhaseStatus &writerStatus) const
    {
        if (controlFileReaderStatus == BackupPhaseStatus::ABORTED &&
            aggregatorStatus == BackupPhaseStatus::ABORTED &&
            readerStatus == BackupPhaseStatus::ABORTED &&
            writerStatus == BackupPhaseStatus::ABORTED) {
            return true;
        }
        return false;
    }

    ReaderParams WrapReaderParams() const
    {
        ReaderParams readerParams;
        readerParams.backupParams = m_backupParams;
        readerParams.readQueuePtr = m_readQueue;
        readerParams.aggregateQueuePtr = m_aggregateQueue;
        readerParams.blockBufferMap = m_blockBufferMap;
        readerParams.controlInfo = m_controlInfo;
        readerParams.hardlinkMap = m_hardlinkMap;
        return readerParams;
    }

    WriterParams WrapWriterParams() const
    {
        WriterParams writerParams;
        writerParams.backupParams = m_backupParams;
        writerParams.writeQueuePtr = m_writeQueue;
        writerParams.readQueuePtr = m_readQueue;
        writerParams.blockBufferMap = m_blockBufferMap;
        writerParams.controlInfo = m_controlInfo;
        writerParams.hardlinkMap = m_hardlinkMap;
        return writerParams;
    }

    AggregatorParams WrapAggregatorParams() const
    {
        AggregatorParams aggregatorParams {};
        aggregatorParams.backupParams = m_backupParams;
        aggregatorParams.readQueuePtr = m_readQueue;
        aggregatorParams.writeQueuePtr = m_writeQueue;
        aggregatorParams.aggregateQueuePtr = m_aggregateQueue;
        aggregatorParams.hardlinkMap = m_hardlinkMap;
        aggregatorParams.blockBufferMap = m_blockBufferMap;
        aggregatorParams.controlInfo = m_controlInfo;
        return aggregatorParams;
    }

protected:
    /* need to implement these methods for template methods CreateBackupEngine */
    virtual void InitReaderEngine(BackupIOEngine srcEngine, const ReaderParams& readerParams) = 0;
    virtual void InitWriterEngine(BackupIOEngine dstEngine, const WriterParams& writerParams) = 0;

    virtual void InitHostReaderEngine(const ReaderParams& readerParams) = 0;
    virtual void InitHostWriterEngine(const WriterParams& writerParams) = 0;

protected:
    /* template common members */
    std::unique_ptr<ControlFileReaderType> m_controlFileReader   = nullptr;     /* Control file reader main thread object */
    std::unique_ptr<ReaderBase> m_reader                         = nullptr;     /* Reader main thread object */
    std::unique_ptr<WriterBase> m_writer                         = nullptr;     /* Writer main thread object */
    std::unique_ptr<AggregatorType> m_aggregator                 = nullptr;     /* Aggregator main thread object */

    /* common members */
    std::shared_ptr<BackupControlInfo>      m_controlInfo        = nullptr;
    std::shared_ptr<BlockBufferMap>         m_blockBufferMap     = nullptr;
    std::shared_ptr<HardLinkMap>            m_hardlinkMap        = nullptr;     /* used only for hardlink stage */

    std::shared_ptr<BackupQueue<FileHandle>> m_readQueue         = nullptr;     /* queue used by dir control file reader and dir reader */
    std::shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue    = nullptr;     /* queue used by dir reader and dir aggregator reader */
    std::shared_ptr<BackupQueue<FileHandle>> m_writeQueue        = nullptr;     /* queue used by dir aggregator reader and dir writer */

    bool m_abort { false };

    /* record files/directory failed to backup */
    std::shared_ptr<Module::BackupFailureRecorder> m_failureRecorder = nullptr;

};
#endif  // BACKUP_COMMON_SERVICE_BASE_H