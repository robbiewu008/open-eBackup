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
#ifndef LIBSMB_DELETE_WRITER_H
#define LIBSMB_DELETE_WRITER_H

#include "WriterBase.h"
#include "BlockBufferMap.h"
#include "Libsmb.h"
#include "interface/LibsmbWriterInterface.h"
#include "interface/LibsmbWriterSyncInterface.h"
#include "interface/LibsmbStructs.h"
#include "interface/LibsmbCommon.h"

class LibsmbDeleteWriter : public WriterBase {
public:
    explicit LibsmbDeleteWriter(const WriterParams &deleteWriterParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder = nullptr);
    ~LibsmbDeleteWriter();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
    BackupPhaseStatus GetStatus() override;

    std::shared_ptr<PacketStats> m_pktStats = nullptr;
private:
    void ThreadFunc() override;

    int OpenFile(FileHandle &fileHandle) override;
    int WriteData(FileHandle &fileHandle) override;
    int WriteMeta(FileHandle &fileHandle) override;
    int CloseFile(FileHandle &fileHandle) override;

    bool IsAbort();
    bool IsComplete();
    void HandleComplete();

    BackupRetCode DeleteFileDirectoryLibSmb(FileHandle &fileHandle, bool isDir);
    BackupRetCode CompareTypeOfDeleteEntryAndBackupCopy(FileHandle &fileHandle, bool delStatIsDir);
    int SmbConnectContexts();
    void SmbContextDisconnect();
    void FillSmbWriterCommonData(SmbWriterCommonData *writerCommonData);
    SmbWriterCommonData* GetSmbWriterCommonData(FileHandle &fileHandle);
    void AddDeleteCounter(bool isSuccess, bool isDir);

private:
    std::shared_ptr<LibsmbBackupAdvanceParams> m_srcAdvParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> m_dstAdvParams;
    LibsmbParams m_params;
    std::atomic<uint64_t> m_deleteDir { 0 };
    std::atomic<uint64_t> m_deleteFailedDir { 0 };
    std::atomic<uint64_t> m_deleteFile { 0 };
    std::atomic<uint64_t> m_deleteFailedFile { 0 };
    std::shared_ptr<Module::SmbContextWrapper> m_deleteSmbContext;
    std::thread m_thread;

    bool m_threadDone { false };
    time_t m_isCompleteTimer { 0 };
};

#endif // LIBSMB_DELETE_WRITER_H