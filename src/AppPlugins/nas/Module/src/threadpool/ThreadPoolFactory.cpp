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
#include "ThreadPoolFactory.h"
#include "Log.h"

using namespace std;
using namespace Module;

namespace {
    constexpr auto MODULE = "ThreadPoolFactory";
}

int ThreadPoolFactory::m_scanThreadPoolCnt = 0;
int ThreadPoolFactory::m_backupThreadPoolCnt = 0;
ThreadPool *ThreadPoolFactory::m_scanThreadPool = nullptr;
ThreadPool *ThreadPoolFactory::m_backupThreadPool = nullptr;
std::mutex ThreadPoolFactory::mthreadPoolInstanceLock {};
unordered_map<string, ThreadPool *> ThreadPoolFactory::m_threadPoolMap;

void ThreadPoolFactory::InitThreadPool(int scanThreadCnt, int backupThreadCnt)
{
    m_scanThreadPoolCnt = scanThreadCnt;
    m_backupThreadPoolCnt = backupThreadCnt;
}

ThreadPool *ThreadPoolFactory::GetThreadPoolInstance(const std::string& key, int num)
{
    lock_guard<std::mutex> lk(mthreadPoolInstanceLock);
    auto it = m_threadPoolMap.find(key);
    if (it != m_threadPoolMap.end()) {
        return it->second;
    }
    ThreadPool *tempThreadPool = new(nothrow) ThreadPool(num);
    if (tempThreadPool == nullptr) {
        HCP_Log(ERR, MODULE) << "Allocate memory for ThreadPool failed!" << HCPENDLOG;
        return nullptr;
    }
    m_threadPoolMap.emplace(key, tempThreadPool);
    return tempThreadPool;
}

void ThreadPoolFactory::DestoryThreadPool(const std::string& key)
{
    lock_guard<std::mutex> lk(mthreadPoolInstanceLock);
    auto it = m_threadPoolMap.find(key);
    if (it != m_threadPoolMap.end()) {
        delete it->second;
        m_threadPoolMap.erase(it);
    }
}

bool ThreadPoolFactory::IsThreadPoolAvailable(ThreadPoolType type)
{
    ThreadPool *threadPool = (type == ThreadPoolType::THREAD_POOL_TYPE_SCAN) ? m_scanThreadPool  : m_backupThreadPool;
    return (threadPool == nullptr) ? false : true;
}
