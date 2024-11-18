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
#ifndef _AGENT_TASK_DISPATCH_WORKER_H_
#define _AGENT_TASK_DISPATCH_WORKER_H_

#include "common/Types.h"
#include "common/CMpThread.h"
#include "agent/Communication.h"
#include "agent/TaskWorker.h"
#include "message/tcp/MessageHandler.h"

class TaskDispatchWorker {
public:
    TaskDispatchWorker();
    ~TaskDispatchWorker();

    thread_id_t& GetThreadId();
    mp_bool NeedExit();
    mp_void SetRecvThreadStatus(mp_int32 iThreadStatus);
#ifdef WIN32
    mp_int32 Init(TaskWorker*& pTaskWorkers, mp_int32 iCount, TaskWorker& pVssWorker);
#else
    mp_int32 Init(TaskWorker*& pTaskWorkers, mp_int32 iCount);
#endif
    mp_void Exit();

private:
    thread_id_t m_dispatchTid;
    mp_int32 m_iWorkerCount;
    mp_int32 m_lastPushedWorker;
    volatile mp_bool m_bNeedExit;       // 线程退出标识
    volatile mp_int32 m_iThreadStatus;  // 接收线程状态
    // m_pWorkers和m_pVssWorker由外部分配和释放
    TaskWorker** m_pWorkers;
#ifdef WIN32
    TaskWorker* m_pVssWorker;
#endif

    static const mp_int32 TASK_DISPATCH_WORKER_NUM_100 = 100;
    static const mp_int32 MAX_PUSHED_WORKERS_CNT = 2147483640;  // int -> +2147483647  防止溢出
    static const mp_int32 MAX_GET_WORKERS_COUNT = 30;   // 最多等待3秒钟后退出
    static const mp_int32 ALL_TASK_BUSY_WAIT_TIME = 100;

private:
    mp_void PushMsgToWorker(message_pair_t& msg);
#ifdef WIN32
    mp_bool IsVSSRequst(message_pair_t& msg);
    static DWORD WINAPI DispacthProc(void* pThis);
#else
    static void* DispacthProc(void* pThis);
#endif
    mp_void HandleAllTaskBusy(message_pair_t& msg);
};

#endif  // _AGENT_TASK_DISPATCH_WORKER_H_
