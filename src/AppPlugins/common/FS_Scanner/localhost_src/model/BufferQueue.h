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
#ifndef FS_SCANNER_BUFFER_QUEUE_H
#define FS_SCANNER_BUFFER_QUEUE_H

#include <queue>
#include <memory>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include "IterableQueue.h"

template<typename T>
class BufferQueue {
public:
    BufferQueue(uint32_t max) : m_maxSize(max) {}
    ~BufferQueue() {}

    /* Non-blocking API */
    void Push(T t)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_que.push(t);
        m_notEmpty.notify_one();
        m_pushCount++;
    }

    bool Pop(T& t)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_que.empty()) {
            return false;
        }
        t = m_que.front();
        m_que.pop();
        m_notFull.notify_one();
        m_popCount++;
        return true;
    }

    void PushBatch(std::vector<T> tlist)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        for (T t : tlist) {
            m_que.push(t);
            m_pushCount++;
        }
        m_notEmpty.notify_all();
    }

    std::vector<T> PopBatch(uint16_t count)
    {
        std::vector<T> tlist {};
        T t {};
        std::lock_guard<std::mutex> lock(m_mtx);
        while (!m_que.empty()) {
            t = m_que.front();
            m_que.pop();
            tlist.push_back(t);
            m_popCount++;
            if (tlist.size() == count) {
                break;
            }
        }
        m_notFull.notify_all();
        return tlist;
    }

    /* Blocking API */
    void BlockingPush(T t)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_notFull.wait(lock, [this] { return m_que.size() < m_maxSize; });
        m_que.push(t);
        m_pushCount++;
        m_notEmpty.notify_one();
    }

    void BlockingPop(T& t)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_notEmpty.wait(lock, [this] { return !m_que.empty(); });
        t = m_que.front();
        m_que.pop();
        m_popCount++;
        m_notFull.notify_one();
    }

    void BlockingPushBatch(std::vector<T> tlist)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_notFull.wait(lock, [this, &tlist] { return (m_que.size() + tlist.size()) <= m_maxSize; });
        for (T t : tlist) {
            m_que.push(t);
            m_pushCount++;
        }
        m_notEmpty.notify_all();
    }

    std::vector<T> BlockingPopBatch(uint16_t count)
    {
        std::vector<T> tlist {};
        T t {};
        std::unique_lock<std::mutex> lock(m_mtx);
        m_notEmpty.wait(lock, [this, &count] { return m_que.size() >= count; });
        while (tlist.size() != count) {
            t = m_que.front();
            m_que.pop();
            tlist.push_back(t);
            m_popCount++;
        }
        m_notFull.notify_all();
        return tlist;
    }

    /* Timed blocking API */
    bool BlockingPush(T t, uint32_t timeout)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        if (m_notFull.wait_for(lock, std::chrono::milliseconds(timeout),
            [this] { return m_que.size() < m_maxSize; })) {
            m_que.push(t);
            m_pushCount++;
            m_notEmpty.notify_one();
            return true;
        } else {
            return false;
        }
    }

    bool BlockingPop(T& t, uint32_t timeout)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        if (m_notEmpty.wait_for(lock, std::chrono::milliseconds(timeout),
            [this] { return !m_que.empty(); })) {
            t = m_que.front();
            m_que.pop();
            m_popCount++;
            m_notFull.notify_one();
            return true;
        } else {
            return false;
        }
    }

    bool BlockingPushBatch(std::vector<T> tlist, uint32_t timeout)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        if (m_notFull.wait_for(lock, std::chrono::milliseconds(timeout),
            [this, &tlist] { return (m_que.size() + tlist.size()) <= m_maxSize; })) {
            for (T t : tlist) {
                m_que.push(t);
                m_pushCount++;
            }
            m_notEmpty.notify_all();
            return true;
        } else {
            return false;
        }
    }

    bool BlockingPopBatch(std::vector<T>& tlist, uint16_t count, uint32_t timeout)
    {
        T t {};
        std::unique_lock<std::mutex> lock(m_mtx);
        if (m_notEmpty.wait_for(lock, std::chrono::milliseconds(timeout),
            [this, &count] { return m_que.size() >= 1; })) {
            while (tlist.size() != count) {
                if (m_que.empty()) {
                    break;
                }
                t = m_que.front();
                m_que.pop();
                tlist.push_back(t);
                m_popCount++;
            }
            m_notFull.notify_all();
            return true;
        } else {
            return false;
        }
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_que.empty();
    }

    bool Full()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_maxSize <= m_que.size();
    }

    uint32_t GetPopCount()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_popCount;
    }

    uint32_t GetPushCount()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_pushCount;
    }

    uint32_t GetSize()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_que.size();
    }

    // return true if no more data will be pushed in
    bool IsPushFinished()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_isPushFinished;
    }

    // once invoked. no more data will be pushed in
    void SetPushFinished()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_isPushFinished = true;
    }

    // once invoked, the buffer can be closed
    void SetConsumeFinished()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_consumeFinshed = true;
    }

    // return true if buffer can be closed
    bool IsConsumeFinished()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_consumeFinshed;
    }

    IterableQueue<T> m_que {};
private:
    std::mutex m_mtx;
    std::condition_variable m_notFull;
    std::condition_variable m_notEmpty;
    uint32_t m_popCount = 0;
    uint32_t m_pushCount = 0;
    uint32_t m_maxSize = 0;
    uint64_t m_maxDataSize = 0;

    /*
     * to mark there will be no more data in
     */
    bool m_isPushFinished {false};
    /*
     * to mark all data consumed,
     * used to be set after m_isFinished is set and left data is all consumed
     */
    bool m_consumeFinshed {false};
};

#endif