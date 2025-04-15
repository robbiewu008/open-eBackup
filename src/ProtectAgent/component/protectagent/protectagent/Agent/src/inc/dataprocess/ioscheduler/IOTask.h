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
#ifndef __IO_TASK_H__
#define __IO_TASK_H__

#include <cstdint>
#include <memory>
#include "common/Defines.h"
#include "Task.h"
#include "apps/vmwarenative/VMwareDef.h"
#include "IOEngine.h"


class IOTask : public Task {
public:
    IOTask(mp_int32 taskType, uint64_t startAddr, uint64_t bufSize)
        : m_taskType(taskType),
          m_startAddr(startAddr),
          m_bufferSize(bufSize)
    {}

    ~IOTask(){};

    void Exec() override;

    /**
     *  @brief  设置IO任务的读端
     *  @param  reader 读端
     */
    void Reader(std::shared_ptr<IOEngine> reader)
    {
        m_reader = reader;
    }

    /**
     *  @brief  设置IO任务的写端
     *  @param  writer 写端
     */
    void Writer(std::shared_ptr<IOEngine> writer)
    {
        m_writer = writer;
    }
    
    mp_string GetErrDesc()
    {
        return m_errDesc;
    }

private:
    mp_int32 m_taskType;   // 任务类型
    uint64_t m_startAddr;  // 块起始地址
    uint64_t m_bufferSize; // 块大小
    std::shared_ptr<IOEngine> m_reader;
    std::shared_ptr<IOEngine> m_writer;
    mp_string m_errDesc;
};

#endif