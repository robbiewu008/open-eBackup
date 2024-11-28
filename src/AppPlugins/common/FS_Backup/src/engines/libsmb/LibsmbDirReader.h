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
#ifndef LIBSMB_DIR_READER_H
#define LIBSMB_DIR_READER_H

#include "ReaderBase.h"

#include <memory>

#include "BlockBufferMap.h"
#include "interface/LibsmbStructs.h"

class LibsmbDirReader : public ReaderBase {
public:
    explicit LibsmbDirReader(const ReaderParams &dirReaderParams);
    ~LibsmbDirReader();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
    BackupPhaseStatus GetStatus() override;

private:
    void ThreadFunc() override;

    int OpenFile(FileHandle &fileHandle) override;
    int ReadData(FileHandle &fileHandle) override;
    int ReadMeta(FileHandle &fileHandle) override;
    int CloseFile(FileHandle &fileHandle) override;

    bool IsAbort();
    bool IsComplete();
    void HandleComplete();

    std::shared_ptr<LibsmbBackupAdvanceParams> m_srcAdvParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> m_dstAdvParams;
    LibsmbParams m_params;
    std::atomic<uint64_t> m_readedDir { 0 };
    std::thread m_thread;

    bool m_threadDone { false };
    time_t m_isCompleteTimer { 0 };
};

#endif // LIBSMB_DIR_READER_H