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
#ifndef LIBNFS_HARDLINK_WRITER_H
#define LIBNFS_HARDLINK_WRITER_H

#include "ThreadPoolFactory.h"
#include "CopyCtrlParser.h"
#include "WriterBase.h"
#include "NfsContextContainer.h"
#include "PacketStats.h"
#include "FileHandleCache.h"
#include "LibnfsCommonMethods.h"
#include "LibnfsInterface.h"

class LibnfsHardlinkWriter : public WriterBase {
public:
    explicit LibnfsHardlinkWriter(const WriterParams &hardlinkWriterParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder = nullptr);
    ~LibnfsHardlinkWriter();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
    BackupPhaseStatus GetStatus() override;

    static bool CanRecv(void *cbData);
    bool IsBlockRecv() const;
    void BlockRecv() const;
    bool IsResumeRecv() const;
    void ResumeRecv() const;
    bool IsBlockSend();
    void BlockSend() const;
    bool IsResumeSend();
    void ResumeSend() const;

    static bool IsResumeSendCb(void *cbObj);
    static void ResumeSendCb(void *cbObj);

    std::shared_ptr<LibnfsBackupAdvanceParams> m_advParams;
    std::shared_ptr<PacketStats> m_pktStats = nullptr;
    std::shared_ptr<FileHandleCache> m_fileHandleCache = nullptr;
    std::shared_ptr<BackupQueue<FileHandle>> m_writeWaitQueue; // This is to push the blocks until file is created

private:
    int OpenFile(FileHandle &fileHandle) override;
    int WriteData(FileHandle &fileHandle) override;
    int WriteMeta(FileHandle &fileHandle) override;
    int CloseFile(FileHandle &fileHandle) override;

    bool IsComplete();
    void PrintIsComplete(bool forcePrint);
    void HandleComplete();
    void ThreadFunc() override;
    void ProcRetryTimers();
    bool IsRetryReqEmpty();
    uint64_t GetRetryTimerCnt();

    int StartMkdirThread();
    void MkdirThreadFunc();

    void ProcessWriteQueue();
    int ProcessWriteEntries(FileHandle &fileHandle);
    int ProcessMetaModifiedFiles(FileHandle &fileHandle, FileDescState state);
    int ProcessFilesCreateWriteClose(FileHandle &fileHandle, FileDescState state);
    int ProcessSymlink(FileHandle &fileHandle, FileDescState state);
    int ProcessFileOpen(FileHandle &fileHandle);

    void FillNfsServerCheckParams();

    int FillWriteContainers();
    void DeleteWriteContainers();

    void BatchPush() const;

    void HandleQueueBlock();

    bool IsReplacePolicySkip() const;

    NfsCommonData m_commonData {};

    std::thread m_thread;
    std::thread m_mkdirThread {};                    /* Copy phase mkdir main thread */
    std::shared_ptr<BackupQueue<FileHandle>> m_mkdirSyncQueue;

    Module::NfsContextContainer m_nfsContextContainer;               /* write nfs context containers */
    Module::NfsContextContainer m_syncNfsContextContainer;           /* Server check nfs context containers */

    time_t m_isCompleteTimer { 0 };
    time_t m_ratelimitTimer { 0 };

    BackupPhaseStatus m_failReason = BackupPhaseStatus::FAILED;
    Libnfscommonmethods::NasServerCheckParams m_nasServerCheckParams {};

    bool m_mkdirComplete { false };

    std::mutex mtx {};
};

#endif // LIBNFS_HARDLINK_WRITER_H
