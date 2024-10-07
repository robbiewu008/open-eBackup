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
#ifndef HOST_DELETE_READER_H
#define HOST_DELETE_READER_H

#include <memory>
#include "ReaderBase.h"
#include "BlockBufferMap.h"
#include "log/BackupFailureRecorder.h"
#include "OsPlatformDefines.h"

class HostDeleteReader : public ReaderBase {
public:
    explicit HostDeleteReader(
        const ReaderParams &deleteReaderParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder);
    virtual ~HostDeleteReader();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
    BackupPhaseStatus GetStatus() override;

private:
    void ThreadFunc() override;

    int OpenFile(FileHandle& fileHandle) override;
    int ReadData(FileHandle& fileHandle) override;
    int ReadMeta(FileHandle& fileHandle) override;
    int CloseFile(FileHandle& fileHandle) override;

    bool IsAbort() const;
    bool IsComplete();

private:
    HostParams      m_params;
    std::thread     m_thread;
    bool            m_threadDone { false };
    time_t          m_isCompleteTimer { 0 };
    std::shared_ptr<HostBackupAdvanceParams> m_srcAdvParams;
    std::shared_ptr<HostBackupAdvanceParams> m_dstAdvParams;
};
#endif  // HOST_DELETE_READER_H