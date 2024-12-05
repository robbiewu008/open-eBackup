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
#ifndef ARCHIVE_DELETE_READER_H
#define ARCHIVE_DELETE_READER_H

#include <memory>
#include "ReaderBase.h"
#include "ThreadPool.h"
#include "BlockBufferMap.h"

class ArchiveDeleteReader : public ReaderBase {
public:
    explicit ArchiveDeleteReader(const ReaderParams &deleteReaderParams);
    ~ArchiveDeleteReader();

    void SetArchiveClient(std::shared_ptr<ArchiveClientBase> client);
    BackupRetCode Start() override;
    BackupRetCode Destroy() override;
    BackupRetCode Abort() override;
    BackupPhaseStatus GetStatus() override;

private:
    int OpenFile(FileHandle& fileHandle) override;
    int ReadData(FileHandle& fileHandle) override;
    int ReadMeta(FileHandle& fileHandle) override;
    int CloseFile(FileHandle& fileHandle) override;

    void ThreadFunc() override;
    bool IsAbort();
    bool IsComplete() const;
    void HandleComplete() const;
    void PushToAggregator(FileHandle& fileHandle);

    std::shared_ptr<ArchiveRestoreAdvanceParams> m_advParams;
    std::shared_ptr<ArchiveClientBase> m_archiveClient { nullptr };
    std::mutex mtx {};
    std::thread m_thread;
    bool m_threadDone { false };
};
#endif // ARCHIVE_DELETE_READER_H