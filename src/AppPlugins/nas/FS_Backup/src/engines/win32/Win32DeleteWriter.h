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
#ifndef WIN32_DELETE_WRITER_H
#define WIN32_DELETE_WRITER_H

#include <memory>
#include <string>

#include "HostDeleteWriter.h"
#include "BlockBufferMap.h"
#include "log/BackupFailureRecorder.h"
#include "OsPlatformDefines.h"

class Win32DeleteWriter : public HostDeleteWriter {
public:
    explicit Win32DeleteWriter(const WriterParams &deleteWriterParams,
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder);
};

#endif  // WIN32_DELETE_WRITER_H