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
#ifndef NFS_ANTI_RANSOMWARE_WORM_WRITER_H
#define NFS_ANTI_RANSOMWARE_WORM_WRITER_H

#include <memory>
#include <string>
#include "AntiRansomwareWriter.h"
#include "BlockBufferMap.h"
#include "PacketStats.h"
#include "ThreadPool.h"
#include "LibnfsServiceTask.h"
#include "LibnfsInterface.h"

class NfsWormWriter : public AntiRansomwareWriter {
public:
    explicit NfsWormWriter(const AntiWriterParams &antiWriterParams);
    ~NfsWormWriter();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;

    bool IsResumeSend();
    void ResumeSend();
    static bool IsResumeSendCb(void *cbObj);
    static void ResumeSendCb(void *cbObj);

    std::shared_ptr<PacketStats> m_pktStats = nullptr;

private:
    bool IsComplete();
    bool IsAbort() const;
    void HandleComplete();
    void ThreadFunc() override;

    int OpenFile(FileHandle& fileHandle) override;
    int WriteData(FileHandle& fileHandle) override;
    int WriteMeta(FileHandle& fileHandle) override;
    int CloseFile(FileHandle& fileHandle) override;

    void HandleAbort();
    int NfsServerCheck();
    int FillWriteContainers();

    void ProcRetryTimers();
    bool IsRetryReqEmpty();
    uint64_t GetRetryTimerCnt();
    int SendSetMetaRequest(FileHandle &fileHandle);
    void HandleSendNfsRequestFailure(FileHandle &fileHandle);

    std::thread m_thread;
    std::string m_threadPoolKey;
    std::shared_ptr<Module::JobScheduler> m_jsPtr;
    std::shared_ptr<NfsAntiRansomwareAdvanceParams> m_advParams;
    LibnfsParams m_params;
    std::shared_ptr<FileHandleCache> m_fileHandleCache { nullptr };
    NfsCommonData m_commonData {};


    std::atomic<uint64_t> m_writerProduce { 0 };

    Module::NfsContextContainer m_nfsContextContainer;             /* write nfs context containers */
    Module::NfsContextContainer m_syncNfsContextContainer;           /* Server check nfs context containers */

    std::atomic<uint16_t> m_runningJob = 0;                        /* Running Job count */

    time_t m_isCompleteTimer { 0 };

    BackupPhaseStatus m_failReason = BackupPhaseStatus::FAILED;

    std::mutex mtx {};
};

#endif // NFS_ANTI_RANSOMWARE_WORM_WRITER_H