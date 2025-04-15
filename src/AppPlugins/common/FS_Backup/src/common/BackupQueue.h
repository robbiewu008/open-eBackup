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
#ifndef BACKUPQUEUE_H
#define BACKUPQUEUE_H

#include <queue>
#include <map>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>

struct BackupQueueConfig {
    uint64_t maxSize;
    uint64_t maxMemorySize;
};

template<typename T>
class BackupQueue {
private:
    mutable std::mutex m_mut;
    std::queue<T> m_dataQueue;
    std::condition_variable m_popcv;
    std::condition_variable m_pushcv;
    std::atomic<bool> m_blockPushFlag {false};
    std::atomic<bool> m_blockPopFlag {false};
    std::condition_variable m_blockPushCv;
    std::condition_variable m_blockPopCv;
    uint64_t m_maxSize;
    uint64_t m_maxMemorySize;
    std::function<bool(void *)> m_predicateCb;
    void* m_predicateCbData = nullptr;

public:
    explicit BackupQueue(BackupQueueConfig config)
    {
        m_maxSize = config.maxSize;
        m_maxMemorySize = config.maxMemorySize;
    }
    BackupQueue(BackupQueue const& other)
    {
        std::lock_guard<std::mutex> lk(other.m_mut);
        m_maxSize = other.m_maxSize;
        m_maxMemorySize = other.m_maxMemorySize;
        swap(m_dataQueue, other.m_dataQueue);
    }
    BackupQueue& operator = (const BackupQueue& other)
    {
        if (this != &other) {
            std::lock_guard<std::mutex> lk(other.m_mut);
            m_maxSize = other.m_maxSize;
            m_maxMemorySize = other.m_maxMemorySize;
            swap(m_dataQueue, other.m_dataQueue);
        }
        return *this;
    }

    void RegisterPredicate(std::function<bool(void *)> predicateCb, void* predicateCbData)
    {
        std::lock_guard<std::mutex> lk(m_mut);
        m_predicateCb = predicateCb;
        m_predicateCbData = predicateCbData;
        return;
    }

    void NotifyPredicate()
    {
        std::lock_guard<std::mutex> lk(m_mut);
        m_pushcv.notify_one();
        return;
    }

    bool Empty() const
    {
        std::lock_guard<std::mutex> lk(m_mut);
        return m_dataQueue.empty();
    }

    void SetSize(uint64_t size)
    {
        m_maxSize = size;
    }

    void BlockPush()
    {
        m_blockPushFlag = true;
    }

    void BlockPop()
    {
        m_blockPopFlag = true;
    }

    void CancelBlockPush()
    {
        if (m_blockPushFlag == true) {
            m_blockPushFlag = false;
            m_blockPushCv.notify_one();
        }
    }

    void CancelBlockPop()
    {
        if (m_blockPopFlag == true) {
            m_blockPopFlag = false;
            m_blockPopCv.notify_one();
        }
    }

    uint64_t GetMaxSize()
    {
        return m_maxSize;
    }

    uint64_t GetSize() const
    {
        std::lock_guard<std::mutex> lk(m_mut);
        return m_dataQueue.size();
    }

    uint64_t GetSizeWithOutLock() const
    {
        return m_dataQueue.size();
    }

    bool GetPredicateResult() const
    {
        if (m_predicateCb != nullptr) {
            /* user registered predicate */
            return m_predicateCb(m_predicateCbData);
        } else {
            /* default predicate */
            return m_dataQueue.size() < m_maxSize;
        }
    }

    void WaitAndPush(T new_value)
    {
        CheckBlockPush();
        bool isFull = false;
        {
            std::lock_guard<std::mutex> lk(m_mut);
            isFull = m_dataQueue.size() >= m_maxSize;
        }
        if (isFull) {
            std::unique_lock<std::mutex> lk(m_mut);
            m_pushcv.wait(lk, [this] { return GetPredicateResult(); });

            m_dataQueue.push(new_value);
            m_popcv.notify_one();
        } else {
            std::lock_guard<std::mutex> lk(m_mut);
            m_dataQueue.push(new_value);
            m_popcv.notify_one();
        }
    }

