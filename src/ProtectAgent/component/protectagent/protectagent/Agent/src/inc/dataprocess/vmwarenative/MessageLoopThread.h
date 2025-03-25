/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file MessageLoopThread.h
 * @brief  The implemention about message thread
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_VMWARENATIVE_DEADLOCK_CHECK_THREAD_HPP__H
#define AGENT_VMWARENATIVE_DEADLOCK_CHECK_THREAD_HPP__H

#include <string>
#include <thread>
#include <chrono>
#include <queue>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>

#include <functional>
#include <mutex>

class MessageLoop {
public:
    MessageLoop() : m_running(true), m_delayedTaskSeq(0), m_epFd(-1), m_wakeupOutFd(-1), m_wakeupInFd(-1)
    {}

    ~MessageLoop()
    {
        if (m_wakeupOutFd >= 0) {
            close(m_wakeupOutFd);
            m_wakeupOutFd = -1;
        }
        if (m_wakeupInFd >= 0) {
            close(m_wakeupInFd);
            m_wakeupInFd = -1;
        }
        if (m_epFd >= 0) {
            close(m_epFd);
            m_epFd = -1;
        }
    }

    void Run()
    {
        struct epoll_event event;
        struct epoll_event events[20];
        event.data.fd = m_wakeupOutFd;
        event.events = EPOLLIN;
        epoll_ctl(m_epFd, EPOLL_CTL_ADD, m_wakeupOutFd, &event);
        m_recentTime = std::chrono::steady_clock::now();

        m_running = true;
        while (m_running) {
            uint64_t nextDelayedTaskDuration = 0;
            bool did_work = DoWork();
            did_work |= DoDelayedWork(nextDelayedTaskDuration);
            if (did_work) {
                continue;
            }
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                if (!m_workQueue.empty()) {
                    continue;
                }
            }

            int number = 0;
            int waitNum = 20;
            if (nextDelayedTaskDuration != 0) {
                number = epoll_wait(m_epFd, events, waitNum, (int)nextDelayedTaskDuration);
            } else {
                number = epoll_wait(m_epFd, events, waitNum, -1);
            }
            if (number > 0) {
                char buf;
                (void)read(m_wakeupOutFd, &buf, 1);
            }
        }
    }

    void QuitNow()
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_running = false;
        }
        char buf = 0;
        (void)write(m_wakeupInFd, &buf, 1);
    }

    void PostTask(const std::function<void()>& fun)
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            PendingTask task;
            task.fun = fun;
            m_workQueue.push(task);
        }
        char buf = 0;
        (void)write(m_wakeupInFd, &buf, 1);
    }

    void PostDelayedTask(const std::function<void()>& fun, const uint64_t milliseconds)
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            PendingTask task;
            task.fun = fun;
            task.seq = m_delayedTaskSeq++;
            task.runTimePoint = std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds);
            m_workQueue.push(task);
        }

        char buf = 0;
        (void)write(m_wakeupInFd, &buf, 1);
    }

    bool Init()
    {
        int fdSize = 256;
        m_epFd = epoll_create(fdSize);
        if (-1 == m_epFd) {
            return false;
        }
        int fds[2];
        if (pipe(fds)) {
            return false;
        }
        if (!SetNonBlocking(fds[0])) {
            return false;
        }
        if (!SetNonBlocking(fds[1])) {
            return false;
        }
        m_wakeupOutFd = fds[0];
        m_wakeupInFd = fds[1];
        return true;
    }

private:
    bool DoWork()
    {
        bool ret = false;
        std::queue<PendingTask> workQueue;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            std::swap(workQueue, m_workQueue);
        }

        while (!workQueue.empty()) {
            PendingTask pending_task = workQueue.front();
            workQueue.pop();
            if (pending_task.runTimePoint.time_since_epoch().count() == 0) {
                ret = true;
                pending_task.fun();
            } else {
                m_delayedWorkQueue.push(pending_task);
            }
        }
        return ret;
    }

    bool DoDelayedWork(uint64_t& nextDelayedTaskDuration)
    {
        nextDelayedTaskDuration = 0;
        if (m_delayedWorkQueue.empty()) {
            return false;
        }

        std::chrono::steady_clock::time_point nextRunTime = m_delayedWorkQueue.top().runTimePoint;
        if (nextRunTime > m_recentTime) {
            m_recentTime = std::chrono::steady_clock::now();
            if (nextRunTime > m_recentTime) {
                int tmpDen = 1000000;
                int inCrement = 1;
                nextDelayedTaskDuration = (nextRunTime - m_recentTime).count() / tmpDen + inCrement;
                return false;
            }
        }

        PendingTask pendingTask = m_delayedWorkQueue.top();
        m_delayedWorkQueue.pop();

        pendingTask.fun();
        return true;
    }

    struct PendingTask {
        std::function<void()> fun;
        std::chrono::steady_clock::time_point runTimePoint;
        uint64_t seq;

        // Used to support sorting.
        bool operator<(const PendingTask& other) const
        {
            if (runTimePoint > other.runTimePoint) {
                return true;
            }
            if (runTimePoint < other.runTimePoint) {
                return false;
            }
            return seq > other.seq;
        }
    };
    bool SetNonBlocking(int fd)
    {
        const int flags = fcntl(fd, F_GETFL);
        if (flags == -1)
            return false;
        if (flags & O_NONBLOCK)
            return true;
        if (-1 == fcntl(fd, F_SETFL, flags | O_NONBLOCK))
            return false;
        return true;
    }

private:
    std::queue<PendingTask> m_workQueue;
    std::priority_queue<PendingTask> m_delayedWorkQueue;
    std::chrono::steady_clock::time_point m_recentTime;
    std::mutex m_mutex;
    bool m_running;
    uint64_t m_delayedTaskSeq;
    int m_epFd;
    int m_wakeupOutFd;
    int m_wakeupInFd;
};

class MessageLoopThread {
public:
    MessageLoopThread()
    {}
    ~MessageLoopThread()
    {}

    bool Start()
    {
        try {
            m_messageLoop = std::make_unique<MessageLoop>();
            (void)m_messageLoop->Init();
            m_thread = std::make_unique<std::thread>(std::bind(&MessageLoopThread::ThreadMain, this));
        } catch (...) {
            m_messageLoop.reset();
            m_thread.reset();
            return false;
        }

        return true;
    }

    bool Stop()
    {
        if (NULL == m_messageLoop.get() || NULL == m_thread.get()) {
            return false;
        }
        m_messageLoop->QuitNow();
        m_thread->join();
        m_messageLoop.reset();
        m_thread.reset();
        return true;
    }

    MessageLoop* GetMessageloop()
    {
        return m_messageLoop.get();
    }

    bool IsRunning()
    {
        return NULL != m_thread.get();
    }

private:
    void ThreadMain()
    {
        m_messageLoop->Run();
    }

private:
    std::unique_ptr<std::thread> m_thread;
    std::unique_ptr<MessageLoop> m_messageLoop;
};
#endif
