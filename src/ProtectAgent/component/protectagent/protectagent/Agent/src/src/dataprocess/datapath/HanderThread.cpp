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
#include <unistd.h>
#include "common/Utils.h"
#include "dataprocess/datapath/HanderThread.h"

namespace {
    const mp_int32 TRY_END_TIME = 10;
    const mp_int32 HANDER_TASK_TIME = 300;
    const mp_int32 ONE_MINUTE = 1000;
}

HanderThread::HanderThread()
{
    CMpThread::InitLock(&m_dataFuncLock);
}

HanderThread::~HanderThread()
{
    if (m_thread.get() != nullptr && m_runTask) {
        StopThread();
        CMpThread::DestroyLock(&m_dataFuncLock);
        m_thread->join();
    } else {
        CMpThread::DestroyLock(&m_dataFuncLock);
    }
}

void HanderThread::StartThread()
{
    if (m_thread == nullptr) {
        m_thread = std::make_unique<std::thread>(&HanderThread::FuncHander, this);
    }
}

bool HanderThread::IsRuning()
{
    return m_runTask;
}

void HanderThread::StopThread()
{
    CThreadAutoLock threadLock(&m_dataFuncLock);
    m_FuncVec.clear();
    m_runTask = false;
}

void HanderThread::InsertTask(std::function<void()> func)
{
    CThreadAutoLock threadLock(&m_dataFuncLock);
    INFOLOG("Hander Thread Insert Task Success!");
    m_FuncVec.push_back(func);
}

void HanderThread::FuncHander()
{
    INFOLOG("Hander Thread Run Success");
    while (m_runTask || (!m_FuncVec.empty())) {
        std::function<void(void)> func = nullptr;
        {
            CThreadAutoLock threadLock(&m_dataFuncLock);
            if (!m_FuncVec.empty()) {
                INFOLOG("Do Run task!");
                func = m_FuncVec.front();
                m_FuncVec.pop_front();
            }
        }
        if (func != nullptr) {
            func();
        }
        DoSleep(HANDER_TASK_TIME);
    }
    DoSleep(HANDER_TASK_TIME * TRY_END_TIME);
    INFOLOG("Hander Thread End Success");
}