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
#include "ThreadPool.h"
#include "Log.h"

using namespace std;

namespace Module {
    std::mutex tp_mutex;
    template<typename O, typename A>
    inline void Remove(O& o, A a)
    {
        lock_guard<std::mutex> lock(tp_mutex);
        o.remove(a);
    }

    template<typename O, typename A>
    inline void PushBack(O& o, A a)
    {
        lock_guard<std::mutex> lock(tp_mutex);
        o.push_back(a);
    }

    template<typename O>
    inline size_t Size(O& o)
    {
        lock_guard<std::mutex> lock(tp_mutex);
        return o.size();
    }

    std::mutex g_poolLock;

    void CreateThreads(ThreadPool* pool, size_t nThread)
    {
        lock_guard<std::mutex> lock(g_poolLock);
        DBGLOG("CreateThreads");
        if (nThread == 0) {
            HCP_Logger_noid(ERR, "BackupNode") << "nThreads is zero."<< HCPENDLOG;
            return;
        }
        HCP_Logger_noid(INFO, "BackupNode") << "Create thread:"<<nThread<< HCPENDLOG;
        pool->m_Threads = new std::thread* [nThread];
        pool->m_threadNum = nThread;
        pool->m_threadWorkingFlags = new std::atomic<bool>[nThread]{false}; // 初始化每个线程的停止标志
        if (NULL == pool->m_Threads) {
            HCP_Logger_noid(ERR, "BackupNode") << "new fault,m_threads is NULL."<< HCPENDLOG;
            return;
        }
        for (size_t i = 0; i < nThread; ++i) {
            pool->m_Threads[i] = new std::thread([pool, i]() {
            pool->WorkerLoop(i);
            });
        }
        DBGLOG("CreateThreads success");
    }

    // 获取线程工作状态
    bool GetThreadWorkStatus(ThreadPool* pool, size_t threadID)
    {
        std::chrono::milliseconds checkInterval(CHECK_WORK_STATUS_INTERVAL);  //一秒钟, 每0.1秒检查一次

        for (int i = 0; i < CHECK_WORK_STATUS_MAX_TIMES; ++i) {
            if (!(pool->m_threadWorkingFlags[threadID])) {
                return false;
            }
            std::this_thread::sleep_for(checkInterval);
        }
        return true;
    }

    void DeleteThreads(ThreadPool* pool)
    {
        lock_guard<std::mutex> lock(g_poolLock);
        DBGLOG("DeleteThreads");
        for (size_t i = 0; i < pool->m_threadNum; ++i) {
            pool->m_input.Put(pool->m_haltItem, true, PUT_HALTITEM_IN_POOL_TIME_LIMIT);
        }
        if (NULL == pool->m_Threads) {
            HCP_Logger_noid(ERR, "BackupNode") << "m_threads is NULL."<< HCPENDLOG;
            return;
        }
        for (size_t i = 0; i < pool->m_threadNum; ++i) {
            if (NULL == pool->m_Threads[i]) {
                continue;
            }
            // 根据线程工作状态使用join或detach方法
            
            HCP_Logger_noid(INFO, "BackupNode") << "Joining thread " << i << HCPENDLOG;
            pool->m_Threads[i]->join();
           
            delete pool->m_Threads[i];
            pool->m_Threads[i] = NULL;
        }

        delete []pool->m_Threads;
        pool->m_Threads = NULL;
        DBGLOG("DeleteThreads success");
    }

    // new in constructor for class 'ThreadPool' which has no assignment operator, and has no copy constructor
    ThreadPool::ThreadPool(size_t nThreads)
        : m_input(), m_haltItem(new ExecutableItem())
    {
        DBGLOG("ThreadPool create");
        CreateThreads(this, nThreads);
    }

    ThreadPool::~ThreadPool()
    {
        DBGLOG("ThreadPool destory");
        DeleteThreads(this);
    }

    SingleItemQueue<ExecutableItem>* ThreadPool::GetInputQueue()
    {
        return &m_input;
    }

    void ThreadPool::Run(shared_ptr<ExecutableItem>& item)
    {
        item->Exec();
    }

