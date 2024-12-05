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
#ifndef WIN32_COPY_READER_H
#define WIN32_COPY_READER_H

#include "ReaderBase.h"
#include "ThreadPool.h"
#include "BlockBufferMap.h"
#include "OsPlatformDefines.h"
#include "HostCopyReader.h"
#include "log/BackupFailureRecorder.h"

class Win32CopyReader : public HostCopyReader {
public:
    explicit Win32CopyReader(
        const ReaderParams &copyReaderParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder);
    ~Win32CopyReader();

private:
    int ReadMeta(FileHandle& fileHandle) override;
    int ReadEmptyData(FileHandle& fileHandle);
    void ProcessReadEntries(FileHandle& fileHandle) override; /* state to event */
    void HandleSuccessEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr);
    bool IsComplete() override;
    bool HandleAdsFile(FileHandle& fileHandle);
    void CloseOpenedHandle() override;
};

#endif  // WIN32_COPY_READER_H