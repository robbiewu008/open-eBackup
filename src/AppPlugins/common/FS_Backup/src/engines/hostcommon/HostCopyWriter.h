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
#ifndef HOST_COPY_WRITER_H
#define HOST_COPY_WRITER_H

#include <unordered_map>
#include "WriterBase.h"
#include "BlockBufferMap.h"
#include "ThreadPool.h"
#include "log/BackupFailureRecorder.h"
#include "OsPlatformDefines.h"

class HostCopyWriter : public WriterBase {
public:
    explicit HostCopyWriter(
        const WriterParams &copyWriterParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder);
    virtual ~HostCopyWriter();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;

protected:
    void ThreadFunc() override;

    /* implement for CopyWriter */
    int OpenFile(FileHandle& fileHandle) override;
    int WriteData(FileHandle& fileHandle) override;
    int CloseFile(FileHandle& fileHandle) override;
    int CreateDir(FileHandle& fileHandle);

    int64_t ProcessTimers();
    void ClearWriteCache();
    void InsertWriteCache(FileHandle& fileHandle);
    virtual void CloseOpenedHandle();

    /* Hint:: used in template method ThreadFunc, implement according to os platform and can be unified later */
    virtual void ProcessWriteEntries(FileHandle& fileHandle) = 0;

    bool IsAbort() const;
    virtual bool IsComplete();

    void CloseWriteFailedHandle(FileHandle& fileHandle);
    void PollWriteTask();
    virtual void HandleSuccessEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr);
    virtual void HandleFailedEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr);
    bool IsOpenBlock(const FileHandle& fileHandle);
    virtual void ProcessWriteData(FileHandle& fileHandle) = 0;

protected:
    std::string             m_threadPoolKey;
    std::thread             m_thread;
    std::thread             m_pollThread;
    bool                    m_threadDone { false };
    bool                    m_pollThreadDone { false };
    std::atomic<uint64_t>   m_writeTaskProduce { 0 };
    std::atomic<uint64_t>   m_writeTaskConsume { 0 };
    std::mutex              m_cacheMutex;
    HostParams              m_params;
    time_t                  m_isCompleteTimer { 0 };
    std::unordered_set<std::shared_ptr<FileDesc>>   m_dstOpenedHandleSet;

    std::shared_ptr<Module::JobScheduler>       m_jsPtr;
    std::shared_ptr<HostBackupAdvanceParams>    m_dstAdvParams;
    std::unordered_map<std::string, std::vector<FileHandle>> m_writeCache;
};

#endif // HOST_COPY_WRITER_H