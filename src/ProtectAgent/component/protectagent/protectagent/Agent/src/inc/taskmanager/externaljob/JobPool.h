/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file Job.h
 * @brief Implement for excuting external job pool
 * @version 1.1.0
 * @date 2021-10-29
 * @author wangguitao 00510599
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
