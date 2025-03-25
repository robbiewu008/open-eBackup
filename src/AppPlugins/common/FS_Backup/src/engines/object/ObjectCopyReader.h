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
#ifndef OBJECT_COPY_READER_H
#define OBJECT_COPY_READER_H

#include "ReaderBase.h"
#include "ThreadPool.h"
#include "PacketStats.h"
#include "ObjectServiceTask.h"

class ObjectCopyReader : public ReaderBase {
public:
    explicit ObjectCopyReader(const ReaderParams &copyReaderParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder = nullptr);
    ~ObjectCopyReader();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
    BackupPhaseStatus GetStatus() override;
    void ThreadFunc() override;
    void PollReadTask();

    bool IsAbort() const;
    bool IsComplete();

    std::shared_ptr<PacketStats> m_pktStats = nullptr;

private:
    int OpenFile(FileHandle& fileHandle) override;
    int ReadData(FileHandle& fileHandle) override;
    int ReadMeta(FileHandle& fileHandle) override;
    int CloseFile(FileHandle& fileHandle) override;

    bool CheckObsServer();
    void BlockReadQueuePop() const;
    bool IsNeedRead(FileHandle& fileHandle);
    void PushToAggregator(FileHandle& fileHandle);
    void ProcessReadEntries(FileHandle& fileHandle);
    int64_t ProcessTimers();
    void HandleComplete();
    void PrintControlInfo(std::string head);

    std::string GetBucketName(std::string& path);
    void DecomposeAndPush(FileHandle& fileHandle, uint64_t startOffset, uint64_t totalSize, uint64_t& startSeqCnt);
    void PushToReader(FileHandle& fileHandle);
    void HandleSuccessEvent(std::shared_ptr<ObjectServiceTask> taskPtr);
    void HandleFailedEvent(std::shared_ptr<ObjectServiceTask> taskPtr);
    bool HandleFailedEventInner(FileHandle& fileHandle, std::shared_ptr<ObjectServiceTask>& taskPtr);

    std::thread m_thread;
    std::thread m_pollThread;
    ObjectServiceParams m_params {};
    std::string m_threadPoolKey;
    std::shared_ptr<Module::JobScheduler> m_jsPtr;
    std::shared_ptr<ObjectBackupAdvanceParams> m_srcAdvParams;

    bool m_failed { false };
    bool m_threadDone { false };
    bool m_pollThreadDone { false };
    BackupPhaseStatus m_failReason = BackupPhaseStatus::FAILED;
    time_t m_isCompleteTimer { 0 };

    bool m_needCheckServer { false };
};

#endif // LIBSMB_COPY_READER_H
