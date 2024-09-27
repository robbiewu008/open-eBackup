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
#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    Semaphore(int value = 0) : m_count(value)
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