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
#ifndef _EXTERNAL_JOB_POOL_H
#define _EXTERNAL_JOB_POOL_H

#include <thread>
#include <vector>
#include <memory>
#include "common/Queue.h"
#include "taskmanager/externaljob/Job.h"

namespace AppProtect {
class JobPool {
public:
    JobPool();
    ~JobPool();

    mp_void CreatePool(mp_int32 poolSize);
    mp_void DestoryPool();

    // 放入一个任务
    void PushJob(std::shared_ptr<Job> pJob)
    {
        m_todoJobs.Put(pJob);
    }

private:
    void WorkerLoop();          // 线程函数，从队列获取任务并执行

private:
    std::vector<std::thread> m_threads; // 线程池工作线程
    std::shared_ptr<Job> m_haltFlag; // 用于通知线程池中线程的退出

    UnlimitedQueue<Job> m_todoJobs;                         // 待执行任务
};
}

#endif
