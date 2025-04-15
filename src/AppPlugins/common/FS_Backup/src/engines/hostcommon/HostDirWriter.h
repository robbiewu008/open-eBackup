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
#ifndef HOST_DIR_WRITER_H
#define HOST_DIR_WRITER_H

#include <memory>
#include <string>
#include "WriterBase.h"
#include "BlockBufferMap.h"
#include "ThreadPool.h"
#include "log/BackupFailureRecorder.h"
#include "OsPlatformDefines.h"

class HostDirWriter : public WriterBase {
public:
    explicit HostDirWriter(
        const WriterParams &dirWriterParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder);
    virtual ~HostDirWriter();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;

protected:
    std::string m_threadPoolKey;
    std::shared_ptr<Module::JobScheduler> m_jsPtr;
    std::shared_ptr<HostBackupAdvanceParams> m_srcAdvParams;
    std::shared_ptr<HostBackupAdvanceParams> m_dstAdvParams;
    HostParams m_params;
    std::thread m_thread;
    bool m_threadDone { false };
    time_t m_isCompleteTimer { 0 };

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
    void HandleSuccessEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr);
    
    virtual void HandleFailedEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr) = 0;
};

#endif // HOST_DIR_WRITER_H