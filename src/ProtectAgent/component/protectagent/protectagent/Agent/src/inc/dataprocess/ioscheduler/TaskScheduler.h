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

class TaskScheduler {
public:
    TaskScheduler(TaskPool& taskPool);
    ~TaskScheduler();

    /**
     *  @brief  将任务对象交由调度器执行
     *  @param  task 要执行的任务对象
     *  @param  block 是否阻塞
     *  @param  ms 取出时阻塞的超时时间，单位毫秒
     *  @return true 成功; false 失败
     */
    bool Put(std::shared_ptr<Task> task, bool block = true, mp_uint64 ms = 0);

    /**
     *  @brief  获取任务对象的执行结果
     *  @param  task 任务对象的结果
     *  @param  block 是否阻塞
     *  @param  ms 取出时阻塞的超时时间，单位毫秒
     *  @return true 成功; false 失败
     */
    bool Get(std::shared_ptr<Task>& task, bool block = true, mp_uint64 ms = 0);

    /**
     *  @brief  执行任务对象
     *  @param  task 要执行的任务对象
     */
    void Exec(std::shared_ptr<Task>& task);

private:
    TaskOutq m_outq; // 保存任务执行结果
    TaskInq* m_inq;  // 指向TaskPool的队列
    std::list<Task*> m_taskList; // 保存任务对象
    std::mutex m_mutex; // 用于m_taskList的互斥访问

    class TaskWrapper : public Task {
    public:
        TaskWrapper(std::shared_ptr<Task> task, TaskOutq outq);
        virtual ~TaskWrapper()
        {}
        virtual void Exec();

    private:
        std::shared_ptr<Task> m_task;
        TaskOutq m_output; // 任务执行完后保存结果
    };
};

#endif