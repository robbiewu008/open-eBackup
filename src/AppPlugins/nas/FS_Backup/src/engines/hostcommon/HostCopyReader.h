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
#ifndef HOST_COPY_READER_H
#define HOST_COPY_READER_H

#include <memory>

#include "ReaderBase.h"
#include "ThreadPool.h"
#include "BlockBufferMap.h"
#include "OsPlatformDefines.h"
#include "log/BackupFailureRecorder.h"

class HostCopyReader : public ReaderBase {
public:
    explicit HostCopyReader(
        const ReaderParams &copyReaderParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder);
    virtual ~HostCopyReader();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;

protected:
    void ThreadFunc() override;
    void PollReadTask();
    int64_t ProcessTimers();

    int OpenFile(FileHandle& fileHandle) override;
    int ReadData(FileHandle& fileHandle) override;
    virtual int ReadMeta(FileHandle& fileHandle) = 0;
    int CloseFile(FileHandle& fileHandle) override;
    virtual void CloseOpenedHandle();

    virtual int ReadEmptyData(FileHandle& fileHandle) = 0;
    int ReadSymlinkData(FileHandle& fileHandle);
    int ReadHugeObjectData(FileHandle& fileHandle);
    int ReadNormalData(FileHandle& fileHandle);

    bool IsAbort() const;
    virtual bool IsComplete();
    void BlockReadQueuePop() const;

    void PushToReader(FileHandle& fileHandle);
    void PushToAggregator(FileHandle& fileHandle);
    void DecomposeAndPush(FileHandle& fileHandle) const;
    void DecomposeAndPush(
        FileHandle&     fileHandle,
        uint64_t        startOffset,
        uint64_t        totalSize,
        uint64_t&       startSeqCnt) const;

    virtual void ProcessReadEntries(FileHandle& fileHandle) = 0; // can be unify later
    virtual void HandleSuccessEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr) = 0;
    void HandleFailedEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr);
    bool WriteFailedAndSkipRead(FileHandle& fileHandle);
    bool ProcessReadEntriesScannerMode(FileHandle& fileHandle);

protected:
    std::string             m_threadPoolKey;
    std::thread             m_thread;
    std::thread             m_pollThread;
    bool                    m_threadDone { false };
    bool                    m_pollThreadDone { false };
    std::atomic<uint64_t>   m_readTaskProduce { 0 };
    std::atomic<uint64_t>   m_readTaskConsume { 0 };
    HostParams              m_params;
    time_t                  m_isCompleteTimer { 0 };
    std::unordered_set<std::shared_ptr<FileDesc>>   m_srcOpenedHandleSet;

    std::shared_ptr<Module::JobScheduler>   m_jsPtr;
    std::shared_ptr<HostBackupAdvanceParams> m_srcAdvParams;
};

#endif // HOST_COPY_READER_H