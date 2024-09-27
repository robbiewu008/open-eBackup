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
#ifndef MODULE_THREADPOOL_H
#define MODULE_THREADPOOL_H
#include "define/Defines.h"
#include <queue>
#include <mutex>
#include <atomic>
#include "boost/shared_ptr.hpp"
#include "boost/thread.hpp"
#include "boost/asio.hpp"
#include "SingleItemQueue.h"

namespace Module
{
    class NullItem : public std::exception
    {
        std::string message;
    public:
        NullItem() : message("NullItem") { }
        virtual ~NullItem() throw() { }
        virtual const char* what() const throw() { return message.c_str(); }
    };

    class ExecutableItem
    {
    public:
        virtual void Exec() { return; };
        inline int& Result() { return m_result; }
        int m_result;
        //coverity fix: define a virtual destruct
        virtual ~ExecutableItem(){};
    };

    typedef std::shared_ptr< UnlimitedQueue<ExecutableItem> > STPOutput;
    typedef SingleItemQueue<ExecutableItem> STPInput;

    class ThreadPool
    {
    public:
        ThreadPool(size_t nThreads);
        std::thread** m_Threads = NULL;
        size_t m_threadNum;
        virtual ~ThreadPool();
        SingleItemQueue<ExecutableItem>* GetInputQueue();
        STPInput m_input;
    protected:
        virtual void Run(std::shared_ptr<ExecutableItem>& item);
        virtual void WorkerLoop();
        ThreadPool(const ThreadPool& other);
        ThreadPool& operator=(const ThreadPool& other);
        
        std::shared_ptr<ExecutableItem> m_haltItem;
        
        friend void CreateThreads(ThreadPool* pool,size_t nThread);
        friend void DeleteThreads(ThreadPool* pool);
    };

    class AGENT_API JobScheduler
    {
    public:
        JobScheduler(ThreadPool& threadPool);
        ~JobScheduler();
        bool Put(std::shared_ptr<ExecutableItem> item, bool block = true, uint64_t milliseconds = 0);
        bool Get(std::shared_ptr<ExecutableItem>& item, bool block = true, uint64_t milliseconds = 0);
        void Exec(std::shared_ptr<ExecutableItem>& item);
        bool Empty();
        void Destroy();
    private:
        STPOutput m_output;
        STPInput* m_input;
        std::list<ExecutableItem*> m_list;
        std::atomic<bool> m_destroyed { false };

        class ItemWrapper : public ExecutableItem
        {
            std::shared_ptr<ExecutableItem> m_item;
            STPOutput m_output;
        public:
            ItemWrapper(std::shared_ptr<ExecutableItem> item, STPOutput output);
            virtual void Exec();
            virtual ~ItemWrapper() = default; 
        };
    };

    class ExcutebleObject :public ExecutableItem
    {
    public:
        
       virtual void Read() =0;
       virtual void Write() =0;
       virtual void Exec() = 0;
       virtual void Finish(){};
       virtual void Next() {};

       int GetState(){return m_State;}
       void SetState(int state){m_State = state;}
       int GetQueueType(){return m_QueueType;}
       void SetQueueType(int queueType){m_QueueType = queueType;}
 
    private:
        int m_State;
        int m_QueueType;
    };

    enum OBJECT_STATE
    {
        READ_WRITE_OBJECT_STATE_READ,
        READ_WRITE_OBJECT_STATE_WRITE,
        READ_WRITE_OBJECT_STATE_EXEC,
        READ_WRITE_OBJECT_STATE_FINISH,
    };

    enum QUEUE_TYPE
    {
        QUEUE_TYPE_PRODUCT,
        QUEUE_TYPE_BACKUP,
    };
} // namespace Module

#endif