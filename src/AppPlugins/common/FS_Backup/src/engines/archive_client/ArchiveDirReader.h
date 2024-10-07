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
#ifndef ARCHIVE_DIR_READER_H
#define ARCHIVE_DIR_READER_H

#include "ReaderBase.h"

#include <memory>

#include "ThreadPool.h"
#include "BlockBufferMap.h"
#include "ArchiveServiceTask.h"

class ArchiveDirReader : public ReaderBase {
public:
    explicit ArchiveDirReader(const ReaderParams &dirReaderParams);
    ~ArchiveDirReader();

    ArchiveDirReader(const ArchiveDirReader& reader) = delete;
    ArchiveDirReader& operator=(const ArchiveDirReader& reader) = delete;

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;

    int OpenFile(FileHandle& fileHandle) override;
    int ReadData(FileHandle& fileHandle) override;
    int ReadMeta(FileHandle& fileHandle) override;
    int CloseFile(FileHandle& fileHandle) override;

    void ThreadFunc() override;
    bool IsComplete();
    void HandleComplete() const;

private:
    void HandlePosixSuccessEvent(std::shared_ptr<ArchiveServiceTask> taskPtr);
    void HandlePosixFailedEvent(std::shared_ptr<ArchiveServiceTask> taskPtr);
    std::shared_ptr<ArchiveRestoreAdvanceParams> m_srcAdvParams;
    std::shared_ptr<HostBackupAdvanceParams> m_dstAdvParams;
    ArchiveServiceParams m_params;
    std::atomic<uint64_t> m_readedDir { 0 };
    std::thread m_thread;
    bool m_threadDone { false };
};

#endif // ARCHIVE_DIR_READER_H