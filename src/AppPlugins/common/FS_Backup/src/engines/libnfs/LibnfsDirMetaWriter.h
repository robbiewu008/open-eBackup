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
#ifndef LIBNFS_DIRMETA_WRITER_H
#define LIBNFS_DIRMETA_WRITER_H

#include <memory>
#include <string>
#include "WriterBase.h"
#include "PacketStats.h"
#include "BlockBufferMap.h"
#include "ThreadPool.h"
#include "LibnfsServiceTask.h"
#include "LibnfsCommonMethods.h"

class LibnfsDirMetaWriter : public WriterBase {
public:
    explicit LibnfsDirMetaWriter(const WriterParams &dirWriterParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder = nullptr);
    ~LibnfsDirMetaWriter();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
    BackupPhaseStatus GetStatus() override;

    bool IsBlockSend() const;
    void BlockSend() const;
    bool IsResumeSend() const;
    void ResumeSend() const;
    std::shared_ptr<PacketStats> m_pktStats = nullptr;

private:
    bool IsComplete();
    void PrintIsComplete(bool forcePrint);
    void HandleComplete();

    void ThreadFunc() override;

    int OpenFile(FileHandle &fileHandle) override;
    int WriteData(FileHandle &fileHandle) override;
    int WriteMeta(FileHandle &fileHandle) override;
    int CloseFile(FileHandle &fileHandle) override;

    void PushToWriter(FileHandle &fileHandle, uint16_t &count);

    void MonitorWriteTask();

    void FillNfsServerCheckParams();

    void ProcRetryTimers();
    void ExpireRetryTimers();
    bool IsRetryReqEmpty();
    uint64_t GetRetryTimerCnt();

    void HandleQueueBlock();

    std::thread m_monitorWriteThread;
    std::string m_threadPoolKey;
    std::shared_ptr<Module::JobScheduler> m_jsPtr;
    std::shared_ptr<LibnfsBackupAdvanceParams> m_advParams;
    LibnfsParams m_params;
    NfsCommonData m_commonData {};
    std::atomic<uint64_t> m_writerProduce { 0 };

    Module::NfsContextContainer m_nfsContextContainer;               /* write nfs context containers */
    std::atomic<uint16_t> m_runningJob = 0;                          /* Running Job count */
    time_t m_isCompleteTimer { 0 };
    time_t m_ratelimitTimer { 0 };
    std::shared_ptr<FileHandleCache> m_fileHandleCache { nullptr };
    BackupPhaseStatus m_failReason = BackupPhaseStatus::FAILED;
    Libnfscommonmethods::NasServerCheckParams m_nasServerCheckParams {};
    std::mutex mtx {};
    std::thread m_thread;
};

#endif // LIBNFS_DIRMETA_WRITER_H