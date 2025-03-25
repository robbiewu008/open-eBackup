/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file TaskPool.h
 * @author t00302329
 * @brief 实现IO调度的线程池
 * @version 0.1
 * @date 2021-01-11
 *
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