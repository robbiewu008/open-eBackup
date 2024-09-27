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
#ifndef CALCULATE_SHA_256_H
#define CALCULATE_SHA_256_H

#include <string>
#include <memory>
#include "common/Macros.h"

namespace VirtPlugin {
class CalculateSha256 {
public:
    CalculateSha256();
    ~CalculateSha256();

    /**
     *  @brief 根据4M块数据生成对应的Sha256值
     *
     *  @param pData 待计算的数据
     *  @param dataSize 待计算数据长度
     *  @param outBuf 输出sha256值
     *  @return 错误码：0 成功，非0 失败
     */
    static int32_t CalculateSha256Value(const std::shared_ptr<unsigned char[]>& pData, const uint64_t dataSize,
        std::shared_ptr<uint8_t[]> &outBuf);

    /**
     *  @brief 根据偏移量计算出sha256文件偏移量
     *
     *  @param inDev 磁盘数据偏移量
     *  @param outDev Sha256文件偏移量
     *  @return 错误码：0 成功，非0 失败
     */
    static int32_t CalculateSha256Deviation(const uint64_t& inDev, uint64_t& outDev);
};
}
#endif