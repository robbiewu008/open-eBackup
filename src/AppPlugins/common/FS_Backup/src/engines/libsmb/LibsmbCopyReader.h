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
#ifndef LIBSMB_COPY_READER_H
#define LIBSMB_COPY_READER_H

#include <memory>
#include "ReaderBase.h"
#include "BlockBufferMap.h"
#include "Libsmb.h"
#include "interface/LibsmbReaderInterface.h"
#include "interface/LibsmbStructs.h"
#include "PacketStats.h"

class LibsmbCopyReader : public ReaderBase {
public:
    explicit LibsmbCopyReader(const ReaderParams &copyReaderParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder = nullptr);
    ~LibsmbCopyReader();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
    BackupPhaseStatus GetStatus() override;

    bool IsAbort() const;
    bool IsComplete();
    void HandleComplete();

    void ThreadFunc() override;

    std::shared_ptr<PacketStats> m_pktStats = nullptr;
private:
    void Init(FileHandle fileHandle);

    int OpenFile(FileHandle &fileHandle) override;
    int ReadData(FileHandle &fileHandle) override;
    int ReadMeta(FileHandle &fileHandle) override;
    int CloseFile(FileHandle &fileHandle) override;

    void HandleFileOnlyMetaModified(FileHandle &fileHandle);
    int ReadNormalData(FileHandle& fileHandle);
    void ReadNormalDataRequestFailed(FileHandle& fileHandle);

    int SmbConnectContexts();
    void SmbDisconnectContexts();

    int ServerCheck();
    int ProcessConnectionException();

    int64_t ProcessTimers();
    void ProcessReadEntries();
    void ProcessPartialReadEntries();
    void ProcessFileDescState(FileHandle fileHandle);
    void ProcessFileDescStateInteraction(FileHandle& fileHandle);
    bool IsReaderRequestReachThreshold() const;

    void FillSmbReaderCommonData(SmbReaderCommonData *readerCommonData);
    SmbReaderCommonData* GetSmbReaderCommonData(FileHandle &fileHandle);

    std::thread m_thread;
    std::shared_ptr<LibsmbBackupAdvanceParams> m_srcAdvParams;
    LibsmbParams m_params;

    std::shared_ptr<Module::SmbContextWrapper> m_asyncContext;
    std::shared_ptr<SmbReaderCommonData> m_readerCommonData;

    bool m_failed { false };
    bool m_threadDone { false };
    BackupPhaseStatus m_failReason = BackupPhaseStatus::FAILED;

    time_t m_isCompleteTimer { 0 };

    // 用于放block的queue，因为通常readQueue会有大量的open状态的filehandle，
    // 但是open之后filehandle会被放到队尾，导致无法及时处理。
    std::shared_ptr<BackupQueue<FileHandle>> m_partialReadQueue;

    std::atomic<uint64_t> m_adsFileCnt {0};
};

#endif // LIBSMB_COPY_READER_H