    bool WaitAndPush(T new_value, uint32_t timeOut)
    {
        bool isFull = false;
        {
            std::lock_guard<std::mutex> lk(m_mut);
            isFull = m_dataQueue.size() >= m_maxSize;
        }
        if (isFull) {
            // full , can not push anyway, wait for cv to notify can push
            auto const endTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeOut);
            std::unique_lock<std::mutex> lk(m_mut);
            if (m_pushcv.wait_until(lk, endTime, [this] { return GetPredicateResult(); }) && !m_blockPushFlag) {
                m_dataQueue.push(new_value);
                m_popcv.notify_one();
                return true;
            } else {
                // means timeout
                return false;
            }
        } else {
            // not full, push directly
            // here means has something to push, but block push api is called. if cancel block push api is not calling,
            // wait for timeout then return
            if (m_blockPushFlag) {
                auto const endTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeOut);
                std::unique_lock<std::mutex> lk(m_mut);
                if (m_blockPushCv.wait_until(lk, endTime, [this] { return !m_blockPushFlag; })) {
                    // cancelblockpop api is calling.
                    m_dataQueue.push(new_value);
                    m_popcv.notify_one();
                    return true;
                } else {
                    // means timeout
                    return false;
                }
            } else {
                // block push api is not called, push directly
                std::lock_guard<std::mutex> lk(m_mut);
                m_dataQueue.push(new_value);
                m_popcv.notify_one();
                return true;
            }
            // never call
            return false;
        }
    }

    void Push(T new_value)
    {
        std::lock_guard<std::mutex> lk(m_mut);
        m_dataQueue.push(new_value);
        m_popcv.notify_one();
    }

    void WaitAndPop(T& value)
    {
        CheckBlockPop();
        std::unique_lock<std::mutex> lk(m_mut);
        m_popcv.wait(lk, [this] { return !m_dataQueue.empty(); });
        value = m_dataQueue.front();
        m_dataQueue.pop();
        m_pushcv.notify_one();
    }

    T WaitAndPop()
    {
        CheckBlockPop();
        std::unique_lock<std::mutex> lk(m_mut);
        m_popcv.wait(lk, [this] { return !m_dataQueue.empty(); });
        T res = m_dataQueue.front();
        m_dataQueue.pop();
        m_pushcv.notify_one();
        return res;
    }

    // timeOut ms
    bool WaitAndPop(T& value, uint32_t timeOut)
    {
        auto deadLine = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeOut);
        std::unique_lock<std::mutex> lk(m_mut);
        // this cv means pop waiting for push, if nothing to pop, wait for timeout then return.
        if (m_popcv.wait_until(lk, deadLine, [this] { return !m_dataQueue.empty(); })) {
            // here means has something to pop, but block pop api is called. if cancel block pop api is not calling,
            // wait for timeout then return
            if (m_blockPopFlag) {
                deadLine = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeOut);
                if (m_blockPopCv.wait_until(lk, deadLine, [this] { return !m_blockPopFlag; })) {
                    // cancelblockpop api is calling.
                    value = m_dataQueue.front();
                    m_dataQueue.pop();
                    m_pushcv.notify_one();
                    return true;
                } else {
                    // means timeout
                    return false;
                }
            } else {
                value = m_dataQueue.front();
                m_dataQueue.pop();
                m_pushcv.notify_one();
                return true;
            }
        } else {
            // means time out
            return false;
        }
        // never call
        return false;
    }

    bool TryPop(T& value)
    {
        std::lock_guard<std::mutex> lk(m_mut);
        if (m_dataQueue.empty()) {
            return false;
        }
        value = m_dataQueue.front();
        m_dataQueue.pop();
        m_pushcv.notify_one();
        return true;
    }

    bool Front(T &value) const
    {
        std::lock_guard<std::mutex> lk(m_mut);
        if (m_dataQueue.empty()) {
            return false;
        }
        value = m_dataQueue.front();
        return true;
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lk(m_mut);
        while (!m_dataQueue.empty()) {
            m_dataQueue.pop();
        }
    }

    void CheckBlockPush()
    {
        if (!m_blockPushFlag) {
            return;
        }
        std::unique_lock<std::mutex> lk(m_mut);
        m_blockPushCv.wait(lk, [this] { return !m_blockPushFlag; });
    }

    void CheckBlockPop()
    {
        if (!m_blockPopFlag) {
            return;
        }
        std::unique_lock<std::mutex> lk(m_mut);
        m_blockPopCv.wait(lk, [this] { return !m_blockPopFlag; });
    }
};

template<typename T>
class BackupCacheQueue {
public:
    void Push(const std::string& s, const T& value) const
    {
        std::lock_guard<std::mutex> lk(m_mut);
        m_map[s].push(value);
    }

    bool Pop(const std::string& s, T& value) const
    {
        std::lock_guard<std::mutex> lk(m_mut);
        if (m_map.count(s) == 0) {
            return false;
        }
        if (m_map[s].empty()) {
            auto it = m_map.find(s);
            m_map.erase(it);
            return false;
        }
        value = m_map[s].front();
        m_map[s].pop();
        return true;
    }

    size_t Size(const std::string& s) const
    {
        std::lock_guard<std::mutex> lk(m_mut);
        if (m_map.count(s) == 0) {
            return 0;
        }
        return m_map[s].size();
    }

private:
    std::mutex m_mut;
    std::map<std::string, std::queue<T>> m_map;
};
#endif