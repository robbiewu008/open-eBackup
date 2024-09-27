/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file VMwareNativeTask.h
 * @brief  Contains function declarations for VMwareNativeTask
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_VMWARENATIVE_TASK
#define AGENT_VMWARENATIVE_TASK

#include "common/Types.h"
#include "common/Uuid.h"
#include "common/CMpThread.h"
#include "taskmanager/Task.h"

class VMwareNativeTask : public Task {
public:
    VMwareNativeTask(const mp_string& taskID);
    virtual ~VMwareNativeTask();

    mp_bool GetStatusFlag();

protected:
    mp_void RunTaskBefore();
    mp_void RunTaskAfter();

#ifdef WIN32
    static DWORD WINAPI RunGetProgressTask(mp_void* pThis);
#else
    static mp_void* RunGetProgressTask(mp_void* pThis);
#endif
    static mp_int32 ReportTaskStatus(VMwareNativeTask* task);

protected:
    thread_id_t m_statusTid;
    mp_bool m_statusFlag;
    mp_int32 m_iStorageProtocol;
};

#endif
