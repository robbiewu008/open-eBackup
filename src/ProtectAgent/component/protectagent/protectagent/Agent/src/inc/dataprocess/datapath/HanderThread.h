#include <thread>
#include <list>
#include <functional>
#include <atomic>
#include "message/tcp/CDppMessage.h"

class HanderThread {
public:
    HanderThread();
    virtual ~HanderThread();

    void StartThread();
    void StopThread();
    bool IsRuning();
    void InsertTask(std::function<void()> func);

private:
    void FuncHander();

private:
    thread_lock_t m_dataFuncLock;
    std::atomic<bool> m_runTask { true };
    std::unique_ptr<std::thread> m_thread;
    std::list<std::function<void(void)>> m_FuncVec;
};