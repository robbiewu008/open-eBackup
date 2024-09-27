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
#ifndef __BLOCK_TASK_H__
#define __BLOCK_TASK_H__

#include <memory>
#include "common/Macros.h"
#include "common/Constants.h"

namespace VirtPlugin {
class BlockTask {
public:
    BlockTask() : m_result(0){};
    virtual ~BlockTask() = default;

    /**
     *  @brief  执行读任务
     */
    virtual int32_t Exec()
    {
        return SUCCESS;
    };
    
    /**
     *  @brief  任务执行结果
     */
    inline int32_t Result() const
    {
        return m_result;
    }

    void SetBuffer(std::shared_ptr<unsigned char[]> buffer)
    {
        m_buffer = buffer;
    }

    void SetCalcHashBuffer(std::shared_ptr<uint8_t[]> buffer)
    {
        m_calcHashBuffer = buffer;
    }

    void SetReadHashBuffer(std::shared_ptr<uint8_t[]> buffer)
    {
        m_readHashBuffer = buffer;
    }

protected:
    int32_t m_result;
    std::shared_ptr<unsigned char[]> m_buffer;
    std::shared_ptr<uint8_t[]> m_calcHashBuffer;
    std::shared_ptr<uint8_t[]> m_readHashBuffer;
};
}

#endif