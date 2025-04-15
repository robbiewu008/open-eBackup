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
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <mutex>
#include <memory>
#include <queue>
#include "common/Semaphore.h"

/**
 *  @brief 实现容纳一个元素的双端队列
 */
template<class T>
class SingleSlotQueue {
public:
    SingleSlotQueue() : m_getMutex(), m_putMutex(), m_slot(NULL)
    {
        m_getMutex.lock();  // 初始时阻塞获取
    }

    ~SingleSlotQueue()
    {
        m_putMutex.try_lock();
        m_getMutex.try_lock();
        m_putMutex.unlock();
        m_getMutex.unlock();
    }

    /**
     *  @brief 将元素放入队列
     *  @param  item 放入元素对应的指针
     *  @param  block 放入时是否阻塞
     *  @param  ms 阻塞超时时间，单位毫秒
     *  @return true 成功; false 失败
     */
    bool Put(std::shared_ptr<T> item, bool block = true, uint64_t ms = 0)
    {
        bool result = true;
        if ((block) && (ms == 0)) {
            m_putMutex.lock();
        } else if (block) {
            std::chrono::milliseconds timeout(ms);
            result = m_putMutex.try_lock_for(timeout);
        } else {
            result = m_putMutex.try_lock();
        }

        if (result) {
            m_slot = item;
            m_getMutex.unlock();  // 允许获取
        }
        return result;
    }

    /**
     *  @brief 从队列取出元素
     *  @param  item 取出的元素
     *  @param  block 取出时是否阻塞
     *  @param  ms 取出时阻塞的超时时间，单位毫秒
     *  @return true 成功; false 失败
     */
    bool Get(std::shared_ptr<T>& item, bool block = true, uint64_t ms = 0)
    {
        bool result = true;
        if ((block) && (ms == 0)) {
            m_getMutex.lock();
        } else if (block) {
            std::chrono::milliseconds timeout(ms);
            result = m_getMutex.try_lock_for(timeout);
        } else {
            result = m_getMutex.try_lock();
        }

        if (result) {
            item = m_slot;
            m_slot.reset();
            m_putMutex.unlock();  // 允许放入
        }
        return result;
    }

private:
    std::timed_mutex m_getMutex;
    std::timed_mutex m_putMutex;
    std::shared_ptr<T> m_slot;
};


/**
 *  @brief 实现不限数量的双端队列
 */
template<class T>
class UnlimitedQueue {
public:
    UnlimitedQueue() : m_sem(0)
    {}
    ~UnlimitedQueue() = default;

    void Put(std::shared_ptr<T> item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(item);
        m_sem.Signal();
    }

    /**
     *  @brief 从队列取出元素
     *  @param  item 取出的元素
     *  @param  ms 取出时阻塞的超时时间，单位毫秒
     *  @return true 成功; false 失败
     */
    bool Get(std::shared_ptr<T>& item, uint64_t ms = 0)
    {
        bool result = true;
        if (ms > 0) {
            result = m_sem.TimedWait(ms);
        } else {
            m_sem.Wait();
        }

        if (result) {
            std::lock_guard<std::mutex> lock(m_mutex);
            item = m_queue.front();
            m_queue.pop();
        }
        return result;
    }

private:
    std::queue<std::shared_ptr<T>> m_queue;
    std::mutex m_mutex;
    Semaphore m_sem;
};

#endif