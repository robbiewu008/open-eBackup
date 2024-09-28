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
#include "BackupIoTask.h"
#include "log/Log.h"
#include "common/sha256/Sha256.h"


namespace {
const std::string MODULE_NAME = "BackupIoTask";
}

namespace VirtPlugin {
int32_t BackupIoTask::Exec()
{
    int32_t retValue = FAILED;
    m_result = FAILED;
    if (m_buffer.get() == nullptr) {
        ERRLOG("Volume or repository is not set for this io task.");
        return FAILED;
    }
    retValue = m_volumeHandler->ReadBlocks(m_startAddr, m_bufferSize, m_buffer, m_calcHashBuffer, m_readHashBuffer);
    if (retValue == FAILED) {
        ERRLOG("Read IO: m_startAddr=[%llu], m_bufferSize=[%llu], ret=[%d].", m_startAddr, m_bufferSize, retValue);
        return FAILED;
    } else if (retValue == DATA_SAME_IGNORE_WRITE || retValue == DATA_ALL_ZERO_IGNORE_WRITE) {
        GenCopyVerifySha256();
        m_result = retValue;
    } else {
        std::lock_guard<std::mutex> lock(m_repositoryHandler->m_repoMutex);
        retValue = m_repositoryHandler->Seek(m_startAddr);
        if (retValue != SUCCESS) {
            ERRLOG("Seek IO: m_startAddr=[%llu], ret=[%d].", m_startAddr, retValue);
            return FAILED;
        }
        retValue = m_repositoryHandler->Write(m_buffer, m_bufferSize);
        if (retValue != m_bufferSize) {
            ERRLOG("Write IO: m_startAddr=[%llu], m_bufferSize=[%llu], ret=[%d].", m_startAddr, m_bufferSize, retValue);
            return FAILED;
        }
        GenCopyVerifySha256();
        m_result = SUCCESS;
    }
    return SUCCESS;
}

void BackupIoTask::GenCopyVerifySha256()
{
    if (!m_isCopyVerify) {
        return;
    }
    m_shaDataBuff = std::make_unique<uint8_t[]>(SHA256_DIGEST_LENGTH);
    memset_s(m_shaDataBuff.get(), SHA256_DIGEST_LENGTH, 0, SHA256_DIGEST_LENGTH);
    if (m_volumeHandler->CalculateHashValue(m_buffer, m_calcHashBuffer, m_shaDataBuff, m_bufferSize) != SUCCESS) {
        m_calcSha256Success = false;
        ERRLOG("Volume calculate sha256 data falied. startaddr: %ld", m_startAddr);
    }
}

int32_t BackupIoTask::GetSha256Data(std::shared_ptr<unsigned char[]>& pBuffer, uint64_t& startAddr, bool& calcRet) const
{
    pBuffer = m_shaDataBuff;
    startAddr = m_startAddr;
    calcRet = m_calcSha256Success;
    return SUCCESS;
}
}
