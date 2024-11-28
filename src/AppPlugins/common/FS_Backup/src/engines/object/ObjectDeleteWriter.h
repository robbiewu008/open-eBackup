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
#ifndef OBJECT_DELETE_WRITER_H
#define OBJECT_DELETE_WRITER_H

#include <memory>
#include <string>
#include "WriterBase.h"
#include "ObjectServiceTask.h"
#include "log/BackupFailureRecorder.h"

class ObjectDeleteWriter : public WriterBase {
public:
    explicit ObjectDeleteWriter(
        const WriterParams &deleteWriterParams, std::shared_ptr<Module::BackupFailureRecorder> failureRecorder);
    virtual ~ObjectDeleteWriter();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;

private:
    void ThreadFunc() override;

    int OpenFile(FileHandle& fileHandle) override;
    int WriteData(FileHandle& fileHandle) override;
    int WriteMeta(FileHandle& fileHandle) override;
    int CloseFile(FileHandle& fileHandle) override;

    bool IsAbort() const;
    bool IsComplete();
    int64_t ProcessTimers();
    void PollWriteTask();
    void HandleSuccessEvent(std::shared_ptr<ObjectServiceTask> taskPtr);
    void HandleFailedEvent(std::shared_ptr<ObjectServiceTask> taskPtr);

private:
    std::string           m_threadPoolKey;
    ObjectServiceParams   m_params;
    std::thread           m_thread;
    bool                  m_threadDone { false };
    time_t                m_isCompleteTimer { 0 };

    std::atomic<uint64_t> m_deleteDir { 0 };
    std::atomic<uint64_t> m_deleteFailedDir { 0 };
    std::atomic<uint64_t> m_deleteFile { 0 };
    std::atomic<uint64_t> m_deleteFailedFile { 0 };

    std::shared_ptr<Module::JobScheduler> m_jsPtr;
    std::shared_ptr<ObjectBackupAdvanceParams> m_dstAdvParams;
};

#endif  // OBJECT_DELETE_WRITER_H