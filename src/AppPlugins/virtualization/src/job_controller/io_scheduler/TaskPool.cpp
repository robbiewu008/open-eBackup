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
#include "TaskPool.h"
#include "securec.h"
#include "config_reader/ConfigIniReader.h"
#include "log/Log.h"

namespace {
constexpr uint32_t MAX_POOL_THREADS = 100;
constexpr uint32_t VOLUME_DATABLOCK_SIZE = 4194304;  // Byte
const std::string IO_TASK_POOL_THREADS = "IOTaskPoolThreads";
const std::string STRING_MAX_POOL_THREADS = "max_pool_threads";
const std::string MODULE_NAME = "TaskPool";
}

namespace VirtPlugin {
std::mutex TaskPool::m_dclMutex;
TaskPool* TaskPool::m_pInst = nullptr;

TaskPool* TaskPool::GetInstance()
{
    if (m_pInst == nullptr) {
        std::lock_guard<std::mutex> lock(m_dclMutex);
        if (m_pInst == nullptr) {
            uint32_t maxThrs = 0;
            maxThrs = Module::ConfigReader::getUint(IO_TASK_POOL_THREADS, STRING_MAX_POOL_THREADS);
            if (maxThrs == 0) {
                maxThrs = MAX_POOL_THREADS;
            }
            m_pInst = new (std::nothrow) TaskPool(maxThrs);
        }
    }

    return m_pInst;
}

TaskPool::TaskPool(size_t workers) : m_inq(), m_workers(workers), m_haltTask(new BlockTask())
{
    for (size_t i = 0; i < m_workers; i++) {
        std::shared_ptr<unsigned char[]> buffer(
            new unsigned char[VOLUME_DATABLOCK_SIZE], std::default_delete<unsigned char[]>());
        memset_s(buffer.get(), VOLUME_DATABLOCK_SIZE, 0, VOLUME_DATABLOCK_SIZE);
        std::shared_ptr<uint8_t[]> m_calcHashBuffer(
            new uint8_t[SHA256_DIGEST_LENGTH], std::default_delete<uint8_t[]>()
        );
        memset_s(m_calcHashBuffer.get(), SHA256_DIGEST_LENGTH, 0, SHA256_DIGEST_LENGTH);
        std::shared_ptr<uint8_t[]> m_readHashBuffer(
            new uint8_t[SHA256_DIGEST_LENGTH], std::default_delete<uint8_t[]>()
        );
        memset_s(m_readHashBuffer.get(), SHA256_DIGEST_LENGTH, 0, SHA256_DIGEST_LENGTH);
        m_threads.push_back(std::thread([=] { WorkerLoop(buffer, m_calcHashBuffer, m_readHashBuffer); }));
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
    m_pInst = nullptr;
}

void TaskPool::WorkerLoop(std::shared_ptr<unsigned char[]> buffer, std::shared_ptr<uint8_t[]> m_calcHashBuffer,
    std::shared_ptr<uint8_t[]> m_readHashBuffer)
{
    std::shared_ptr<BlockTask> task;
    while (true) {
        if (m_inq.Get(task)) {
            if (!task) {
                continue;
            }

            if (task.get() == m_haltTask.get()) {  // 收到退出消息
                INFOLOG("Taskloop is exit.");
                return;
            }

            task->SetBuffer(buffer);
            task->SetCalcHashBuffer(m_calcHashBuffer);
            task->SetReadHashBuffer(m_readHashBuffer);
            task->Exec();
        }
    }
}
}