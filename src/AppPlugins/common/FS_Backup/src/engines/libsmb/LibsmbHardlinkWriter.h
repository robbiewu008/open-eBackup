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
#ifndef LIBSMB_HARDLINK_WRITER_H
#define LIBSMB_HARDLINK_WRITER_H


#include "WriterBase.h"
#include "BlockBufferMap.h"
#include "Libsmb.h"
#include "interface/LibsmbWriterSyncInterface.h"
#include "interface/LibsmbWriterInterface.h"
#include "interface/LibsmbStructs.h"
#include "interface/LibsmbCommon.h"
#include "PacketStats.h"

class LibsmbHardlinkWriter : public WriterBase {
public:
    explicit LibsmbHardlinkWriter(const WriterParams &hardlinkWriterParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder = nullptr);
    ~LibsmbHardlinkWriter();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
    BackupPhaseStatus GetStatus() override;

    bool IsAbort() const;
    bool IsComplete();
    bool IsMkdirComplete() const;
    void HandleComplete();

    void ThreadFunc() override;
    std::shared_ptr<PacketStats> m_pktStats = nullptr;

    bool m_suspend { false };

private:
    int OpenFile(FileHandle &fileHandle) override;
    int WriteData(FileHandle &fileHandle) override;
    int WriteMeta(FileHandle &fileHandle) override;
    int CloseFile(FileHandle &fileHandle) override;
    int DeleteFile(FileHandle &fileHandle);
    int LinkFiles(FileHandle &fileHandle);
    int LinkFile(FileHandle &fileHandle);

    void ProcessWriteEntries();
    void ProcessFileDescState(FileHandle fileHandle);

    int SmbConnectContexts();
    void SmbDisconnectContexts();
    void SmbDisconnectSyncContexts();

    int ServerCheck();
    int ProcessConnectionException();
    int64_t ProcessTimers();
    void SyncThreadFunc();
    void FillSmbWriterCommonData(SmbWriterCommonData *writerCommonData);
    SmbWriterCommonData* GetSmbWriterCommonData(FileHandle &fileHandle);
    bool IsWriterRequestReachThreshold() const;
    void ProcessHardlinkMap();

    void ClearWriteCache();

    std::shared_ptr<LibsmbBackupAdvanceParams> m_srcAdvParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> m_dstAdvParams;
    std::shared_ptr<BackupQueue<FileHandle>> m_dirQueue;
    LibsmbParams m_params;

    std::shared_ptr<Module::SmbContextWrapper> m_asyncContext;
    // 同步请求用的context，创建目录和删除目录因为要递归地操作，用同步请求比较容易实现，但是同步接口又影响异步请求的性能，所以需要分开
    std::shared_ptr<Module::SmbContextWrapper> m_syncContext;
    std::shared_ptr<SmbWriterCommonData> m_writerCommonData;
    std::unordered_map<std::string, std::vector<FileHandle>> m_writeCache;

    std::thread m_thread;
    std::thread m_syncThread;

    bool m_failed { false };
    bool m_threadDone { false };
    BackupPhaseStatus m_failReason = BackupPhaseStatus::FAILED;

    time_t m_isCompleteTimer { 0 };
};

#endif // LIBSMB_HARDLINK_WRITER_H