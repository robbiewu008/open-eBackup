/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file Semaphore.h
 * @author t00302329
 * @brief 利用互斥锁和条件变量实现信号量
 * @version 0.1
 * @date 2021-01-09
 *
 */
#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    Semaphore(int value = 0) : m_count(value)
    {}

    ~Semaphore()
    {}

    /**
     *  @brief 等待信号量
     */
    void Wait()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_cv.wait(lock, [&]() -> bool { return m_count > 0; });
        --m_count;
    }

    /**
     *  @brief 超时等待信号量
     *  @param  ms 超时时间，单位毫秒
     *  @return true 成功; false 失败
     */
    bool TimedWait(uint64_t ms)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        if (!m_cv.wait_for(lock, std::chrono::milliseconds(ms), [&]() -> bool { return m_count > 0; })) {
            return false;
        }
        --m_count;
        return true;
    }

    /**
     *  @brief 释放信号量
     */
    void Signal()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        ++m_count;
        m_cv.notify_one();
    }

private:
    int m_count;
    std::mutex m_mtx;
    std::condition_variable m_cv;
};

#endif