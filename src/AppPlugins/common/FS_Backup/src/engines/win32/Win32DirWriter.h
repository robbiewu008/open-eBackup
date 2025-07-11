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
#ifndef WIN32_DIR_WRITER_H
#define WIN32_DIR_WRITER_H

#include <memory>
#include <string>
#include "HostDirWriter.h"
#include "BlockBufferMap.h"
#include "Win32ServiceTask.h"
#include "log/BackupFailureRecorder.h"
#include "OsPlatformDefines.h"

class Win32DirWriter : public HostDirWriter {
public:
    explicit Win32DirWriter(
        const WriterParams &dirWriterParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder);
    ~Win32DirWriter();
    
private:
    void HandleFailedEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr) override;
};

#endif  // WIN32_DIR_WRITER_H