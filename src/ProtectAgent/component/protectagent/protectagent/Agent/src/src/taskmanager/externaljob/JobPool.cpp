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
#include "taskmanager/externaljob/JobPool.h"
#include "taskmanager/externaljob/PluginMainJob.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "common/Log.h"


namespace AppProtect {
constexpr int JOBPOOL_MAX_NUM = 50;

JobPool::JobPool() : m_haltFlag(std::make_shared<PluginMainJob>(PluginJobData()))
{
}

JobPool::~JobPool()
{
    DestoryPool();
}

mp_void JobPool::CreatePool(mp_int32 poolSize)
{
    if (poolSize > JOBPOOL_MAX_NUM) {
        WARNLOG("User-defined data(%d) exceeds the threshold, use default value.", poolSize);
        poolSize = JOBPOOL_MAX_NUM;
    }
    COMMLOG(OS_LOG_INFO, "create worker thread %d", poolSize);
    for (mp_int32 i = 0; i < poolSize; ++i) {
        m_threads.push_back(std::thread([this] { WorkerLoop(); }));
    }
}

mp_void JobPool::DestoryPool()
{
    for (auto& thr : m_threads) {
        m_todoJobs.Put(m_haltFlag);  // 通知所有工作线程退出
    }

    for (auto& thr : m_threads) {
        thr.join();  // 等待线程退出
    }
    m_threads.clear();
}

void JobPool::WorkerLoop()
{
    while (true) {
        std::shared_ptr<Job> pJob;
        if (m_todoJobs.Get(pJob)) {
            if (pJob.get() == nullptr) {
                COMMLOG(OS_LOG_WARN, "pJob is null");
                continue;
            }

            if (pJob.get() == m_haltFlag.get()) {  // 收到退出消息
                COMMLOG(OS_LOG_WARN, "Received halt task, all worker threads will be stoped");
                return;
            }

            pJob->Exec();
        }
    }
}
}