    void ThreadPool::WorkerLoop(size_t threadID)
    {
        shared_ptr<ExecutableItem> item;
        m_threadWorkingFlags[threadID] = true;  // 线程开始工作
        while (true) {
            try {
                if (m_input.Get(item)) {
                    if (!item) {
                        HCP_Logger_noid(ERR, "BackupNode") << "item is null" << HCPENDLOG;
                        m_threadWorkingFlags[threadID] = false;
                        return;
                    }
                    if (item.get() == m_haltItem.get()) {
                        m_threadWorkingFlags[threadID] = false;
                        return;
                    }
                    Run(item);
                }
                item.reset();
            }
            catch (exception& e) {
                HCP_Logger_noid(ERR, "BackupNode")
                    << " Exception, what=" << e.what()
                    << ", &item=" << ((uint64_t)item.get()) << HCPENDLOG;
                m_threadWorkingFlags[threadID] = false;
            }
            catch (...) {
                HCP_Logger_noid(ERR, "BackupNode")
                    << " Non standard exception thown"
                    << ", &item=" << ((uint64_t)item.get()) << HCPENDLOG;
                m_threadWorkingFlags[threadID] = false;
            }
        }
        item.reset();
        m_threadWorkingFlags[threadID] = false;
    }

        /*lint -e1524*/ /*lint -e1732*/ /*lint -e1733*/
    JobScheduler::JobScheduler(ThreadPool& threadPool)
        : m_output(new UnlimitedQueue<ExecutableItem>()),
          m_input(threadPool.GetInputQueue())
    {}

    JobScheduler::~JobScheduler()
    {
        DBGLOG("JobScheduler destory, %d", m_list.size());
    }

    void JobScheduler::Destroy()
    {
        m_destroyed = true;
    }

    bool JobScheduler::Put(shared_ptr<ExecutableItem> item, bool block, uint64_t milliseconds)
    {
        if (!item) {
            HCP_Logger_noid(ERR, "BackupNode") << "item is null" << HCPENDLOG;
            return false;
        }
        if (m_destroyed) {
            HCP_Logger_noid(ERR, "BackupNode") << "thread pool already destroyed" << HCPENDLOG;
            return false;
        }
        ItemWrapper* wrapper = new ItemWrapper(item, m_output);
        shared_ptr<ExecutableItem> wrapped((ExecutableItem*)wrapper);
        bool result = m_input->Put(wrapped, block, milliseconds);
        if (result) {
            PushBack(m_list, item.get());
        }
        return result;
    }

    bool JobScheduler::Get(shared_ptr<ExecutableItem>& item, bool block, uint64_t milliseconds)
    {
        if (m_destroyed) {
            HCP_Logger_noid(ERR, "BackupNode") << "thread pool already destroyed" << HCPENDLOG;
            return false;
        }
        bool result = m_output->Get(item, block, milliseconds);
        if (result) {
            Remove(m_list, item.get());
        }
        return result;
    }

    void JobScheduler::Exec(shared_ptr<ExecutableItem>& item)
    {
        if (!item) {
            HCP_Logger_noid(ERR, "BackupNode") << "item is null" << HCPENDLOG;
            return;
        }
        PushBack(m_list, item.get());
        ItemWrapper wrapper(item, m_output);
        wrapper.Exec();
    }

    bool JobScheduler::Empty()
    {
        return Size(m_list) == 0;
    }

    JobScheduler::ItemWrapper::ItemWrapper(shared_ptr<ExecutableItem> item,
        STPOutput output)
        : m_item(item), m_output(output)
    {
    }
    void JobScheduler::ItemWrapper::Exec()
    {
        try {
            if (!m_item) {
                HCP_Logger_noid(ERR, "BackupNode") << "m_item is null" << HCPENDLOG;
                return;
            }
            m_item->Exec();
        }
        catch (exception& e) {
            HCP_Logger_noid(ERR, "BackupNode")
                << " Exception, what=" << e.what() << ", result=" << m_result
                << ", &item=" << ((uint64_t)m_item.get()) << HCPENDLOG;
            if (m_item->m_result == 0) {
                m_item->m_result = -1;
            }
        }
        m_output->Put(m_item);
    }

    class NullExcutebleObject : public ExcutebleObject {
    public:
    void Read() override {};
    void Write() override {};
    void Exec() override {};
    void Finish() override {};
    void Next() override {};
    };
} // namespace Module