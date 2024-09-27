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
#ifndef HOST_HARDLINK_WRITER_H
#define HOST_HARDLINK_WRITER_H
 
#include "WriterBase.h"
#include "BlockBufferMap.h"
#include "ThreadPool.h"
#include "log/BackupFailureRecorder.h"
#include "OsPlatformDefines.h"
 
class HostHardlinkWriter : public WriterBase {
public:
    explicit HostHardlinkWriter(
        const WriterParams &hardlinkWriterParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder);
    virtual ~HostHardlinkWriter();
 
    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
 
protected:
    void ThreadFunc() override;
 
    int OpenFile(FileHandle& fileHandle) override;
    int WriteData(FileHandle& fileHandle) override;
    int WriteMeta(FileHandle& fileHandle) override;
    int CloseFile(FileHandle& fileHandle) override;
    int LinkFile(FileHandle& fileHandle);
 
    void ClearWriteCache();
    void InsertWriteCache(FileHandle& fileHandle);
    int64_t ProcessTimers();
    virtual void ProcessWriteEntries(FileHandle& fileHandle) = 0;
 
    bool IsAbort() const;
    bool IsComplete();
 
    void PollWriteTask();
    void HandleSuccessEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr);
    void HandleFailedEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr);
    bool IsOpenBlock(const FileHandle& fileHandle);
    virtual void ProcessWriteData(FileHandle& fileHandle) = 0;
    void ProcessHardlinkMap();
    void HandleFailedLinkSource(FileHandle& fileHandle);
    void HandleFailedLinkLink(FileHandle& fileHandle);
 
protected:
    std::string         m_threadPoolKey;
    std::thread         m_thread;
    std::thread         m_pollThread;
    HostParams          m_params;
    time_t              m_isCompleteTimer { 0 };
    bool                    m_threadDone { false };
    bool                    m_pollThreadDone { false };
    std::atomic<uint64_t>   m_writeTaskProduce { 0 };
    std::atomic<uint64_t>   m_writeTaskConsume { 0 };
    std::shared_ptr<Module::JobScheduler>       m_jsPtr;
    std::shared_ptr<HostBackupAdvanceParams>    m_srcAdvParams;
    std::shared_ptr<HostBackupAdvanceParams>    m_dstAdvParams;
    std::unordered_map<std::string, std::vector<FileHandle>> m_writeCache;
};
 
#endif // HOST_HARDLINK_WRITER_H