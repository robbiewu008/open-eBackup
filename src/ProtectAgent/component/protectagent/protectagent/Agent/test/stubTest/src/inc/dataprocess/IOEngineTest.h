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
#ifndef __IO_ENGINE_H__
#define __IO_ENGINE_H__

#include <cstdint>
#include "common/Types.h"

class IOEngine {
public:
    virtual ~IOEngine() = default;
    /**
     *  @brief  指定位置读
     *  @param  offsetInBytes 偏移地址
     *  @param  bufferSizeInBytes 数据块大小
     *  @param  buffer 数据块
     */
    virtual mp_int32 Read(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer) = 0;

    /**
     *  @brief  指定位置写
     *  @param  offsetInBytes 偏移地址
     *  @param  bufferSizeInBytes 数据块大小
     *  @param  buffer 数据块
     */
    virtual mp_int32 Write(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer) = 0;
    
    /**
     *  @brief  打开IO引擎，默认为空处理
     */
    virtual mp_int32 Open()
    {
        return MP_SUCCESS;
    }

    /**
     *  @brief  关闭IO引擎，默认为空处理
     */
    virtual mp_int32 Close()
    {
        return MP_SUCCESS;
    }
    
    /**
     *  @brief  备份任务的后置处理，默认为空处理
     */
    virtual mp_int32 PostBackup()
    {
        return MP_SUCCESS;
    }

    /**
     *  @brief  恢复任务的后置处理，默认为空处理
     */
    virtual mp_int32 PostRecovery()
    {
        return MP_SUCCESS;
    }
};

#endif