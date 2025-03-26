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
#ifndef VOLUMEBACKUP_BLOCKING_QUEUE_H
#define VOLUMEBACKUP_BLOCKING_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

#include "common/VolumeProtectMacros.h"
#include "log/Log.h"

template<typename T>
class BlockingQueue {
public:
    BlockingQueue(std::size_t maxSize);

    bool BlockingPush(const T&);    // blocking push, return false if queue is set to finished

    bool BlockingPop(T&);           // blocking pop, return false if queue is set to finished and empty

    void Finish();

    bool TryBlockingPush(const T&); // non-blocking push

    bool TryBlockingPop(T&);        // non-blocking pop

    bool Empty();

    std::size_t Size();

private:
    std::queue<T>           m_queue;
    std::mutex              m_mutex;
    std::condition_variable m_notEmpty;
    std::condition_variable m_notFull;
    bool                    m_finished;
    std::size_t             m_maxSize;
};

template<typename T>
BlockingQueue<T>::BlockingQueue(std::size_t maxSize)
    : m_finished(false), m_maxSize(maxSize)
{}

/**
 * @brief blocking push an item, invoker thread will be blocked if queue is full
 * @tparam T
 * @param v
 * @return true if push successfully
 * @return false if queue is set to finish
 */
template<typename T>
bool BlockingQueue<T>::BlockingPush(const T &v)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (m_finished) {
        return false;
    }
    m_notFull.wait(lk, [&]() {
        return m_queue.size() < m_maxSize;
    });
    m_queue.push(v);
    DBGLOG("BlockingPush one, size remain %d", m_queue.size());
    m_notEmpty.notify_one();
    return true;
}

/**
 * @brief blocking pop an item from queue, invoker thread will be blocked if queue is empty
 * @tparam T
 * @param v
 * @return true if item is poped successfully
 * @return false if queue is empty and has been set to finished
 */
template<typename T>
bool BlockingQueue<T>::BlockingPop(T &v)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    m_notEmpty.wait(lk, [&]() {
        return !m_queue.empty() || m_finished;
    });
    if (m_queue.empty() && m_finished) {
        return false;
    }
    v = m_queue.front();
    m_queue.pop();
    DBGLOG("BlockingPop one, size remain %d", m_queue.size());
    m_notFull.notify_one();
    return true;
}

/**
 * @brief to mark the queue to finished
 * No more item should be pushed anymore and BlockingPop may return false once empty
 * @tparam T
 */
template<typename T>
void BlockingQueue<T>::Finish()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    m_finished = true;
    m_notFull.notify_all();
    m_notEmpty.notify_all();
}

/**
 * @brief try to push an item (non-blocking)
 * @tparam T
 * @param v
 * @return true if push successfully
 * @return false if push failed
 */
template<typename T>
bool BlockingQueue<T>::TryBlockingPush(const T& v)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (m_finished || m_queue.size() >= m_maxSize) {
        return false;
    }
    m_queue.push(v);
    m_notEmpty.notify_one();
    return true;
}

/**
 * @brief try to pop an item (non-blocking)
 * @tparam T
 * @param v
 * @return true if pop successfully
 * @return false if pop failed
 */
template<typename T>
bool BlockingQueue<T>::TryBlockingPop(T& v)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (m_queue.empty()) {
        return false;
    }
    v = m_queue.front();
    m_queue.pop();
    m_notFull.notify_one();
    return true;
}

template<typename T>
bool BlockingQueue<T>::Empty()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_queue.empty();
}

template<typename T>
std::size_t BlockingQueue<T>::Size()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_queue.size();
}

#endif