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
#include <algorithm>
#include <vector>
#include "apps/vmwarenative/VMwareNativeTask.h"

#include "common/Utils.h"
#include "message/tcp/CDppMessage.h"
#include "message/tcp/MessageHandler.h"
#include "taskmanager/TaskContext.h"

using namespace std;

VMwareNativeTask::VMwareNativeTask(const mp_string& taskID) : Task(taskID)
{
    m_statusFlag = MP_FALSE;
}

VMwareNativeTask::~VMwareNativeTask()
{
    m_statusFlag = MP_FALSE;
}

mp_void VMwareNativeTask::RunTaskBefore()
{}

mp_void VMwareNativeTask::RunTaskAfter()
{}

#ifdef WIN32
DWORD WINAPI VMwareNativeTask::RunGetProgressTask(mp_void* pThis)
#else
mp_void* VMwareNativeTask::RunGetProgressTask(mp_void* pThis)
#endif
{
#ifdef WIN32
    return 0;
#else
    return NULL;
#endif
}

mp_bool VMwareNativeTask::GetStatusFlag()
{
    return m_statusFlag;
}

mp_int32 VMwareNativeTask::ReportTaskStatus(VMwareNativeTask* task)
{
    mp_int32 ret = MP_SUCCESS;
    return ret;
}
