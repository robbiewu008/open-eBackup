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
#ifndef OBJECT_COPY_WRITER_H
#define OBJECT_COPY_WRITER_H

#include <unordered_map>
#include "WriterBase.h"
#include "ObjectServiceTask.h"
#include "ThreadPool.h"
#include "log/BackupFailureRecorder.h"

class ObjectCopyWriter : public WriterBase {
public:
    explicit ObjectCopyWriter(
            const WriterParams &copyWriterParams,
            std::shared_ptr<Module::BackupFailureRecorder> failureRecorder);
    virtual ~ObjectCopyWriter();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;

protected:
    bool CheckObsServer();
    void ThreadFunc() override;

    /* implement for CopyWriter */
    int OpenFile(FileHandle& fileHandle) override;
    int WriteData(FileHandle& fileHandle) override;
    int WriteMeta(FileHandle& fileHandle) override;
    int CloseFile(FileHandle& fileHandle) override;

    void ProcessInitState(FileHandle& fileHandle);
    int64_t ProcessTimers();
    void ClearWriteCache(std::string fileName);
    void InsertWriteCache(FileHandle& fileHandle);

    bool IsAbort() const;
    bool IsComplete();
    void PrintControlInfo(std::string head);
    bool IsSmallFile(FileHandle& fileHandle);
    bool IsIgnore(FileHandle& fileHandle);

    void CloseWriteFailedHandle(FileHandle& fileHandle);
    void PollWriteTask();
    void HandleSuccessEvent(std::shared_ptr<ObjectServiceTask> taskPtr);
    void HandleFailedEvent(std::shared_ptr<ObjectServiceTask> taskPtr);
    bool IsOpenBlock(const FileHandle& fileHandle);

    /* Hint:: can be unified later */
    void ProcessWriteEntries(FileHandle& fileHandle);
    /* Hint:: can be unified later */
    void ProcessWriteData(FileHandle& fileHandle);
    std::string GetDstBucketName(std::string& path);
    int CreateBucket(FileHandle& fileHandle);

protected:
    std::string             m_threadPoolKey;
    std::thread             m_thread;
    std::thread             m_pollThread;
    std::atomic<uint64_t>   m_writeTaskProduce { 0 };
    std::atomic<uint64_t>   m_writeTaskConsume { 0 };
    std::mutex              m_cacheMutex;
    time_t                  m_isCompleteTimer { 0 };

    std::shared_ptr<Module::JobScheduler>       m_jsPtr;
    std::shared_ptr<ObjectBackupAdvanceParams>    m_dstAdvParams;
    std::unordered_map<std::string, std::vector<FileHandle>> m_writeCache;
    std::shared_ptr<std::map<std::string, std::vector<std::string>>> m_uploadInfoMap;
    std::set<std::string> m_createdBucket;

    ObjectServiceParams m_params;
    bool m_needCheckServer { false };
    bool m_threadDone { false };
    bool m_pollThreadDone { false };
};

#endif // OBJECT_COPY_WRITER_H