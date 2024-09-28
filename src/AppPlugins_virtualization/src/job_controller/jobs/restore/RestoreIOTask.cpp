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
#include "RestoreIOTask.h"

namespace {
constexpr auto MODULE = "RestoreIOTask";
}

namespace VirtPlugin {
int32_t RestoreIOTask::Exec()
{
    m_result = SUCCESS;
    int ret = m_isArchiveRestore ? ArchiveRestoreRead() : RestoreRead();
    if (ret != SUCCESS) {
        if (m_ignoreBadBlock) {
            WARNLOG("Ignore bad block data, start addr=%ld.", m_startAddr);
            return SUCCESS;
        } else {
            m_result = FAILED;
            ERRLOG("Restore failed for read data, start addr=%ld.", m_startAddr);
            return FAILED;
        }
    }
    {
        std::lock_guard<std::mutex> lock(m_volumeHander->m_volMutex);
        ret = m_volumeHander->WriteBlocks(m_startAddr, m_bufferSize, m_buffer);
        if (ret != SUCCESS) {
            m_result = FAILED;
            ERRLOG("Failed to writeBlocks to volume.");
            return m_result;
        }
    }
    return SUCCESS;
}

int32_t RestoreIOTask::RestoreRead()
{
    if (m_volumeHander.get() == nullptr || m_repoHandler.get() == nullptr || m_buffer.get() == nullptr) {
        ERRLOG("The handler is null.");
        return FAILED;
    }
    {
        // 加锁读取数据
        std::lock_guard<std::mutex> lock(m_repoHandler->m_repoMutex);
        if (m_repoHandler->Seek(m_startAddr) != SUCCESS) {
            ERRLOG("Failed to seek file handler.");
            return FAILED;
        }
        size_t readSize = m_repoHandler->Read(m_buffer, m_bufferSize);
        if (readSize != m_bufferSize) {
            ERRLOG("Failed to read data from repo.");
            return FAILED;
        }
    }
    return SUCCESS;
}
int32_t RestoreIOTask::ArchiveRestoreRead()
{
    if (m_volumeHander.get() == nullptr || m_clientHandler == nullptr || m_buffer.get() == nullptr) {
        ERRLOG("The handler is null.");
        return FAILED;
    }
    ArchiveStreamGetFileRsq retValue{};
    if (m_clientHandler->GetFileData(m_req, retValue) != MP_SUCCESS) {
        ERRLOG("GetFileData failed, filePath: %s, offset: %llu", WIPE_SENSITIVE(m_req.filePath).c_str(),
            retValue.offset);
        return FAILED;
    }
    DBGLOG("read data from s3. offset: %llu filesize: %d m_bufferSize: %llu isEnd: %d",
        retValue.offset, retValue.fileSize, m_bufferSize, retValue.readEnd);
    if (retValue.fileSize != m_bufferSize && retValue.readEnd == 0) {
        ERRLOG("read data from s3. offset: %llu filesize: %d m_bufferSize: %llu isEnd: %d",
            retValue.offset, retValue.fileSize, m_bufferSize, retValue.readEnd);
        return FAILED;
    }
    memcpy_s(m_buffer.get(),  retValue.fileSize, retValue.data,  retValue.fileSize);
    DBGLOG("ArchiveRestoreRead SUCCESS.");
    free(retValue.data);
    retValue.data = nullptr;
    return SUCCESS;
}

void RestoreIOTask::SetIgnoreBadBlockFlag(bool flag)
{
    m_ignoreBadBlock = flag;
}
}
