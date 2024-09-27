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
#ifndef HOST_HARDLINK_READER_H
#define HOST_HARDLINK_READER_H
 
#include <memory>
#include "ReaderBase.h"
#include "ThreadPool.h"
#include "BlockBufferMap.h"
#include "log/BackupFailureRecorder.h"
#include "OsPlatformDefines.h"
 
class HostHardlinkReader : public ReaderBase {
public:
    explicit HostHardlinkReader(
        const ReaderParams &hardlinkReaderParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder);
    virtual ~HostHardlinkReader();
    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
    
protected:
    void ThreadFunc() override;
 
    int OpenFile(FileHandle& fileHandle) override;
    int ReadData(FileHandle& fileHandle) override;
    int ReadMeta(FileHandle& fileHandle) override;
    int CloseFile(FileHandle& fileHandle) override;
 
    virtual int ReadEmptyData(FileHandle& fileHandle) = 0;
    virtual int ReadSymlinkData(FileHandle& fileHandle) = 0;
    virtual int ReadNormalData(FileHandle& fileHandle) = 0;
 
    int64_t ProcessTimers();
    virtual void ProcessReadEntries(FileHandle& fileHandle) = 0; /* state to event */
 
    bool IsAbort() const;
    bool IsComplete();
    void BlockReadQueuePop() const;
 
    void Init(FileHandle& fileHandle);
    void PollReadTask();
    virtual void HandleSuccessEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr) = 0; /* event to state */
    void HandleFailedEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr);
    void PushToAggregator(FileHandle& fileHandle);
    void PushToReader(FileHandle& fileHandle);
    void DecomposeAndPush(FileHandle& fileHandle) const;
    void DecomposeAndPush(FileHandle& fileHandle, uint64_t startOffset, uint64_t totalSize, uint64_t& startSeq) const;
    void HandleFailedLink(FileHandle& fileHandle);

protected:
    std::string             m_threadPoolKey;
    std::thread             m_thread;
    std::thread             m_pollThread;
    HostParams              m_params;
    time_t                  m_isCompleteTimer { 0 };
    bool                    m_threadDone { false };
    bool                    m_pollThreadDone { false };
    std::atomic<uint64_t>   m_readTaskProduce { 0 };
    std::atomic<uint64_t>   m_readTaskConsume { 0 };

    std::shared_ptr<Module::JobScheduler> m_jsPtr;
    std::shared_ptr<HostBackupAdvanceParams> m_srcAdvParams;
    std::shared_ptr<HostBackupAdvanceParams> m_dstAdvParams;
};
 
#endif // HOST_HARDLINK_READER_H