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
#include "Sha256.h"
#include <openssl/sha.h>
#include "define/Types.h"
#include "common/Constants.h"
#include "log/Log.h"
#include "securec.h"

namespace {
const int32_t EXEC_SUCCESS = 1;
}

namespace VirtPlugin {
CalculateSha256::CalculateSha256() {}

CalculateSha256::~CalculateSha256() {}

int32_t CalculateSha256::CalculateSha256Value(const std::shared_ptr<unsigned char[]>& pData, const uint64_t dataSize,
    std::shared_ptr<uint8_t[]> &outBuf)
{
    SHA256_CTX sha256;
    if (SHA256_Init(&sha256) != EXEC_SUCCESS) {
        ERRLOG("Init sha256 fialed.");
        return FAILED;
    }

    if (SHA256_Update(&sha256, pData.get(), dataSize) != EXEC_SUCCESS) {
        ERRLOG("Update sha256 fialed.");
        return FAILED;
    }

    if (SHA256_Final(outBuf.get(), &sha256) != EXEC_SUCCESS) {
        ERRLOG("Final sha256 fialed.");
        return FAILED;
    }

    return SUCCESS;
}

int32_t CalculateSha256::CalculateSha256Deviation(const uint64_t& inDev, uint64_t& outDev)
{
    uint64_t blockSize = inDev / DIRTY_RANGE_BLOCK_SIZE;
    outDev = blockSize * SHA256_DIGEST_LENGTH;

    return SUCCESS;
}

}
