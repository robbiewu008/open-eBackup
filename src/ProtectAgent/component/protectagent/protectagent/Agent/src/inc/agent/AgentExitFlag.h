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
#ifndef AGENT_MAIN_H
#define AGENT_MAIN_H

#include "common/Defines.h"

static const mp_uchar AGENT_RUNNING_PROCESS_COUNT = 1;

// AgentÍË³öµÄ±ê¼ÇÎ»
class AgentExitFlag {
public:
    static AgentExitFlag& GetInstace()
    {
        static AgentExitFlag agentExitFlag;
        return agentExitFlag;
    }

    ~AgentExitFlag()
    {
        exitFlag = MP_FALSE;
    }

    mp_bool GetExitFlag()
    {
        return exitFlag;
    }

    mp_void SetExitFlag(mp_bool eFlag)
    {
        exitFlag = eFlag;
    }

private:
    mp_bool exitFlag;

private:
    AgentExitFlag() : exitFlag(MP_FALSE) {}
};

#ifdef WIN32


class WinServerStatus {
public:
    static WinServerStatus& GetInstace()
    {
        static WinServerStatus winServerStatus;
        return winServerStatus;
    }
    SERVICE_STATUS_HANDLE& GetServiceHandle()
    {
        return hServiceStatus;
    }

    mp_void InitServiceHandle(SERVICE_STATUS_HANDLE sHandle)
    {
        hServiceStatus = sHandle;
    }

private:
    WinServerStatus() {}
    ~WinServerStatus() {}
    SERVICE_STATUS_HANDLE hServiceStatus;
};

#endif

#endif  // __AGENT_MAIN_H__
