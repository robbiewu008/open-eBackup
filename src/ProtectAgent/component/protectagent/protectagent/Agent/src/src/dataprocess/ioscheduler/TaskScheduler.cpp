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
#include "dataprocess/ioscheduler/TaskScheduler.h"
#include "common/Log.h"

TaskScheduler::TaskScheduler(TaskPool& taskPool) : m_outq(new UnlimitedQueue<Task>()), m_inq(taskPool.GetInputQueue())
{}

TaskScheduler::~TaskScheduler()
{
    /* 清理队列中所有尚未处理的任务 */
    while (m_taskList.size() > 0) {
        std::shared_ptr<Task> task;
        (void)Get(task);
    }
}

bool TaskScheduler::Put(std::shared_ptr<Task> task, bool block, mp_uint64 ms)
{
    if (!task) {
        COMMLOG(OS_LOG_ERROR, "Task is invalid");
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_taskList.push_back(task.get());
    }

    /* 封装任务，其中m_outq用于异步接收任务结果 */
    std::shared_ptr<Task> wrapped = std::make_shared<TaskWrapper>(task, m_outq);
    return m_inq->Put(wrapped, block, ms);
}

bool TaskScheduler::Get(std::shared_ptr<Task>& task, bool block, mp_uint64 ms)
{
    bool result = m_outq->Get(task, ms); // 从调度器队列中获取任务执行结果
    std::lock_guard<std::mutex> lock(m_mutex);
    m_taskList.remove(task.get());
    return result;
}

void TaskScheduler::Exec(std::shared_ptr<Task>& task)
{
    if (task.get() == NULL) {
        COMMLOG(OS_LOG_ERROR, "Task is invalid");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_taskList.push_back(task.get());
    }
    TaskWrapper wrapper(task, m_outq);
    wrapper.Exec();
}

TaskScheduler::TaskWrapper::TaskWrapper(std::shared_ptr<Task> task, TaskOutq output)
    : m_task(task), m_output(output)
{}

void TaskScheduler::TaskWrapper::Exec()
{
    if (m_task.get() == NULL) {
        COMMLOG(OS_LOG_ERROR, "Task is invalid");
        return;
    }
    m_task->SetBuffer(m_buffer);
    m_task->Exec();
    m_output->Put(m_task);
}