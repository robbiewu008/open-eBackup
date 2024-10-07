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
#ifndef APPPLUGIN_FILE_SEMAPHORE_H
#define APPPLUGIN_FILE_SEMAPHORE_H
#include <mutex>
#include <condition_variable>
namespace {
    const int MUTEX_SIZE = 1;
}

class Semaphore {
public:
    explicit Semaphore(int count) : m_count(count)
    {}

    void Signal()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        ++m_count;
        m_cv.notify_one();
    }

    void Wait()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [=] { return m_count > 0; });
        --m_count;
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    int m_count { MUTEX_SIZE };
};
#endif