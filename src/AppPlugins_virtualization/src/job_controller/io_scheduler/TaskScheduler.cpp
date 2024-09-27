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
#include "TaskScheduler.h"
#include <common/utils/Utils.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "log/Log.h"

namespace {
const std::string MODULE_NAME = "TaskScheduler";
}

namespace VirtPlugin {
TaskScheduler::~TaskScheduler()
{
    /* 清理队列中所有尚未处理的任务 */
    while (m_taskList.size() > 0) {
        std::shared_ptr<BlockTask> task;
        (void)Get(task);
    }
}

bool TaskScheduler::Put(std::shared_ptr<BlockTask> task, bool block, uint64_t ms)
{
    if (!task) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_taskList.push_back(task.get());
    }

    /* 封装任务，其中m_outq用于异步接收任务结果 */
    std::shared_ptr<BlockTask> wrapped = std::make_shared<TaskWrapper>(task, m_outq);
    return m_inq->Put(wrapped, block, ms);
}

bool TaskScheduler::Get(std::shared_ptr<BlockTask>& task, uint64_t ms)
{
    bool result = m_outq->Get(task, ms); // 从调度器队列中获取任务执行结果
    std::lock_guard<std::mutex> lock(m_mutex);
    m_taskList.remove(task.get());
    return result;
}

TaskScheduler::TaskWrapper::TaskWrapper(std::shared_ptr<BlockTask> task, TaskOutq outq)
    : m_task(task), m_taskOutQ(outq)
{}

int32_t TaskScheduler::TaskWrapper::Exec()
{
    if (m_task.get() == nullptr) {
        ERRLOG("Task obj is nullptr.");
        return FAILED;
    }
    m_task->SetBuffer(m_buffer);
    m_task->SetCalcHashBuffer(m_calcHashBuffer);
    m_task->SetReadHashBuffer(m_readHashBuffer);
    int32_t execCount = 0;
    uint64_t retryInterval = 3;
    const int32_t maxExecCount = 5;
    int32_t retValue = FAILED;

    while (execCount < maxExecCount) {
        retValue = m_task->Exec();
        if (retValue == SUCCESS) {
            break;
        }

        Utils::SleepSeconds(retryInterval);
        execCount++;
        ERRLOG("Task exec failed, try again.");
    }
    m_taskOutQ->Put(m_task);

    return retValue;
}
}