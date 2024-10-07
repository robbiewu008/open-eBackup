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
#ifndef POSIX_HARDLINK_READER_H
#define POSIX_HARDLINK_READER_H

#include <memory>
#include "HostHardlinkReader.h"
#include "ThreadPool.h"
#include "BlockBufferMap.h"
#include "OsPlatformDefines.h"
#include "log/BackupFailureRecorder.h"

class PosixHardlinkReader : public HostHardlinkReader {
public:
    explicit PosixHardlinkReader(
        const ReaderParams &hardlinkReaderParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder = nullptr);
    ~PosixHardlinkReader();

private:
    int ReadEmptyData(FileHandle& fileHandle) override;
    int ReadSymlinkData(FileHandle& fileHandle) override;
    int ReadNormalData(FileHandle& fileHandle) override;
 
    void ProcessReadEntries(FileHandle& fileHandle) override; /* state to event */
    void HandleSuccessEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr) override; /* event to state */
};

#endif // POSIX_HARDLINK_READER_H