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
#ifndef LIBNFS_COPY_READER_H
#define LIBNFS_COPY_READER_H

#include "ThreadPoolFactory.h"
#include "ReaderBase.h"
#include "NfsContextContainer.h"
#include "PacketStats.h"
#include "LibnfsCommonMethods.h"
#include "LibnfsInterface.h"

class LibnfsCopyReader : public ReaderBase {
public:
    explicit LibnfsCopyReader(const ReaderParams &copyReaderParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder = nullptr);
    ~LibnfsCopyReader();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
    BackupPhaseStatus GetStatus() override;

    static bool CanRecv(void *cbData);
    bool IsBlockRecv();
    void BlockRecv();
    bool IsResumeRecv() const;
    void ResumeRecv();
    bool IsBlockSend();
    void BlockSend() const;
    bool IsResumeSend();
    void ResumeSend() const;

    static bool IsResumeSendCb(void *cbObj);
    static void ResumeSendCb(void *cbObj);

    std::shared_ptr<PacketStats> m_pktStats = nullptr;
    std::shared_ptr<LibnfsBackupAdvanceParams> m_advParams;

private:
    int OpenFile(FileHandle &fileHandle) override;
    int ReadData(FileHandle &fileHandle) override;
    int ReadMeta(FileHandle &fileHandle) override;
    int CloseFile(FileHandle &fileHandle) override;

    bool IsComplete();
    void PrintIsComplete(bool forcePrint);
    void HandleComplete();
    void ThreadFunc() override;

    void ProcessPartialReadQueue();
    void ProcessReadQueue();

    int ProcessReadEntries(FileHandle &fileHandle);
    int ProcessFileTypes(FileHandle &fileHandle);
    int ProcessFileToRead(FileHandle &fileHandle);

    void ProcRetryTimers();
    bool IsRetryReqEmpty();
    uint64_t GetRetryTimerCnt();
    void FillNfsServerCheckParams();

    int FillReadContainers();
    void DeleteReadContainers();

    void HandleQueueBlock();

    NfsCommonData m_commonData {};

    Module::NfsContextContainer m_nfsContextContainer;             /* nfs contexts  (used for async calls) */
    Module::NfsContextContainer m_syncNfsContextContainer;         /* nfs contexts  (used for sync calls) */

    time_t m_isCompleteTimer { 0 };
    time_t m_ratelimitTimer { 0 };

    BackupPhaseStatus m_failReason = BackupPhaseStatus::FAILED;

    Libnfscommonmethods::NasServerCheckParams m_nasServerCheckParams {};

    std::shared_ptr<BackupQueue<FileHandle>> m_partialReadQueue;

    std::mutex mtx {};
    std::thread m_thread;
    bool m_threadDone { false };
};

#endif // LIBNFS_COPY_READER_H
