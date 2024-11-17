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