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
#ifndef ARCHIVE_COPY_READER_H
#define ARCHIVE_COPY_READER_H

#include <memory>

#include "ReaderBase.h"

#include "threadpool/ThreadPool.h"
#include "BlockBufferMap.h"
#include "ArchiveServiceTask.h"

class ArchiveCopyReader : public ReaderBase {
public:
    explicit ArchiveCopyReader(const ReaderParams &copyReaderParams);
    ~ArchiveCopyReader();

    ArchiveCopyReader(const ArchiveCopyReader& reader) = delete;
    ArchiveCopyReader& operator=(const ArchiveCopyReader& reader) = delete;

    void SetArchiveClient(std::shared_ptr<ArchiveClientBase> client);

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
    BackupRetCode Enqueue(FileHandle& fileHandle);

private:
    int OpenFile(FileHandle& fileHandle) override;
    int ReadData(FileHandle& fileHandle) override;
    int ReadMeta(FileHandle& fileHandle) override;
    int CloseFile(FileHandle& fileHandle) override;

    bool IsAbort() const;

    void ThreadFunc() override;
    int64_t ProcessTimers();
    void ProcessReadEntries(FileHandle& fileHandle);
    void BlockReadQueuePop() const;

    int ReadEmptyData(FileHandle& fileHandle);
    int ReadNormalData(FileHandle& fileHandle);
    int ReadSymlinkData(FileHandle& fileHandle);
    bool IsComplete();

    void PollReadTask();
    void HandleSuccessEvent(std::shared_ptr<ArchiveServiceTask> taskPtr);
    void HandleFailedEvent(std::shared_ptr<ArchiveServiceTask> taskPtr);
    void PushFileHandleToAggregator(FileHandle& fileHandle);
    std::string m_threadPoolKey;
    std::thread m_thread;
    std::thread m_pollThread;
    std::shared_ptr<Module::JobScheduler> m_jsPtr;
    std::shared_ptr<ArchiveRestoreAdvanceParams> m_srcAdvParams;
    std::shared_ptr<HostBackupAdvanceParams> m_dstAdvParams;
    std::atomic<uint64_t> m_readTaskProduce { 0 };
    std::atomic<uint64_t> m_readTaskConsume { 0 };

    ArchiveServiceParams m_params; // params used in ArchiveServiceTask
    std::shared_ptr<ArchiveClientBase> m_archiveClient { nullptr };
    time_t m_isCompleteTimer { 0 };
    bool m_threadDone { false };
    bool m_pollThreadDone { false };
};

#endif // POSIX_COPY_READER_H