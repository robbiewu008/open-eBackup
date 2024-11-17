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
#include "taskmanager/TaskStepScanDisk.h"
#include "common/Log.h"
#include "common/CMpThread.h"
#include "host/host.h"

namespace {
    static thread_lock_t *scanMutex = NULL;
}

TaskStepScanDisk::TaskStepScanDisk(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStep(id, taskId, name, ratio, order)
{}

TaskStepScanDisk::~TaskStepScanDisk()
{}

mp_int32 TaskStepScanDisk::Init(const Json::Value& param)
{
    LOGGUARD("");
    if (scanMutex == NULL) {
        scanMutex = new (std::nothrow) thread_lock_t();
        mp_int32 iRet = CMpThread::InitLock(scanMutex);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "init scan lock failed, iRet %d.", iRet);
            return iRet;
        }
    }
    return MP_SUCCESS;
}

mp_int32 TaskStepScanDisk::Run()
{
    CThreadAutoLock scanLock(scanMutex);
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to scan disk info...", m_taskId.c_str());
    CHost host;
    mp_int32 iRet = host.ScanDisk();
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Scan disk faield, iRet %d.", iRet);
        return iRet;
    }
    m_stepStatus = STATUS_INPROGRESS;
    COMMLOG(OS_LOG_INFO, "Task(%s) scan disk info succ", m_taskId.c_str());
    return MP_SUCCESS;
}

mp_int32 TaskStepScanDisk::Stop(const Json::Value& param)
{
    m_stepStatus = STATUS_COMPLETED;
    return MP_SUCCESS;
}

mp_int32 TaskStepScanDisk::Redo(mp_string& innerPID)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepScanDisk::Cancel()
{
    m_stepStatus = STATUS_DELETED;
    return MP_SUCCESS;
}

mp_int32 TaskStepScanDisk::Cancel(Json::Value& respParam)
{
    m_stepStatus = STATUS_DELETED;
    return MP_SUCCESS;
}

mp_int32 TaskStepScanDisk::Update(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepScanDisk::Update(Json::Value& param, Json::Value& respParam)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepScanDisk::Finish(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepScanDisk::Finish(Json::Value& param, Json::Value& respParam)
{
    return MP_SUCCESS;
}
