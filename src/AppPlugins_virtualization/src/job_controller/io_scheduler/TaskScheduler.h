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
#ifndef __TASK_SCHEDULER_H__
#define __TASK_SCHEDULER_H__

#include "TaskPool.h"
#include "BlockTask.h"

namespace VirtPlugin {
class TaskScheduler {
public:
    explicit TaskScheduler(TaskPool &taskPool)
        : m_outq(new UnlimitedQueue<BlockTask>()), m_inq(taskPool.GetInputQueue())
    {}
    ~TaskScheduler();

    /**
     *  @brief  将任务对象交由调度器执行
     *  @param  task 要执行的任务对象
     *  @param  block 是否阻塞
     *  @param  ms 取出时阻塞的超时时间，单位毫秒
     *  @return true 成功; false 失败
     */
    bool Put(std::shared_ptr<BlockTask> task, bool block = true, uint64_t ms = 0);

    /**
     *  @brief  获取任务对象的执行结果
     *  @param  task 任务对象的结果
     *  @param  ms 取出时阻塞的超时时间，单位毫秒
     *  @return true 成功; false 失败
     */
    bool Get(std::shared_ptr<BlockTask>& task, uint64_t ms = 0);

private:
    TaskOutq m_outq; // 保存任务执行结果
    TaskInq* m_inq;  // 指向TaskPool的队列
    std::list<BlockTask*> m_taskList; // 保存任务对象
    std::mutex m_mutex; // 用于m_taskList的互斥访问

    class TaskWrapper : public BlockTask {
    public:
        TaskWrapper(std::shared_ptr<BlockTask> task, TaskOutq outq);
        ~TaskWrapper() override
        {}
        int32_t Exec() override;

    private:
        std::shared_ptr<BlockTask> m_task;
        TaskOutq m_taskOutQ;
    };
};
}
#endif