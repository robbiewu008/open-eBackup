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
#ifndef __TASK_POOL_H__
#define __TASK_POOL_H__

#include <thread>
#include <list>
#include <memory>
#include "common/Queue.h"
#include "common/Defines.h"
#include "Task.h"

using TaskOutq = std::shared_ptr<UnlimitedQueue<Task>>;
using TaskInq = SingleSlotQueue<Task>;

class TaskPool {
public:
    EXTER_ATTACK static TaskPool* GetInstance();
    ~TaskPool();
    
    /**
     *  @brief  获取线程池的输入队列
     */
    TaskInq* GetInputQueue()
    {
        return &m_inq;
    }
public:
    TaskInq m_inq;
private:
    TaskPool(size_t workers);
    TaskPool(const TaskPool &) = delete;
    TaskPool& operator=(const TaskPool &) = delete;
    TaskPool(TaskPool &&) = delete;
    TaskPool& operator=(TaskPool &&) = delete;
    void WorkerLoop(std::shared_ptr<unsigned char[]> buffer);

    size_t m_workers;
    std::list<std::thread> m_threads; // 线程池工作线程
    std::shared_ptr<Task> m_haltTask; // 用于通知线程池中线程的退出
    static std::mutex m_dclMutex;
    static TaskPool* m_pInst;
};

#endif