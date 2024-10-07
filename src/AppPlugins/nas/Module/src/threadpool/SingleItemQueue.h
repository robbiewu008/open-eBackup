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
#ifndef MODULE_SINGLEITEMQUEUE_H
#define MODULE_SINGLEITEMQUEUE_H

#include <queue>
#include <mutex>
#include <atomic>
#include<condition_variable>
#include "boost/chrono.hpp"
#include "boost/interprocess/sync/interprocess_semaphore.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/thread.hpp"

namespace Module
{

class Semaphore{
public:
    Semaphore(int count) : m_count(count) {}
    ~Semaphore() {}

    void Wait()
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        while (m_count == 0)
            m_condition.wait(locker);
        m_count--;
        return;
    }

    bool WaitFor(uint64_t milliseconds)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        std::chrono::milliseconds timeout(milliseconds);
        if (m_condition.wait_for(locker, timeout, [&]{if (m_count > 0) return true;})) {
            return false;
        }
        m_count--;
        return true;
    }

    bool TryWait()
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        if (m_count > 0) {
            m_count--;
            return true;
        } else {
            return false;
        }
    }

    void Signal()
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        ++m_count;
        m_condition.notify_one();
    }

private:
    int m_count { 0 };
    std::mutex m_mutex;
    std::condition_variable m_condition;
};

template<typename Type>
class SingleItemQueue
{
private:
    std::timed_mutex m_getMutex;
    std::timed_mutex m_putMutex;
    std::shared_ptr<Type> m_item;

public:
    SingleItemQueue() : m_getMutex(), m_putMutex(), m_item((Type*)NULL)
    {
        m_getMutex.lock();
    }

    ~SingleItemQueue()
    {
        m_putMutex.try_lock();
        m_getMutex.try_lock();
        m_putMutex.unlock();
        m_getMutex.unlock();
    }

    bool Put(std::shared_ptr<Type> item, bool block = true, uint64_t milliseconds = 0)
    {
        // Try acquiring the put mutex
        bool result = true;
        if ((block) && (milliseconds == 0)) {
            m_putMutex.lock();
        } else if (block) {
            std::chrono::milliseconds timeout(milliseconds);
            result = m_putMutex.try_lock_for(timeout);
        } else {
            result = m_putMutex.try_lock();
        }

        // If mutex acquired, insert item and unlock the get mutex to allow getting this item.
        if (result) {
            m_item = item;
            m_getMutex.unlock();
        }
        return result;
    }

    bool Get(std::shared_ptr<Type>& item, bool block = true, uint64_t milliseconds = 0)
    {
        // Try acquiring the get mutex
        bool result = true;
        if ((block) && (milliseconds == 0)) {
            m_getMutex.lock();
        } else if (block) {
            std::chrono::milliseconds timeout(milliseconds);
            result = m_getMutex.try_lock_for(timeout);
        } else {
            result = m_getMutex.try_lock();
        }

        // If mutex acquired, insert item and unlock the put mutex to allow getting this item.
        if (result) {
            item = m_item;
            m_item.reset((Type*)NULL);
            m_putMutex.unlock();
        }
        return result;
    }
};

template<typename Type>
class UnlimitedQueue
{
public:
    UnlimitedQueue() : m_queue(), m_mu(), m_sem(0) { }
    ~UnlimitedQueue() = default;

    void Put(std::shared_ptr<Type> item, bool block = true, uint64_t milliseconds = 0)
    {
        // Lock the internal lock, add item, and post semaphore
        block = block;
        milliseconds = milliseconds;
        std::lock_guard<std::mutex> lock(m_mu);
        m_queue.push(item);
        m_sem.post();
    }

    bool Get(std::shared_ptr<Type>& item, bool block = true, uint64_t milliseconds = 0)
    {
        // Try waiting for semaphore
        bool result = true;
        if ((block) && (milliseconds == 0)) {
            m_sem.wait();
        } else if (block) {
            boost::posix_time::ptime timeout = boost::posix_time::microsec_clock::universal_time() + boost::posix_time::milliseconds(milliseconds);
            result = m_sem.timed_wait(timeout);
        } else {
            result = m_sem.try_wait();
        }

        // If we decreased the semaphore, lock the internal lock, and pop the item
        if (result) {
            std::lock_guard<std::mutex> lock(m_mu);
            item = m_queue.front();
            m_queue.pop();
        }
        return result;
    }
private:
    std::queue<std::shared_ptr<Type>> m_queue;
    std::mutex m_mu;
    boost::interprocess::interprocess_semaphore m_sem;
};
} // namespace Module

#endif