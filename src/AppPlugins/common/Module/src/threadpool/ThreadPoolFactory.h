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
#ifndef THREADPOOLFACTORY_H
#define THREADPOOLFACTORY_H

#include <string>
#include <unordered_map>
#include "ThreadPool.h"
#include "define/Defines.h"

namespace Module {

enum class ThreadPoolType {
    THREAD_POOL_TYPE_SCAN = 0,
    THREAD_POOL_TYPE_BACKUP
};

const int MAX_THREADS_CNT = 1000;
const int DEFAULT_THREADS_CNT = 32;
class AGENT_API ThreadPoolFactory {
public:
    static ThreadPool *GetThreadPoolInstance(const std::string& key, int num = DEFAULT_THREADS_CNT);
    static void DestoryThreadPool(const std::string& key);
    static void InitThreadPool(int scanThreadCnt, int backupThreadCnt);
    static bool IsThreadPoolAvailable(ThreadPoolType type);
private:
    static int m_scanThreadPoolCnt;
    static int m_backupThreadPoolCnt;
    static ThreadPool *m_scanThreadPool;
    static ThreadPool *m_backupThreadPool;
    static std::mutex mthreadPoolInstanceLock;
    static std::unordered_map<std::string, ThreadPool *> m_threadPoolMap;
};
} // namespace Module

#endif