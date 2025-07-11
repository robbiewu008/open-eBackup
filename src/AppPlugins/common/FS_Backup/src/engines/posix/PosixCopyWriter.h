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
#ifndef POSIX_COPY_WRITER_H
#define POSIX_COPY_WRITER_H

#include <unordered_map>
#include "BlockBufferMap.h"
#include "ThreadPool.h"
#include "HostCopyWriter.h"
#include "log/BackupFailureRecorder.h"
#include "OsPlatformDefines.h"

class PosixCopyWriter : public HostCopyWriter {
public:
    explicit PosixCopyWriter(
        const WriterParams &copyWriterParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder = nullptr);
    ~PosixCopyWriter();

private:
    int WriteMeta(FileHandle& fileHandle) override;
    /* Hint:: can be unified later */
    void ProcessWriteEntries(FileHandle& fileHandle) override;
    /* Hint:: can be unified later */
    void ProcessWriteData(FileHandle& fileHandle) override;
    void CloseOpenedHandle() override;
};

#endif // POSIX_COPY_WRITER_H