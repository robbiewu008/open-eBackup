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
#include "HostServiceTask.h"

using namespace std;
using namespace FS_Backup;

HostServiceTask::~HostServiceTask()
{
    if (m_bufferMapPtr != nullptr) {
        m_bufferMapPtr.reset();
    }
}

HostServiceTask::HostServiceTask(
    HostEvent event,
    std::shared_ptr<BlockBufferMap> bufferMapPtr,
    FileHandle& fileHandle,
    const HostParams& params)
    : m_event(event), m_bufferMapPtr(bufferMapPtr), m_fileHandle(fileHandle), m_params(params) {}

void HostServiceTask::Exec()
{
    if (m_bufferMapPtr == nullptr) {
        return;
    }
    switch (m_event) {
        case HostEvent::OPEN_SRC: {
            return HandleOpenSrc();
        }
        case HostEvent::OPEN_DST: {
            return HandleOpenDst();
        }
        case HostEvent::READ_DATA: {
            return HandleReadData();
        }
        case HostEvent::READ_META: {
            return HandleReadMeta();
        }
        case HostEvent::WRITE_DATA: {
            return HandleWriteData();
        }
        case HostEvent::WRITE_META: {
            return HandleWriteMeta();
        }
        case HostEvent::LINK: {
            return HandleLink();
        }
        case HostEvent::CLOSE_SRC: {
            return HandleCloseSrc();
        }
        case HostEvent::CLOSE_DST: {
            return HandleCloseDst();
        }
        case HostEvent::DELETE_ITEM: {
            return HandleDelete();
        }
        case HostEvent::CREATE_DIR: {
            return HandleCreateDir();
        }
        default:
            break;
    }
}

bool HostServiceTask::IsCriticalError() const
{
    if (m_backupFailReason == BackupPhaseStatus::FAILED_NOACCESS ||
        m_backupFailReason == BackupPhaseStatus::FAILED_NOSPACE ||
        m_backupFailReason == BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE ||
        m_backupFailReason == BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE) {
        return true;
    }
    return false;
}
