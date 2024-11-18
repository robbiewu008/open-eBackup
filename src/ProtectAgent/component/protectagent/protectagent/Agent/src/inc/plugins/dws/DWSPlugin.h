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
#ifndef __AGENT_DWS_PLUGIN_H__
#define __AGENT_DWS_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "apps/dws/XBSAServer/DwsTaskCommonDef.h"

class DWSPlugin : public CServicePlugin {
public:
    DWSPlugin();
    ~DWSPlugin();

#ifdef WIN32
    static DWORD WINAPI ScheduleThread(mp_void *object);
#else
    static mp_void *ScheduleThread(mp_void *object);
#endif

    mp_int32 Init(std::vector<mp_uint32> &cmds);
    mp_int32 DoAction(CDppMessage &reqMsg, CDppMessage &rspMsg);

private:
    mp_void ScheduleThreadRun();
    mp_void WriteSpeedFile(uint64_t totalSize, const std::string &output, const DwsCacheInfo &cacheInfo);

private:
    thread_id_t m_scheduleThreadId;
    volatile mp_bool m_schedulethreadExitFlag;
    mp_uint32 m_scheduleInterval;
};

#endif // __AGENT_DWS_PLUGIN_H__
