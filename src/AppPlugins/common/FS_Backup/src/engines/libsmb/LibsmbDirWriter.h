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
#ifndef LIBSMB_DIR_WRITER_H
#define LIBSMB_DIR_WRITER_H

#include <memory>
#include <string>
#include "WriterBase.h"

#include "BlockBufferMap.h"
#include "interface/LibsmbStructs.h"

class LibsmbDirWriter : public WriterBase {
public:
    explicit LibsmbDirWriter(const WriterParams &dirWriterParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder = nullptr);
    ~LibsmbDirWriter();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
    BackupPhaseStatus GetStatus() override;

private:
    void ThreadFunc() override;

    int OpenFile(FileHandle &fileHandle) override;
    int WriteData(FileHandle &fileHandle) override;
    int WriteMeta(FileHandle &fileHandle) override;
    int CloseFile(FileHandle &fileHandle) override;

    bool IsAbort();
    bool IsComplete();
    void HandleComplete();

    int SmbConnectContexts();
    void SmbDisconnectContexts();

    SmbWriterCommonData* GetSmbWriterCommonData(FileHandle &fileHandle);
    int ProcessConnectionException();
    void ProcessWriteEntries();
    bool IsWriterRequestReachThreshold() const;

private:
    std::shared_ptr<LibsmbBackupAdvanceParams> m_srcAdvParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> m_dstAdvParams;
    LibsmbParams m_params;

    std::shared_ptr<Module::SmbContextWrapper> m_asyncContext;
    std::thread m_thread;
    bool m_threadDone { false };
    BackupPhaseStatus m_failReason = BackupPhaseStatus::FAILED;

    time_t m_isCompleteTimer { 0 };
    
    std::shared_ptr<PacketStats> m_pktStats = nullptr;
};

#endif // LIBSMB_DIR_WRITER_H