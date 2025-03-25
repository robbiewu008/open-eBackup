/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file AgentExitFlag.h
 * @brief  Contains function declarations AgentExitFlag
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
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
