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
#ifndef LIBNFS_DIRMETA_READER_H
#define LIBNFS_DIRMETA_READER_H

#include <memory>
#include "ReaderBase.h"
#include "ThreadPool.h"
#include "BlockBufferMap.h"
#include "LibnfsCommonMethods.h"

class LibnfsDirMetaReader : public ReaderBase {
public:
    explicit LibnfsDirMetaReader(const ReaderParams &dirReaderParams);
    ~LibnfsDirMetaReader();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
    BackupPhaseStatus GetStatus() override;

private:
    int OpenFile(FileHandle &fileHandle) override;
    int ReadData(FileHandle &fileHandle) override;
    int ReadMeta(FileHandle &fileHandle) override;
    int CloseFile(FileHandle &fileHandle) override;

    void ThreadFunc() override;
    bool IsComplete() const;
    void HandleComplete() const;

    void PushToAggregator(FileHandle& fileHandle);

    std::shared_ptr<LibnfsBackupAdvanceParams> m_advParams;
    NfsCommonData m_commonData {};
    std::mutex mtx {};
    std::thread m_thread;
    bool m_threadDone { false };
};

#endif // LIBNFS_DIRMETA_READER_H
