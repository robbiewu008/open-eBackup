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
