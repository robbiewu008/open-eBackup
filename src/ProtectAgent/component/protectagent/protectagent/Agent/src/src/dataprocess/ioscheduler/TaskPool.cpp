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
#include <memory>
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "dataprocess/ioscheduler/TaskPool.h"

std::mutex TaskPool::m_dclMutex;
TaskPool* TaskPool::m_pInst = NULL;
namespace {
constexpr mp_uint32 MAX_POOL_THREADS = 16;
constexpr mp_uint32 TASKPOOL_MAX_NUM = 50;
}

EXTER_ATTACK TaskPool* TaskPool::GetInstance()
{
    if (m_pInst == NULL) {
        std::lock_guard<std::mutex> lock(m_dclMutex);
        if (m_pInst == NULL) {
            int maxThrs = MAX_POOL_THREADS;
            mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(
                CFG_DATAPROCESS_SECTION, "max_pool_threads", maxThrs);
            if (iRet != MP_SUCCESS || maxThrs > TASKPOOL_MAX_NUM) {
                COMMLOG(OS_LOG_ERROR, "Get max_pool_threads from configure file failed, iRet = %d.", iRet);
                maxThrs = TASKPOOL_MAX_NUM;
            }
            m_pInst = new (std::nothrow) TaskPool(maxThrs);
        }
    }

    return m_pInst;
}

TaskPool::TaskPool(size_t workers) : m_inq(), m_workers(workers), m_haltTask(new Task())
{
    const static int VMWARE_DATABLOCK_SIZE = 4194304;  // Byte
    for (size_t i = 0; i < m_workers; i++) {
        COMMLOG(OS_LOG_INFO, "Starting worker thread %d", i);
        std::shared_ptr<unsigned char[]> buffer(
            new unsigned char[VMWARE_DATABLOCK_SIZE], std::default_delete<unsigned char[]>());
        memset_s(buffer.get(), VMWARE_DATABLOCK_SIZE, 0, VMWARE_DATABLOCK_SIZE);
        m_threads.push_back(std::thread([=] { WorkerLoop(buffer); }));
    }
}

TaskPool::~TaskPool()
{
    for (size_t i = 0; i < m_workers; ++i) {
        m_inq.Put(m_haltTask);  // 通知所有工作线程退出
    }

    for (auto& thr : m_threads) {
        thr.join();  // 等待线程退出
    }

    delete m_pInst;
    m_pInst = NULL;
    COMMLOG(OS_LOG_INFO, "All worker threads in task pool are exited");
}

void TaskPool::WorkerLoop(std::shared_ptr<unsigned char[]> buffer)
{
    std::shared_ptr<Task> task;
    while (true) {
        if (m_inq.Get(task)) {
            if (!task) {
                COMMLOG(OS_LOG_WARN, "Task is null");
                continue;
            }

            if (task.get() == m_haltTask.get()) {  // 收到退出消息
                COMMLOG(OS_LOG_WARN, "Received halt task, all worker threads will be stoped");
                return;
            }

            task->SetBuffer(buffer);
            task->Exec();
        }
    }
}