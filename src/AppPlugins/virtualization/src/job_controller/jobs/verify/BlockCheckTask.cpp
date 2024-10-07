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
#include "BlockCheckTask.h"
#include <openssl/sha.h>
#include "common/sha256/Sha256.h"
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace VirtPlugin {
int32_t BlockCheckTask::Exec()
{
    if (m_repoHandler.get() == nullptr) {
        ERRLOG("Failed. the repohandler is nullptr.");
        return FAILED;
    }
    uint64_t startAddr = m_blockID * DIRTY_RANGE_BLOCK_SIZE;
    // 最后一个块不满4M
    uint64_t bufSize =
#ifdef WIN32
        static_cast<uint64_t>(DIRTY_RANGE_BLOCK_SIZE) > (m_imgSize - startAddr) ? (m_imgSize - startAddr) :
            static_cast<uint64_t>(DIRTY_RANGE_BLOCK_SIZE);
#else
        std::min(static_cast<uint64_t>(DIRTY_RANGE_BLOCK_SIZE), m_imgSize - startAddr);
#endif
    {
        // 涉及多线程操作同一文件指针，串行读
        std::lock_guard<std::mutex> lock(m_repoHandler->m_repoMutex);
        if (m_repoHandler->Seek(startAddr) != SUCCESS) {
            m_result = FAILED;
            ERRLOG("Failed to seek block: %llu.", m_blockID);
            return m_result;
        }
        size_t readSize = m_repoHandler->Read(m_buffer, bufSize);
        if (readSize != bufSize) {
            m_result = FAILED;
            ERRLOG("Failed to read block: %llu.", m_blockID);
            return m_result;
        }
    }

    std::shared_ptr<uint8_t[]> shaBuf = std::make_unique<uint8_t[]>(SHA256_DIGEST_LENGTH);
    if (CalculateSha256::CalculateSha256Value(m_buffer, bufSize, shaBuf) != SUCCESS) {
        m_result = FAILED;
        ERRLOG("Failed to calculate sha256 checksum for block: %llu.", m_blockID);
        return m_result;
    }
    if (memcmp(shaBuf.get(), m_blockCheckSum.get(), SHA256_DIGEST_LENGTH) != 0) {
        m_result = DAMAGED;
        ERRLOG("CheckSum mismatch, block: %llu.", m_blockID);
        return m_result;
    }
    DBGLOG("Check Block sha256 success, block id: %llu, start addr: %llu.", m_blockID, startAddr);

    return SUCCESS;
}
}
